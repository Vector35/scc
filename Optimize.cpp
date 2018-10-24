#include <queue>
#include <list>
#include <stdio.h>
#include <math.h>
#include "Optimize.h"
#include "Struct.h"

using namespace std;


Optimize::Optimize(Linker* linker): m_linker(linker), m_settings(linker->GetSettings())
{
}


void Optimize::PerformControlFlowAnalysis(Function* func)
{
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
		(*i)->ClearEntryAndExitBlocks();
	func->ClearExitBlocks();

	// Compute entry and exit blocks, starting at the first block of the function
	set<ILBlock*> visited;
	queue<ILBlock*> toProcess;
	toProcess.push(func->GetIL()[0]);
	visited.insert(func->GetIL()[0]);

	while (!toProcess.empty())
	{
		ILBlock* block = toProcess.front();
		toProcess.pop();

		for (vector<ILInstruction>::const_iterator i = block->GetInstructions().begin();
			i != block->GetInstructions().end(); i++)
		{
			if ((i->operation == ILOP_RETURN) || (i->operation == ILOP_RETURN_VOID))
				func->AddExitBlock(block);

			for (vector<ILParameter>::const_iterator j = i->params.begin(); j != i->params.end(); j++)
			{
				if (j->cls != ILPARAM_BLOCK)
					continue;

				ILBlock* dest = j->block;
				block->AddExitBlock(dest);
				dest->AddEntryBlock(block);

				if (m_settings.internalDebug)
				{
					bool ok = false;
					for (vector<ILBlock*>::const_iterator k = func->GetIL().begin(); k != func->GetIL().end(); k++)
					{
						if (*k == dest)
						{
							ok = true;
							break;
						}
					}
					if (!ok)
					{
						fprintf(stderr, "error: instruction '");
						i->Print();
						fprintf(stderr, "' in function '%s' has invalid exit block\n", func->GetName().c_str());
					}
				}

				if (!visited.count(dest))
				{
					toProcess.push(dest);
					visited.insert(dest);
				}
			}
		}
	}
}


