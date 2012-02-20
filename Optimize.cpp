#include <queue>
#include <list>
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

				if (!visited.count(dest))
				{
					toProcess.push(dest);
					visited.insert(dest);
				}
			}
		}
	}
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


void Optimize::InlineFunction(Function* func, Function* target)
{
}


void Optimize::PerformGlobalOptimizations()
{
	if (m_settings.optimization == OPTIMIZE_DISABLE)
		return;

	// Collect function inlining candidates.  Any function that is only called once, and is not
	// referenced using a pointer, will be inlined into the calling function.
	map<Function*, Function*> functionCaller;
	for (vector< Ref<Function> >::iterator i = m_linker->GetFunctions().begin(); i != m_linker->GetFunctions().end(); i++)
		functionCaller[*i] = NULL;

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
}


void Optimize::OptimizeFunction(Function* func)
{
	// Run optimization until nothing more can be done
	bool changed = true;
	while (changed)
	{
		changed = false;

		PerformControlFlowAnalysis(func);
		if ((m_settings.optimization != OPTIMIZE_DISABLE) && ConsolidateBasicBlocks(func))
			changed = true;

//		PerformDataFlowAnalysis(func);
//		if ((m_settings.optimization != OPTIMIZE_DISABLE) && RemoveDeadCode(func))
//			changed = true;
	}

//	RemoveUnusedVariables(func);
}

