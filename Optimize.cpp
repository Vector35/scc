#include <queue>
#include "Optimize.h"
#include "Struct.h"

using namespace std;


Optimize::Optimize(const Settings& settings): m_settings(settings)
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
	vector<ILBlock*> toRemove;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin() + 1; i != func->GetIL().end(); i++)
	{
		if ((*i)->GetEntryBlocks().size() == 0)
			toRemove.push_back(*i);
	}

	for (vector<ILBlock*>::iterator i = toRemove.begin(); i != toRemove.end(); i++)
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