void Optimize::PerformDataFlowAnalysis(Function* func)
{
	// Find all definitions of all variables and organize them by variable
	map< Ref<Variable>, vector<size_t> > varDefs;
	vector< pair<ILBlock*, size_t> > defs;
	size_t bitCount = 0;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			if (!(*i)->GetInstructions()[j].WritesToFirstParameter())
				continue;

			const ILParameter* param = &(*i)->GetInstructions()[j].params[0];
			while (param->cls == ILPARAM_MEMBER)
				param = param->parent;

			if (param->cls != ILPARAM_VAR)
				continue;

			// Global variables are not analyzed with data flow, these are assumed to be read/written directly
			if (param->variable->IsGlobal())
				continue;

			varDefs[param->variable].push_back(bitCount);
			defs.push_back(pair<ILBlock*, size_t>(*i, j));
			(*i)->SetInstructionDataFlowBit(j, bitCount++);
		}
	}

	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
		(*i)->ResetDataFlowInfo(bitCount);
	func->GetExitReachingDefinitions().Reset(bitCount, false);

	// Populate definition preservation and generation information
	map< Ref<Variable>, size_t > varDef;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		for (vector<ILInstruction>::const_iterator j = (*i)->GetInstructions().begin();
			j != (*i)->GetInstructions().end(); j++)
		{
			// Ignore instructions that aren't definitions of a local variable
			if (!j->WritesToFirstParameter())
				continue;
			const ILParameter* param = &j->params[0];
			bool full = (j->operation != ILOP_ARRAY_INDEX_ASSIGN);
			while (param->cls == ILPARAM_MEMBER)
			{
				param = param->parent;
				full = false;
			}

			if (param->cls != ILPARAM_VAR)
				continue;
			if (param->variable->IsGlobal())
				continue;

			// Kill other definitions of this variable, if it is a complete overwrite
			if (full)
			{
				if (varDef.find(param->variable) != varDef.end())
				{
					// Variable already defined in this block, kill the old definition
					(*i)->GetGeneratedDefinitions().SetBit(varDef[param->variable], false);
				}

				for (vector<size_t>::iterator k = varDefs[param->variable].begin(); k != varDefs[param->variable].end(); k++)
					(*i)->GetPreservedDefinitions().SetBit(*k, false);
			}

			varDef[param->variable] = j->dataFlowBit;
			(*i)->GetGeneratedDefinitions().SetBit(j->dataFlowBit, true);
		}
	}

	// Iteratively compute reaching definitions at the block level
	BitVector tempVector;
	tempVector.Reset(bitCount, false);
	bool changed = true;
	while (changed)
	{
		changed = false;

		for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
		{
			// Add definitions from output of all predecessor blocks to the input of the current block
			tempVector = (*i)->GetReachingDefinitionsInput();
			for (set<ILBlock*>::const_iterator j = (*i)->GetEntryBlocks().begin();
				j != (*i)->GetEntryBlocks().end(); j++)
				tempVector.Union((*j)->GetReachingDefinitionsOutput());
			if (tempVector != (*i)->GetReachingDefinitionsInput())
			{
				changed = true;
				(*i)->GetReachingDefinitionsInput() = tempVector;
			}

			// Compute reaching definitions output for this block (generate | (input & preserved))
			tempVector = (*i)->GetReachingDefinitionsInput();
			tempVector.Intersection((*i)->GetPreservedDefinitions());
			tempVector.Union((*i)->GetGeneratedDefinitions());
			if (tempVector != (*i)->GetReachingDefinitionsOutput())
			{
				changed = true;
				(*i)->GetReachingDefinitionsOutput() = tempVector;
			}
		}

		// Update reaching definitions information for exit block
		tempVector = func->GetExitReachingDefinitions();
		for (set<ILBlock*>::const_iterator i = func->GetExitBlocks().begin(); i != func->GetExitBlocks().end(); i++)
			tempVector.Union((*i)->GetReachingDefinitionsOutput());
		if (tempVector != func->GetExitReachingDefinitions())
		{
			changed = true;
			func->GetExitReachingDefinitions() = tempVector;
		}
	}

	// Use reaching definitions information to generate definition->use and use->definition chains
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		// Copy reaching definitions into temporary vector, as it will be updated per-instruction to generate
		// localized information
		tempVector = (*i)->GetReachingDefinitionsInput();

		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			// For each instruction, iterate over all variable uses and update information
			size_t startParam = 0;
			if ((*i)->GetInstructions()[j].WritesToFirstParameter())
			{
				// Instruction is a definition, ignore first parameter
				startParam = 1;
			}

			for (size_t k = startParam; k < (*i)->GetInstructions()[j].params.size(); k++)
			{
				const ILParameter* param = &(*i)->GetInstructions()[j].params[k];
				while (param->cls == ILPARAM_MEMBER)
					param = param->parent;

				if (param->cls != ILPARAM_VAR)
					continue;
				if (param->variable->IsGlobal())
					continue;

				// Add reachable definitions to the use->definition and definition->use chains
				for (vector<size_t>::iterator n = varDefs[param->variable].begin(); n != varDefs[param->variable].end(); n++)
				{
					if (tempVector.GetBit(*n))
					{
						// Definition is reachable
						(*i)->GetUseDefinitionChains()[j].push_back(defs[*n]);
						defs[*n].first->GetDefinitionUseChains()[defs[*n].second].push_back(
							pair<ILBlock*, size_t>(*i, j));
					}
				}
			}

			if ((*i)->GetInstructions()[j].WritesToFirstParameter())
			{
				// Instruction is a definition, update reaching definitions information according to the
				// variable being written
				const ILParameter* param = &(*i)->GetInstructions()[j].params[0];
				bool full = ((*i)->GetInstructions()[j].operation != ILOP_ARRAY_INDEX_ASSIGN);
				while (param->cls== ILPARAM_MEMBER)
				{
					param = param->parent;
					full = false;
				}

				if ((param->cls == ILPARAM_VAR) && (!param->variable->IsGlobal()))
				{
					// If instruction is performing a complete overwrite, kill any other definitions
					if (full)
					{
						for (vector<size_t>::iterator k = varDefs[param->variable].begin(); k != varDefs[param->variable].end(); k++)
							tempVector.SetBit(*k, false);
					}

					// Set the bit for this definition
					tempVector.SetBit((*i)->GetInstructions()[j].dataFlowBit, true);
				}
			}
		}
	}

	func->SetDefinitions(varDefs, defs);
}


bool Optimize::ConsolidateBasicBlocks(Function* func)
{
	bool changed = false;

	// Remove unused blocks (don't process first block, this has an implicit entry)
	list<ILBlock*> toRemove;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin() + 1; i != func->GetIL().end(); i++)
	{
		if ((*i)->GetEntryBlocks().size() == 0)
			toRemove.push_back(*i);
	}

	for (list<ILBlock*>::iterator i = toRemove.begin(); i != toRemove.end(); i++)
	{
		func->RemoveILBlock(*i);
		changed = true;
	}

	bool iterationChanged = true;
	while (iterationChanged)
	{
		iterationChanged = false;

		// Remove blocks that contain only a goto instruction to another block
		for (vector<ILBlock*>::const_iterator i = func->GetIL().begin() + 1; i != func->GetIL().end(); i++)
		{
			if ((*i)->GetInstructions().size() != 1)
				continue;
			if ((*i)->GetInstructions()[0].operation != ILOP_GOTO)
				continue;
			if ((*i)->GetInstructions()[0].params[0].cls != ILPARAM_BLOCK)
				continue;
			if ((*i)->GetInstructions()[0].params[0].block == *i) // don't remove infinite loops
				continue;

			// This block has one instruction, which is a goto to another block, replace all references to this
			// block with the target block
			ILBlock* target = (*i)->GetInstructions()[0].params[0].block;
			for (vector<ILBlock*>::const_iterator j = func->GetIL().begin(); j != func->GetIL().end(); j++)
			{
				for (vector<ILInstruction>::iterator k = (*j)->GetInstructions().begin();
					k != (*j)->GetInstructions().end(); k++)
				{
					for (vector<ILParameter>::iterator p = k->params.begin(); p != k->params.end(); p++)
					{
						if (p->cls != ILPARAM_BLOCK)
							continue;
						if (p->block == *i)
							p->block = target;
					}
				}
			}

			// Update control flow information
			target->RemoveEntryBlock(*i);
			for (set<ILBlock*>::iterator j = (*i)->GetEntryBlocks().begin(); j != (*i)->GetEntryBlocks().end(); j++)
			{
				target->AddEntryBlock(*j);
				(*j)->RemoveExitBlock(*i);
				(*j)->AddExitBlock(target);
			}

			// Remove the unneeded block and restart loop
			func->RemoveILBlock(*i);
			iterationChanged = true;
			changed = true;
			break;
		}

		if (iterationChanged)
			continue;

		// Combine blocks that simply fall down into another block and don't branch
		for (vector<ILBlock*>::const_iterator i = func->GetIL().begin() + 1; i != func->GetIL().end(); i++)
		{
			// Block must have a single entry point, and the preceding block must have a single exit point
			if ((*i)->GetEntryBlocks().size() != 1)
				continue;
			ILBlock* entryBlock = *((*i)->GetEntryBlocks().begin());
			if (entryBlock->GetExitBlocks().size() != 1)
				continue;

			// Blocks can be combined, remove the goto instruction at the end of the preceding block
			entryBlock->RemoveLastInstruction();

			// Append instructions from the current block to the preceding block
			for (vector<ILInstruction>::const_iterator j = (*i)->GetInstructions().begin();
				j != (*i)->GetInstructions().end(); j++)
				entryBlock->AddInstruction(*j);

			// Update control flow information
			entryBlock->RemoveExitBlock(*i);
			for (set<ILBlock*>::const_iterator j = (*i)->GetExitBlocks().begin();
				j != (*i)->GetExitBlocks().end(); j++)
			{
				entryBlock->AddExitBlock(*j);
				(*j)->RemoveEntryBlock(*i);
				(*j)->AddEntryBlock(entryBlock);
			}

			// If block was a function exit block, update the function's exit block information
			if (func->GetExitBlocks().count(*i) != 0)
			{
				func->RemoveExitBlock(*i);
				func->AddExitBlock(entryBlock);
			}

			// Remove the unneeded block and restart loop
			func->RemoveILBlock(*i);
			iterationChanged = true;
			changed = true;
			break;
		}
	}

	return changed;
}


bool Optimize::OptimizeForNoReturnCalls(Function* func)
{
	bool changed = false;
	bool canReturn = false;
	
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		bool blockCanExit = true;

		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			const ILInstruction& instr = (*i)->GetInstructions()[j];
			if (instr.operation != ILOP_CALL)
				continue;
			if (instr.params[1].cls != ILPARAM_FUNC)
				continue;
			if (instr.params[1].function->DoesReturn())
				continue;

			blockCanExit = false;

			if (((j + 1) < (*i)->GetInstructions().size()) && ((*i)->GetInstructions()[j + 1].operation == ILOP_NORETURN))
			{
				// Already marked as no return
				break;
			}

			// Calling function that does not return, eliminate return value
			(*i)->SetInstructionParameter(j, 0, ILParameter());

			// Eliminate any IL instructions after this one
			(*i)->SplitBlock(j + 1, NULL);

			// Add a "no return" IL instruction to tell the code generator that no code should follow the call
			(*i)->AddInstruction(ILOP_NORETURN);

			// Update control flow information to reflect that this block is now an "exit" block.  This may eliminate
			// blocks on the next call to basic block consolidation.
			for (set<ILBlock*>::const_iterator k = (*i)->GetExitBlocks().begin();
				k != (*i)->GetExitBlocks().end(); k++)
				(*k)->RemoveEntryBlock(*i);
			(*i)->ClearExitBlocks();
			if (func->GetExitBlocks().count(*i) == 0)
				func->AddExitBlock(*i);

			changed = true;
			break;
		}

		if (blockCanExit && (func->GetExitBlocks().count(*i) != 0))
		{
			// Block can cause exit of function
			canReturn = true;
		}
	}

	if (!canReturn)
	{
		// Function cannot return, ensure function is marked to reflect this
		func->SetDoesReturn(false);
	}

	return changed;
}


bool Optimize::FoldConstants(Function* func)
{
	bool changed = false;

	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		for (vector<ILInstruction>::iterator j = (*i)->GetInstructions().begin();
			j != (*i)->GetInstructions().end(); j++)
		{
			switch (j->operation)
			{
			case ILOP_IF_TRUE:
				if (j->params[0].IsConstant())
				{
					bool taken;
					switch (j->params[0].cls)
					{
					case ILPARAM_INT:
						taken = (j->params[0].integerValue != 0);
						break;
					case ILPARAM_BOOL:
						taken = j->params[0].boolValue;
						break;
					default:
						taken = true;
						break;
					}

					ILBlock* dest;
					j->operation = ILOP_GOTO;
					if (taken)
						dest = j->params[1].block;
					else
						dest = j->params[2].block;
					j->params.clear();
					j->params.push_back(ILParameter(dest));
					changed = true;
				}
				break;
			case ILOP_IF_LESS_THAN:
				if (j->params[0].IsConstant() && j->params[1].IsConstant())
				{
					bool taken;
					switch (j->params[0].cls)
					{
					case ILPARAM_INT:
						taken = (j->params[0].integerValue < j->params[1].integerValue);
						break;
					case ILPARAM_FLOAT:
						taken = (j->params[0].floatValue < j->params[1].floatValue);
						break;
					default:
						taken = false;
						break;
					}

					ILBlock* dest;
					j->operation = ILOP_GOTO;
					if (taken)
						dest = j->params[2].block;
					else
						dest = j->params[3].block;
					j->params.clear();
					j->params.push_back(ILParameter(dest));
					changed = true;
				}
				break;
			case ILOP_IF_LESS_EQUAL:
				if (j->params[0].IsConstant() && j->params[1].IsConstant())
				{
					bool taken;
					switch (j->params[0].cls)
					{
					case ILPARAM_INT:
						taken = (j->params[0].integerValue <= j->params[1].integerValue);
						break;
					case ILPARAM_FLOAT:
						taken = (j->params[0].floatValue <= j->params[1].floatValue);
						break;
					default:
						taken = false;
						break;
					}

					ILBlock* dest;
					j->operation = ILOP_GOTO;
					if (taken)
						dest = j->params[2].block;
					else
						dest = j->params[3].block;
					j->params.clear();
					j->params.push_back(ILParameter(dest));
					changed = true;
				}
				break;
			case ILOP_IF_BELOW:
				if (j->params[0].IsConstant() && j->params[1].IsConstant())
				{
					bool taken;
					switch (j->params[0].cls)
					{
					case ILPARAM_INT:
						taken = ((uint64_t)j->params[0].integerValue < (uint64_t)j->params[1].integerValue);
						break;
					case ILPARAM_FLOAT:
						taken = (j->params[0].floatValue < j->params[1].floatValue);
						break;
					default:
						taken = false;
						break;
					}

					ILBlock* dest;
					j->operation = ILOP_GOTO;
					if (taken)
						dest = j->params[2].block;
					else
						dest = j->params[3].block;
					j->params.clear();
					j->params.push_back(ILParameter(dest));
					changed = true;
				}
				break;
			case ILOP_IF_BELOW_EQUAL:
				if (j->params[0].IsConstant() && j->params[1].IsConstant())
				{
					bool taken;
					switch (j->params[0].cls)
					{
					case ILPARAM_INT:
						taken = ((uint64_t)j->params[0].integerValue <= (uint64_t)j->params[1].integerValue);
						break;
					case ILPARAM_FLOAT:
						taken = (j->params[0].floatValue <= j->params[1].floatValue);
						break;
					default:
						taken = false;
						break;
					}

					ILBlock* dest;
					j->operation = ILOP_GOTO;
					if (taken)
						dest = j->params[2].block;
					else
						dest = j->params[3].block;
					j->params.clear();
					j->params.push_back(ILParameter(dest));
					changed = true;
				}
				break;
			case ILOP_IF_EQUAL:
				if (j->params[0].IsConstant() && j->params[1].IsConstant())
				{
					bool taken;
					switch (j->params[0].cls)
					{
					case ILPARAM_INT:
						taken = (j->params[0].integerValue == j->params[1].integerValue);
						break;
					case ILPARAM_FLOAT:
						taken = (j->params[0].floatValue == j->params[1].floatValue);
						break;
					case ILPARAM_BOOL:
						taken = (j->params[0].boolValue == j->params[1].boolValue);
						break;
					default:
						taken = false;
						break;
					}

					ILBlock* dest;
					j->operation = ILOP_GOTO;
					if (taken)
						dest = j->params[2].block;
					else
						dest = j->params[3].block;
					j->params.clear();
					j->params.push_back(ILParameter(dest));
					changed = true;
				}
				break;
			case ILOP_ADD:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue += j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue += j->params[2].floatValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SUB:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue -= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue -= j->params[2].floatValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SMULT:
			case ILOP_UMULT:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue *= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue *= j->params[2].floatValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SDIV:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT) && (j->params[2].integerValue != 0))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue /= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue /= j->params[2].floatValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_UDIV:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT) && (j->params[2].integerValue != 0))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = (int64_t)((uint64_t)j->params[1].integerValue / (uint64_t)j->params[2].integerValue);
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue /= j->params[2].floatValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SMOD:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT) && (j->params[2].integerValue != 0))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue %= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue = fmod(j->params[1].floatValue, j->params[2].floatValue);
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_UMOD:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT) && (j->params[2].integerValue != 0))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = (int64_t)((uint64_t)j->params[1].integerValue % (uint64_t)j->params[2].integerValue);
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_FLOAT) && (j->params[2].cls == ILPARAM_FLOAT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].floatValue = fmod(j->params[1].floatValue, j->params[2].floatValue);
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_AND:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue &= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_OR:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue |= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_XOR:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue ^= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SHL:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue <<= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SHR:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = (int64_t)((uint64_t)j->params[1].integerValue >> (uint8_t)j->params[2].integerValue);
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_SAR:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[2].cls == ILPARAM_INT))
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue >>= j->params[2].integerValue;
					j->params.erase(j->params.begin() + 2);
					changed = true;
				}
				break;
			case ILOP_NEG:
				if (j->params[1].cls == ILPARAM_INT)
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = -j->params[1].integerValue;
					changed = true;
				}
				break;
			case ILOP_NOT:
				if (j->params[1].cls == ILPARAM_INT)
				{
					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = ~j->params[1].integerValue;
					changed = true;
				}
				break;
			case ILOP_BYTESWAP:
				if ((j->params[1].cls == ILPARAM_INT) && (j->params[1].type == ILTYPE_INT16))
				{
					uint16_t value = (uint16_t)j->params[1].integerValue;
					value = (value << 8) | (value >> 8);

					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = value;
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_INT) && (j->params[1].type == ILTYPE_INT32))
				{
					uint32_t value = (uint32_t)j->params[1].integerValue;
					value = (value << 24) | ((value << 8) & 0xff0000) | ((value >> 8) & 0xff00) | (value >> 24);

					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = value;
					changed = true;
				}
				else if ((j->params[1].cls == ILPARAM_INT) && (j->params[1].type == ILTYPE_INT64))
				{
					uint64_t value = (uint64_t)j->params[1].integerValue;
					value = (value << 56) | ((value << 40) & 0xff000000000000LL) | ((value << 24) & 0xff0000000000LL) |
						((value << 8) & 0xff00000000LL) | ((value >> 8) & 0xff000000LL) | ((value >> 24) & 0xff0000LL) |
						((value >> 40) & 0xff00LL) | (value >> 56);

					j->operation = ILOP_ASSIGN;
					j->params[1].integerValue = value;
					changed = true;
				}
				break;
			default:
				break;
			}
		}
	}

	return changed;
}


void Optimize::InlineFunction(Function* func, Function* target)
{
	// Find all the calls to the target function
	list< pair<ILBlock*, size_t> > calls;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			if ((*i)->GetInstructions()[j].operation != ILOP_CALL)
				continue;
			if ((*i)->GetInstructions()[j].params[1].cls != ILPARAM_FUNC)
				continue;
			if ((*i)->GetInstructions()[j].params[1].function != target)
				continue;
			calls.push_back(pair<ILBlock*, size_t>(*i, j));
		}
	}

	// Perform inlining on each call
	for (list< pair<ILBlock*, size_t> >::iterator i = calls.begin(); i != calls.end(); i++)
	{
		// Make a copy of the instruction, as it is going away but information from it is needed
		ILInstruction instr = i->first->GetInstructions()[i->second];

		// Split the basic block at the call instruction.  It will be recombined into a single
		// block if possible later in the optimization process.
		ILBlock* endBlock = func->CreateILBlock();
		i->first->SplitBlock(i->second + 1, endBlock);
		i->first->RemoveLastInstruction();

		// Copy parameters that aren't read only to ensure the target function doesn't corrupt
		// the parent function's context.  For read only parameters, use the caller's value
		// directly in the inlined function.
		std::map< Ref<Variable>, ILParameter > varMapping;
		for (vector< Ref<Variable> >::const_iterator j = target->GetVariables().begin();
			j != target->GetVariables().end(); j++)
		{
			if (!(*j)->IsParameter())
				continue;

			ILParameter param = instr.params[4 + (*j)->GetParameterIndex()];
			if ((*j)->IsWritten())
			{
				// Parameter is written, make a copy
				ILParameter copy = func->CreateTempVariable((*j)->GetType());
				i->first->AddInstruction(ILOP_ASSIGN, copy, param);
				varMapping[*j] = copy;
			}
			else
			{
				// Parameter is read only, map directly
				varMapping[*j] = param;
			}
		}

		// Let the inlined function write the return value directly to the variable which
		// was going to hold it
		ILParameter returnValue = instr.params[0];

		// Copy the variables (excluding the parameters, which have already been mapped)
		for (vector< Ref<Variable> >::const_iterator j = target->GetVariables().begin();
			j != target->GetVariables().end(); j++)
		{
			if (!(*j)->IsParameter())
				func->AddVariable(*j);
		}

		// Create block objects for every block that will be added to the parent function, and
		// keep a mapping of blocks from the target function to the parent function
		map<ILBlock*, ILBlock*> blockMapping;
		for (vector<ILBlock*>::const_iterator j = target->GetIL().begin(); j != target->GetIL().end(); j++)
		{
			ILBlock* newBlock = func->CreateILBlock();
			blockMapping[*j] = newBlock;
		}

		// Copy over the basic blocks, replacing parameter variables along the way, as well as
		// converting return instructions into a store and goto.
		for (vector<ILBlock*>::const_iterator j = target->GetIL().begin(); j != target->GetIL().end(); j++)
		{
			ILBlock* newBlock = blockMapping[*j];

			for (vector<ILInstruction>::iterator k = (*j)->GetInstructions().begin();
				k != (*j)->GetInstructions().end(); k++)
			{
				ILInstruction instr = *k;

				// Replace parameter variables and block references
				for (size_t p = 0; p < instr.params.size(); p++)
				{
					if (instr.params[p].cls == ILPARAM_VAR)
					{
						if (varMapping.find(instr.params[p].variable) == varMapping.end())
							continue;
						instr.params[p] = varMapping[instr.params[p].variable];
					}
					else if (instr.params[p].cls == ILPARAM_BLOCK)
					{
						if (blockMapping.find(instr.params[p].block) == blockMapping.end())
							continue;
						instr.params[p].block = blockMapping[instr.params[p].block];
					}
				}

				if (instr.operation == ILOP_RETURN_VOID)
				{
					if (target->DoesReturn())
						newBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
					else
						newBlock->AddInstruction(ILOP_NORETURN);
				}
				else if (instr.operation == ILOP_RETURN)
				{
					if (target->DoesReturn())
					{
						if (returnValue.cls != ILPARAM_VOID)
							newBlock->AddInstruction(ILOP_ASSIGN, returnValue, instr.params[0]);
						newBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
					}
					else
					{
						newBlock->AddInstruction(ILOP_NORETURN);
					}
				}
				else
				{
					newBlock->AddInstruction(instr);
				}
			}
		}

		// Jump to the inlined function at the start
		i->first->AddInstruction(ILOP_GOTO, ILParameter(blockMapping[target->GetIL()[0]]));
	}
}


void Optimize::RemoveUnreferencedSymbols(Function* protectedFunction)
{
	// Tag everything referenced from the main function
	for (vector< Ref<Function> >::iterator i = m_linker->GetFunctions().begin(); i != m_linker->GetFunctions().end(); i++)
		(*i)->ResetTagCount();
	for (vector< Ref<Variable> >::iterator i = m_linker->GetVariables().begin(); i != m_linker->GetVariables().end(); i++)
		(*i)->ResetTagCount();
	m_linker->GetFunctions()[0]->TagReferences();
	if (protectedFunction)
		protectedFunction->TagReferences();

	// Remove anything not referenced
	for (size_t i = 0; i < m_linker->GetFunctions().size(); i++)
	{
		if (m_linker->GetFunctions()[i]->GetTagCount() == 0)
		{
			m_linker->GetFunctions().erase(m_linker->GetFunctions().begin() + i);
			i--;
		}
	}

	for (size_t i = 0; i < m_linker->GetVariables().size(); i++)
	{
		if (m_linker->GetVariables()[i]->GetTagCount() == 0)
		{
			m_linker->GetVariables().erase(m_linker->GetVariables().begin() + i);
			i--;
		}
	}
}


void Optimize::PerformGlobalOptimizations()
{
	if (m_settings.optimization == OPTIMIZE_DISABLE)
		return;

	// For each function, determine which variables are written to.  This information is used during
	// the inlining process to figure out which parameters need to be copied into a temporary variable
	// and which can be passed directly to the inlined code.
	for (vector< Ref<Function> >::iterator i = m_linker->GetFunctions().begin(); i != m_linker->GetFunctions().end(); i++)
		(*i)->CheckForVariableWrites();

	// Collect function inlining candidates.  Any function that is only called once, and is not
	// referenced using a pointer, will be inlined into the calling function.
	map<Function*, Function*> functionCaller;
	for (vector< Ref<Function> >::iterator i = m_linker->GetFunctions().begin(); i != m_linker->GetFunctions().end(); i++)
	{
		// Don't try to inline functions with variable arguments (it won't work)
		if ((*i)->HasVariableArguments())
			continue;

		// Don't inline functions with a variable sized stack frame (i.e. functions that call alloca)
		if ((*i)->IsVariableSizedStackFrame())
			continue;

		// Don't inline imported or fixed address functions, they are inlined implicitly
		if ((*i)->IsImportedFunction() || (*i)->IsFixedAddress())
			continue;

		functionCaller[*i] = NULL;
	}

	for (vector< Ref<Function> >::iterator i = m_linker->GetFunctions().begin(); i != m_linker->GetFunctions().end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
		{
			for (vector<ILInstruction>::const_iterator k = (*j)->GetInstructions().begin();
				k != (*j)->GetInstructions().end(); k++)
			{
				for (size_t p = 0; p < k->params.size(); p++)
				{
					if (k->params[p].cls != ILPARAM_FUNC)
						continue;

					if (functionCaller.find(k->params[p].function) == functionCaller.end())
					{
						// Function has already been found to not be an inline candidate
						continue;
					}

					if ((k->operation != ILOP_CALL) || (p != 1))
					{
						// Function referenced outside call target, this function cannot
						// be inlined
						functionCaller.erase(k->params[p].function);
						continue;
					}

					if (functionCaller[k->params[p].function] != NULL)
					{
						// Function has been called elsewhere, don't inline it
						functionCaller.erase(k->params[p].function);
						continue;
					}

					if (((*i)->GetApproxStackFrameSize() + k->params[p].function->GetApproxStackFrameSize()) >= 0x80)
					{
						// Combined functions would have a large stack frame, don't inline as it could
						// actually make the code larger
						functionCaller.erase(k->params[p].function);
						continue;
					}

					// Record caller for possible inlining later
					functionCaller[k->params[p].function] = *i;
				}
			}
		}
	}

	// Remove functions that aren't called
	list<Function*> toRemove;
	for (map<Function*, Function*>::iterator i = functionCaller.begin(); i != functionCaller.end(); i++)
	{
		if (i->second == NULL)
			toRemove.push_back(i->first);
	}
	for (list<Function*>::iterator i = toRemove.begin(); i != toRemove.end(); i++)
		functionCaller.erase(*i);

	// Inline any candidate functions, ensuring that leaf nodes get inlined first, so that the inlining results
	// get included if that function gets inlined elsewhere
	while (functionCaller.size() > 0)
	{
		set<Function*> toInline;
		for (map<Function*, Function*>::iterator i = functionCaller.begin(); i != functionCaller.end(); i++)
			toInline.insert(i->first);
		for (map<Function*, Function*>::iterator i = functionCaller.begin(); i != functionCaller.end(); i++)
		{
			// Remove functions that aren't leaf nodes for this iteration
			if (toInline.find(i->second) == toInline.end())
				continue;
			toInline.erase(i->second);
		}

		// Perform inlining
		for (set<Function*>::iterator i = toInline.begin(); i != toInline.end(); i++)
		{
			Function* target = *i;
			Function* caller = functionCaller[*i];
			InlineFunction(caller, target);

			// Once a function is inlined, remove it from the list
			functionCaller.erase(target);
		}

		// Sanity check to ensure forward progress
		if ((toInline.size() == 0) && (functionCaller.size() != 0))
		{
			fprintf(stderr, "warning: function inlining is stuck in a loop\n");
			break;
		}
	}

	// If there are functions that are no longer referenced, remove them
	RemoveUnreferencedSymbols();
}


bool Optimize::OptimizeFunction(Function* func)
{
	if (func->IsImportedFunction() || func->IsFixedAddress())
		return false;

	// Run optimization until nothing more can be done
	bool result = false;
	bool changed = true;
	while (changed)
	{
		changed = false;

		FoldConstants(func);

		PerformControlFlowAnalysis(func);
		if ((m_settings.optimization != OPTIMIZE_DISABLE) && ConsolidateBasicBlocks(func))
			changed = true;
		if ((m_settings.optimization != OPTIMIZE_DISABLE) && OptimizeForNoReturnCalls(func))
			changed = true;

		PerformDataFlowAnalysis(func);
//		if ((m_settings.optimization != OPTIMIZE_DISABLE) && RemoveDeadCode(func))
//			changed = true;

		if (changed)
			result = true;
	}

//	if (RemoveUnusedVariables(func))
//		result = true;

	return result;
}

