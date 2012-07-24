#include <stdio.h>
#include <stdlib.h>
#include "SymInstr.h"
#include "Struct.h"

using namespace std;


SymInstrOperand::SymInstrOperand(): dataFlowBit((size_t)-1)
{
}


void SymInstrOperand::Print(SymInstrFunction* f)
{
	switch (type)
	{
	case SYMOPERAND_REG:
		f->PrintRegister(reg);
		break;
	case SYMOPERAND_IMMED:
		fprintf(stderr, "%lld", (long long)immed);
		break;
	case SYMOPERAND_STACK_VAR:
		fprintf(stderr, "stack%d", reg);
		if (immed != 0)
			fprintf(stderr, "+%lld", (long long)immed);
		break;
	case SYMOPERAND_GLOBAL_VAR:
		fprintf(stderr, "global%lld", (long long)immed);
		break;
	case SYMOPERAND_BLOCK:
		fprintf(stderr, "%s:%d", func->GetName().c_str(), (int)block->GetIndex());
		break;
	}

	if ((access != SYMOPERAND_READ) && (access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(reg)) && (dataFlowBit != (size_t)-1))
		fprintf(stderr, "{%d}", (int)dataFlowBit);

	if (useDefChain.size() != 0)
	{
		fprintf(stderr, "{defs ");
		for (vector<size_t>::iterator i = useDefChain.begin(); i != useDefChain.end(); i++)
		{
			fprintf(stderr, "%d", (int)*i);
			if ((i + 1) != useDefChain.end())
				fprintf(stderr, ",");
		}
		fprintf(stderr, "}");
	}
}


SymInstr::SymInstr(): m_flags(0)
{
}


SymInstr::~SymInstr()
{
}


void SymInstr::AddRegisterOperand(uint32_t reg, SymInstrOperandAccess access)
{
	SymInstrOperand operand;
	operand.type = SYMOPERAND_REG;
	operand.access = access;
	operand.reg = reg;
	m_operands.push_back(operand);

	if ((operand.access != SYMOPERAND_READ) && (operand.access != SYMOPERAND_IGNORED) && (reg == SYMREG_IP))
		EnableFlag(SYMFLAG_CONTROL_FLOW);
}


void SymInstr::AddImmediateOperand(int64_t immed)
{
	SymInstrOperand operand;
	operand.type = SYMOPERAND_IMMED;
	operand.access = SYMOPERAND_READ;
	operand.immed = immed;
	m_operands.push_back(operand);
}


void SymInstr::AddStackVarOperand(uint32_t var, int64_t offset)
{
	SymInstrOperand operand;
	operand.type = SYMOPERAND_STACK_VAR;
	operand.access = SYMOPERAND_READ;
	operand.reg = var;
	operand.immed = offset;
	m_operands.push_back(operand);
}


void SymInstr::AddGlobalVarOperand(int64_t dataOffset)
{
	SymInstrOperand operand;
	operand.type = SYMOPERAND_GLOBAL_VAR;
	operand.access = SYMOPERAND_READ;
	operand.immed = dataOffset;
	m_operands.push_back(operand);
}


void SymInstr::AddBlockOperand(Function* func, ILBlock* block)
{
	SymInstrOperand operand;
	operand.type = SYMOPERAND_BLOCK;
	operand.access = SYMOPERAND_READ;
	operand.func = func;
	operand.block = block;
	m_operands.push_back(operand);
}


bool SymInstr::UpdateInstruction(SymInstrFunction* func, const Settings& settings, vector<SymInstr*>& replacement)
{
	return false;
}


SymInstrBlock::SymInstrBlock(size_t i): m_index(i)
{
}


SymInstrBlock::~SymInstrBlock()
{
	for (vector<SymInstr*>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		delete *i;
}


void SymInstrBlock::ReplaceInstruction(size_t i, const std::vector<SymInstr*>& replacement)
{
	delete m_instrs[i];
	m_instrs.erase(m_instrs.begin() + i);
	m_instrs.insert(m_instrs.begin() + i, replacement.begin(), replacement.end());
}


void SymInstrBlock::ResetDataFlowInfo(size_t defs, size_t regs)
{
	m_defPreserve.Reset(defs, true);
	m_defGenerate.Reset(defs, false);
	m_defReachIn.Reset(defs, false);
	m_defReachOut.Reset(defs, false);
	m_liveDef.Reset(regs, false);
	m_liveUse.Reset(regs, false);
	m_liveIn.Reset(regs, false);
	m_liveOut.Reset(regs, false);

	for (vector<SymInstr*>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		for (vector<SymInstrOperand>::iterator j = (*i)->GetOperands().begin(); j != (*i)->GetOperands().end(); j++)
			j->useDefChain.clear();
}


bool SymInstrBlock::IsRegisterLiveAtDefinition(uint32_t reg, size_t instr)
{
	// The register is always live for the instruction it is defined at
	for (vector<SymInstrOperand>::iterator i = m_instrs[instr]->GetOperands().begin(); i != m_instrs[instr]->GetOperands().end(); i++)
	{
		if ((i->type == SYMOPERAND_REG) && (i->access != SYMOPERAND_READ) && (i->access != SYMOPERAND_IGNORED) && (i->reg == reg))
			return true;
	}

	// Determine if there is a definition of this register before the requested instruction
	bool hasDefBeforeInstr = false;
	if (m_liveIn.GetBit(reg))
		hasDefBeforeInstr = true;
	else
	{
		for (size_t i = 0; i < instr; i++)
		{
			for (vector<SymInstrOperand>::iterator j = m_instrs[i]->GetOperands().begin(); j != m_instrs[i]->GetOperands().end(); j++)
			{
				if ((j->type == SYMOPERAND_REG) && (j->access != SYMOPERAND_READ) &&
					(j->access != SYMOPERAND_TEMPORARY) && (j->access != SYMOPERAND_IGNORED) && (j->reg == reg))
					hasDefBeforeInstr = true;
			}
		}
	}

	if (!hasDefBeforeInstr)
	{
		// Register is not defined, so it cannot be live
		return false;
	}

	// Determine if the definition is still live after the instruction (that is, it is used before being defined again)
	for (size_t i = instr + 1; i < m_instrs.size(); i++)
	{
		bool written = false;

		for (vector<SymInstrOperand>::iterator j = m_instrs[i]->GetOperands().begin(); j != m_instrs[i]->GetOperands().end(); j++)
		{
			if ((j->type == SYMOPERAND_REG) && (j->access != SYMOPERAND_WRITE) &&
				(j->access != SYMOPERAND_TEMPORARY) && (j->access != SYMOPERAND_IGNORED) && (j->reg == reg))
			{
				// Register is being used before another definition, it is live
				return true;
			}

			if ((j->type == SYMOPERAND_REG) && (j->access != SYMOPERAND_READ) &&
				(j->access != SYMOPERAND_TEMPORARY) && (j->access != SYMOPERAND_IGNORED) && (j->reg == reg))
				written = true;
		}

		if (written)
		{
			// Register is being defined before the original definition was used, register is not live at
			// the instruction requested
			return false;
		}
	}

	// Reached end of block, use block level liveness data from the data flow analysis
	return m_liveOut.GetBit(reg);
}


bool SymInstrBlock::EmitCode(SymInstrFunction* func, OutputBlock* out)
{
	for (vector<SymInstr*>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
	{
		if (!(*i)->EmitInstruction(func, out))
			return false;
	}
	return true;
}


void SymInstrBlock::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%d: entry ", (int)m_index);
	for (set<SymInstrBlock*>::iterator i = m_entryBlocks.begin(); i != m_entryBlocks.end(); i++)
		fprintf(stderr, "%d ", (int)(*i)->m_index);
	fprintf(stderr, ", exit ");
	for (set<SymInstrBlock*>::iterator i = m_exitBlocks.begin(); i != m_exitBlocks.end(); i++)
		fprintf(stderr, "%d ", (int)(*i)->m_index);

	size_t count = 0;
	for (size_t i = 0; i < m_liveOut.GetSize(); i++)
	{
		if (m_liveIn.GetBit(i))
			count++;
	}
	if (count > 0)
	{
		fprintf(stderr, ", live start ");
		for (size_t i = 0; i < m_liveIn.GetSize(); i++)
		{
			if (m_liveIn.GetBit(i))
			{
				func->PrintRegister((uint32_t)i);
				fprintf(stderr, " ");
			}
		}
	}

	count = 0;
	for (size_t i = 0; i < m_liveOut.GetSize(); i++)
	{
		if (m_liveOut.GetBit(i))
			count++;
	}
	if (count > 0)
	{
		fprintf(stderr, ", live end ");
		for (size_t i = 0; i < m_liveOut.GetSize(); i++)
		{
			if (m_liveOut.GetBit(i))
			{
				func->PrintRegister((uint32_t)i);
				fprintf(stderr, " ");
			}
		}
	}

	fprintf(stderr, "\n");
	for (size_t i = 0; i < m_instrs.size(); i++)
	{
		fprintf(stderr, "\t[%d] ", (int)i);
		m_instrs[i]->Print(func);
		fprintf(stderr, "\n");
	}
}


SymInstrFunction::SymInstrFunction(const Settings& settings): m_settings(settings)
{
}


SymInstrFunction::~SymInstrFunction()
{
	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		delete *i;
}


void SymInstrFunction::PerformDataFlowAnalysis()
{
	// Find all definitions of all registers and organize them by register
	map< uint32_t, vector<size_t> > regDefs;
	vector< pair<SymInstrBlock*, size_t> > defs;
	size_t bitCount = 0;
	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
	{
		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			for (vector<SymInstrOperand>::iterator k = (*i)->GetInstructions()[j]->GetOperands().begin();
				k != (*i)->GetInstructions()[j]->GetOperands().end(); k++)
			{
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) &&
					(k->access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					regDefs[k->reg].push_back(bitCount);
					defs.push_back(pair<SymInstrBlock*, size_t>(*i, j));
					k->dataFlowBit = bitCount++;
				}
			}
		}
	}

	for (vector<SymInstrBlock*>::const_iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		(*i)->ResetDataFlowInfo(bitCount, m_symRegClass.size());
	m_exitReachingDefs.Reset(bitCount, false);
	m_defUseChains.clear();
	m_defUseChains.resize(bitCount);

	// Populate definition preservation and generation information
	map< uint32_t, size_t > regDef;
	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
	{
		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			for (vector<SymInstrOperand>::iterator k = (*i)->GetInstructions()[j]->GetOperands().begin();
				k != (*i)->GetInstructions()[j]->GetOperands().end(); k++)
			{
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) &&
					(k->access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					// Kill other definitions of this register
					if (regDef.find(k->reg) != regDef.end())
					{
						// Variable already defined in this block, kill the old definition
						(*i)->GetGeneratedDefinitions().SetBit(regDef[k->reg], false);
					}

					for (vector<size_t>::iterator x = regDefs[k->reg].begin(); x != regDefs[k->reg].end(); x++)
						(*i)->GetPreservedDefinitions().SetBit(*x, false);

					regDef[k->reg] = k->dataFlowBit;
					(*i)->GetGeneratedDefinitions().SetBit(k->dataFlowBit, true);
				}
			}
		}
	}

	// Iteratively compute reaching definitions at the block level
	BitVector tempVector;
	tempVector.Reset(bitCount, false);
	bool changed = true;
	while (changed)
	{
		changed = false;

		for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		{
			// Add definitions from output of all predecessor blocks to the input of the current block
			tempVector = (*i)->GetReachingDefinitionsInput();
			for (set<SymInstrBlock*>::const_iterator j = (*i)->GetEntryBlocks().begin();
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
		tempVector = m_exitReachingDefs;
		for (set<SymInstrBlock*>::iterator i = m_exitBlocks.begin(); i != m_exitBlocks.end(); i++)
			tempVector.Union((*i)->GetReachingDefinitionsOutput());
		if (tempVector != m_exitReachingDefs)
		{
			changed = true;
			m_exitReachingDefs = tempVector;
		}
	}

	// Use reaching definitions information to generate definition->use and use->definition chains
	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
	{
		// Copy reaching definitions into temporary vector, as it will be updated per-instruction to generate
		// localized information
		tempVector = (*i)->GetReachingDefinitionsInput();

		// For each instruction, iterate over all register uses and update information
		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			for (vector<SymInstrOperand>::iterator k = (*i)->GetInstructions()[j]->GetOperands().begin();
				k != (*i)->GetInstructions()[j]->GetOperands().end(); k++)
			{
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_WRITE) &&
					(k->access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					// Add reachable definitions to the use->definition and definition->use chains
					for (vector<size_t>::iterator n = regDefs[k->reg].begin(); n != regDefs[k->reg].end(); n++)
					{
						if (tempVector.GetBit(*n))
						{
							// Definition is reachable
							k->useDefChain.push_back(*n);
							m_defUseChains[*n].push_back(pair<SymInstrBlock*, size_t>(*i, j));
						}
					}
				}
			}

			for (vector<SymInstrOperand>::const_iterator k = (*i)->GetInstructions()[j]->GetOperands().begin();
				k != (*i)->GetInstructions()[j]->GetOperands().end(); k++)
			{
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) &&
					(k->access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					// Instruction is a definition, update reaching definitions information according to the
					// register being written
					for (vector<size_t>::iterator n = regDefs[k->reg].begin(); n != regDefs[k->reg].end(); n++)
						tempVector.SetBit(*n, false);

					// Set the bit for this definition
					tempVector.SetBit(k->dataFlowBit, true);
				}
			}
		}
	}

	// Save definition information
	m_regDefs = regDefs;
	m_defLocs = defs;

	// Populate liveness analysis information
	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
	{
		for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
		{
			// Check uses of registers
			for (vector<SymInstrOperand>::iterator k = (*i)->GetInstructions()[j]->GetOperands().begin();
				k != (*i)->GetInstructions()[j]->GetOperands().end(); k++)
			{
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_WRITE) && (k->access != SYMOPERAND_TEMPORARY) &&
					(k->access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					if (!(*i)->GetLiveDefinitions().GetBit(k->reg))
					{
						// Used before definition
						(*i)->GetLiveUses().SetBit(k->reg, true);
					}
				}
			}

			// Check definitions of registers
			for (vector<SymInstrOperand>::iterator k = (*i)->GetInstructions()[j]->GetOperands().begin();
				k != (*i)->GetInstructions()[j]->GetOperands().end(); k++)
			{
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) && (k->access != SYMOPERAND_TEMPORARY) &&
					(k->access != SYMOPERAND_IGNORED) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					if (!(*i)->GetLiveUses().GetBit(k->reg))
					{
						// Defined before use
						(*i)->GetLiveDefinitions().SetBit(k->reg, true);
					}
				}
			}
		}
	}

	// Iteratively compute liveness at the block level
	tempVector.Reset(m_symRegClass.size(), false);
	changed = true;
	while (changed)
	{
		changed = false;

		for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		{
			// Add liveness of all successor blocks to the output of the current block
			tempVector.Reset(m_symRegClass.size(), false);
			for (set<SymInstrBlock*>::const_iterator j = (*i)->GetExitBlocks().begin();
				j != (*i)->GetExitBlocks().end(); j++)
				tempVector.Union((*j)->GetLiveInput());
			if (tempVector != (*i)->GetLiveOutput())
			{
				changed = true;
				(*i)->GetLiveOutput() = tempVector;
			}

			// Compute liveness at start of this block ((out - defined) | used)
			tempVector = (*i)->GetLiveOutput();
			tempVector.Difference((*i)->GetLiveDefinitions());
			tempVector.Union((*i)->GetLiveUses());
			if (tempVector != (*i)->GetLiveInput())
			{
				changed = true;
				(*i)->GetLiveInput() = tempVector;
			}
		}
	}
}


void SymInstrFunction::SplitRegisters()
{
	// For each register, check definitions to see if they are fully separated in usage
	for (map< uint32_t, vector<size_t> >::iterator i = m_regDefs.begin(); i != m_regDefs.end(); i++)
	{
		if (i->second.size() <= 1)
		{
			// If there isn't more than one definition, there is nothing to split
			continue;
		}

		// Initialize definitions to be separate
		vector<size_t> defSet;
		size_t sets = i->second.size();
		for (size_t j = 0; j < i->second.size(); j++)
			defSet.push_back(j);

		// Combine definition->use chains that share uses in common.  A read/write register access
		// must force a merge of two definition->use chains.  Any that are not combined after this
		// process are fully separate and can be assigned a new symbolic register.
		for (size_t j = 0; j < i->second.size(); j++)
		{
			// Check all other definitions
			for (size_t k = j + 1; k < i->second.size(); k++)
			{
				// Don't need to check definitions that are already in common
				if (defSet[j] == defSet[k])
					continue;

				// Check for uses in common
				// TODO: Use something better than O(n^2) here?
				bool common = false;
				for (vector< pair<SymInstrBlock*, size_t> >::iterator a = m_defUseChains[i->second[j]].begin();
					a != m_defUseChains[i->second[j]].end(); a++)
				{
					for (vector< pair<SymInstrBlock*, size_t> >::iterator b = m_defUseChains[i->second[k]].begin();
						b != m_defUseChains[i->second[k]].end(); b++)
					{
						if ((a->first == b->first) && (a->second == b->second))
						{
							common = true;
							break;
						}
					}
				}

				// Check for read/write registers
				for (vector< pair<SymInstrBlock*, size_t> >::iterator a = m_defUseChains[i->second[j]].begin();
					a != m_defUseChains[i->second[j]].end(); a++)
				{
					for (vector<SymInstrOperand>::iterator operand = a->first->GetInstructions()[a->second]->GetOperands().begin();
						operand != a->first->GetInstructions()[a->second]->GetOperands().end(); operand++)
					{
						if ((operand->type == SYMOPERAND_REG) && (operand->dataFlowBit == i->second[k]) &&
							(operand->access = SYMOPERAND_READ_WRITE))
							common = true;
					}
				}
				for (vector< pair<SymInstrBlock*, size_t> >::iterator a = m_defUseChains[i->second[k]].begin();
					a != m_defUseChains[i->second[k]].end(); a++)
				{
					for (vector<SymInstrOperand>::iterator operand = a->first->GetInstructions()[a->second]->GetOperands().begin();
						operand != a->first->GetInstructions()[a->second]->GetOperands().end(); operand++)
					{
						if ((operand->type == SYMOPERAND_REG) && (operand->dataFlowBit == i->second[j]) &&
							(operand->access = SYMOPERAND_READ_WRITE))
							common = true;
					}
				}

				if (common)
				{
					// These two definitions have uses in common, merge them into the same set and delete the merged set
					size_t fromSet = defSet[k];
					size_t toSet = defSet[j];
					sets--;
					for (size_t a = 0; a < i->second.size(); a++)
					{
						if (defSet[a] == fromSet)
							defSet[a] = toSet;
						else if (defSet[a] > fromSet)
							defSet[a]--;
					}
				}
			}
		}

		// Create a new register for each additional set of definitions and rewrite its uses
		for (size_t j = 1; j < sets; j++)
		{
			uint32_t fromReg = i->first;
			uint32_t toReg = AddRegister(m_symRegClass[fromReg], m_symRegVar[fromReg]);

			for (size_t k = 0; k < i->second.size(); k++)
			{
				if (defSet[k] != j)
					continue;

				if (m_settings.internalDebug)
					fprintf(stderr, "Splitting reg%d from def %d into reg%d\n", fromReg, (int)i->second[k], toReg);

				SymInstrBlock* defBlock = m_defLocs[i->second[k]].first;
				size_t defInstr = m_defLocs[i->second[k]].second;
				for (vector<SymInstrOperand>::iterator operand = defBlock->GetInstructions()[defInstr]->GetOperands().begin();
					operand != defBlock->GetInstructions()[defInstr]->GetOperands().end(); operand++)
				{
					if ((operand->type == SYMOPERAND_REG) && (operand->reg == fromReg))
						operand->reg = toReg;
				}

				for (vector< pair<SymInstrBlock*, size_t> >::iterator a = m_defUseChains[i->second[k]].begin();
					a != m_defUseChains[i->second[k]].end(); a++)
				{
					for (vector<SymInstrOperand>::iterator operand = a->first->GetInstructions()[a->second]->GetOperands().begin();
						operand != a->first->GetInstructions()[a->second]->GetOperands().end(); operand++)
					{
						if ((operand->type == SYMOPERAND_REG) && (operand->reg == fromReg))
							operand->reg = toReg;
					}
				}
			}
		}
	}
}


SymInstrBlock* SymInstrFunction::AddBlock(ILBlock* il)
{
	SymInstrBlock* block = new SymInstrBlock(il->GetIndex());
	m_blocks.push_back(block);
	m_blockMap[il] = block;
	return block;
}


void SymInstrFunction::AddExitBlock(SymInstrBlock* block, SymInstrBlock* exitBlock)
{
	block->AddExitBlock(exitBlock);
	exitBlock->AddEntryBlock(block);
}


void SymInstrFunction::InitializeBlocks(Function* func)
{
	// First create all blocks
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
		AddBlock(*i);

	// Now update exit block information
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
		for (set<ILBlock*>::const_iterator j = (*i)->GetExitBlocks().begin(); j != (*i)->GetExitBlocks().end(); j++)
			AddExitBlock(GetBlock(*i), GetBlock(*j));
	for (set<ILBlock*>::const_iterator i = func->GetExitBlocks().begin(); i != func->GetExitBlocks().end(); i++)
		m_exitBlocks.insert(GetBlock(*i));
}


SymInstrBlock* SymInstrFunction::GetBlock(ILBlock* il) const
{
	map<ILBlock*, SymInstrBlock*>::const_iterator i = m_blockMap.find(il);
	if (i == m_blockMap.end())
		return NULL;
	return i->second;
}


uint32_t SymInstrFunction::AddRegister(uint32_t cls, uint32_t var)
{
	uint32_t i = (uint32_t)m_symRegClass.size();
	m_symRegClass.push_back(cls);
	m_symRegVar.push_back(var);
	m_symRegAssignment.push_back(SYMREG_NONE); // Will be filled in by register allocator
	return i;
}


void SymInstrFunction::AssignRegister(uint32_t reg, uint32_t native)
{
	m_symRegAssignment[reg] = native;
}


uint32_t SymInstrFunction::AddStackVar(int64_t offset)
{
	uint32_t i = (uint32_t)m_stackVarOffsets.size();
	m_stackVarOffsets.push_back(offset);
	return i;
}


bool SymInstrFunction::AllocateRegisters()
{
	while (true) // May need to spill one or more times
	{
		if (m_settings.internalDebug)
			Print();

		// Perform an initial data flow analysis path so that we can analyze the symbolic register usage and see
		// if there are any that can be split into multiple registers to reduce register pressure when the same
		// register is used for multiple non-overlapping purposes.  These are called "webs" in the literature.
		PerformDataFlowAnalysis();

		if (m_settings.internalDebug)
			Print();

		SplitRegisters();

		// Data flow information may have changed, recalculate it before continuing
		PerformDataFlowAnalysis();

		// Compute register interference graph
		m_regInterference.clear();
		m_regInterference.resize(m_symRegClass.size());

		for (uint32_t i = 0; i < (uint32_t)m_symRegClass.size(); i++)
		{
			// Analyze each definition of the current register
			for (vector<size_t>::iterator j = m_regDefs[i].begin(); j != m_regDefs[i].end(); j++)
			{
				SymInstrBlock* defBlock = m_defLocs[*j].first;
				size_t defInstr = m_defLocs[*j].second;

				// Check all other registers, if a register is live at this definition, the two registers interfere
				for (uint32_t k = 0; k < (uint32_t)m_symRegClass.size(); k++)
				{
					if (i == k)
						continue;

					if (defBlock->IsRegisterLiveAtDefinition(k, defInstr))
					{
						// Register is live at this definition, add to interference graph
						m_regInterference[i].insert(k);
						m_regInterference[k].insert(i);
					}
				}

				// If this is a temporary register usage, register interferes with all other registers used by this instruction
				bool tempUse = false;
				for (vector<SymInstrOperand>::iterator k = defBlock->GetInstructions()[defInstr]->GetOperands().begin();
					k != defBlock->GetInstructions()[defInstr]->GetOperands().end(); k++)
				{
					if ((k->type == SYMOPERAND_REG) && (k->access == SYMOPERAND_TEMPORARY) && (k->reg == i))
					{
						tempUse = true;
						break;
					}
				}

				if (tempUse)
				{
					// Using as temporary, interfere with any other registers, even reads, since the register is being
					// used as part of the instruction itself
					for (vector<SymInstrOperand>::iterator k = defBlock->GetInstructions()[defInstr]->GetOperands().begin();
						k != defBlock->GetInstructions()[defInstr]->GetOperands().end(); k++)
					{
						if ((k->type == SYMOPERAND_REG) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
						{
							m_regInterference[i].insert(k->reg);
							m_regInterference[k->reg].insert(i);
						}
					}
				}
			}

			// Add interference edges to native registers according to the register class
			set<uint32_t> nativeInterference = GetRegisterClassInterferences(m_symRegClass[i]);
			m_regInterference[i].insert(nativeInterference.begin(), nativeInterference.end());
		}

		// For each call instruction, generate interferences with caller saved registers to cause them to be
		// spilled to the stack
		vector<uint32_t> callerSavedRegs = GetCallerSavedRegisters();
		vector<uint32_t> calleeSavedRegs = GetCalleeSavedRegisters();
		for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		{
			for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
			{
				if (!(*i)->GetInstructions()[j]->IsFlagSet(SYMFLAG_CALL))
					continue;

				// Check all registers, if a register is live at this call, generate caller saved register interferences
				for (uint32_t k = 0; k < (uint32_t)m_symRegClass.size(); k++)
				{
					// Skip registers that are defined by the call itself
					bool definedByCall = false;
					for (vector<SymInstrOperand>::iterator n = (*i)->GetInstructions()[j]->GetOperands().begin();
						n != (*i)->GetInstructions()[j]->GetOperands().end(); n++)
					{
						if ((n->type == SYMOPERAND_REG) && (n->access != SYMOPERAND_READ) &&
							(n->access != SYMOPERAND_IGNORED) && (n->reg == k))
						{
							definedByCall = true;
							break;
						}
					}
					if (definedByCall)
						continue;

					if ((*i)->IsRegisterLiveAtDefinition(k, j))
					{
						// Register is live at this call, add caller saved registers to interference graph
						for (vector<uint32_t>::iterator n = callerSavedRegs.begin(); n != callerSavedRegs.end(); n++)
							m_regInterference[k].insert(*n);
					}
				}
			}
		}

		// Initialize register allocation structures
		vector<uint32_t> toProcess;
		vector< set<uint32_t> > pruned = m_regInterference;
		stack<uint32_t> assignmentStack;
		vector<uint32_t> realRegs;
		vector<bool> spill;
		size_t spillCount = 0;
		realRegs.insert(realRegs.end(), callerSavedRegs.begin(), callerSavedRegs.end());
		realRegs.insert(realRegs.end(), calleeSavedRegs.begin(), calleeSavedRegs.end());
		for (uint32_t i = 0; i < (uint32_t)m_symRegClass.size(); i++)
		{
			m_symRegAssignment[i] = SYMREG_NONE;
			toProcess.push_back(i);
			spill.push_back(false);
		}

		// First place all symbolic registers that must be in a specific hardware register onto the assignment stack
		for (size_t i = 0; i < toProcess.size(); i++)
		{
			uint32_t reg = toProcess[i];
			if (IsRegisterClassFixed(m_symRegClass[reg]))
			{
				assignmentStack.push(reg);
				toProcess.erase(toProcess.begin() + i);
				i--;
			}
		}

		// Assign fixed registers before doing anything else
		while (!assignmentStack.empty())
		{
			uint32_t reg = assignmentStack.top();
			assignmentStack.pop();

			// Get the list of available registers for assignment
			vector<uint32_t> available;
			for (vector<uint32_t>::iterator i = realRegs.begin(); i != realRegs.end(); i++)
			{
				// Check edge nodes for conflicting assignments
				bool valid = true;
				for (set<uint32_t>::iterator j = m_regInterference[reg].begin(); j != m_regInterference[reg].end(); j++)
				{
					// Check for native register conflicts
					if (*i == *j)
					{
						valid = false;
						break;
					}

					// Check for interference between symbolic registers
					if ((!SYMREG_IS_SPECIAL_REG(*j)) && (m_symRegAssignment[*j] == *i))
					{
						valid = false;
						break;
					}
				}

				if (valid)
				{
					// Found the correct register
					available.push_back(*i);
					break;
				}
			}

			if (available.size() == 0)
			{
				// Fixed register conflicts, this is a compiler bug
				fprintf(stderr, "error: fixed registers conflict, cannot assign reg%d to ", (int)reg);
				PrintRegisterClass(m_symRegClass[reg]);
				fprintf(stderr, "\n");
				return false;
			}

			m_symRegAssignment[reg] = available[0];
		}

		// Attempt to assign registers by pruning nodes with less edges than the total number of real registers
		while (toProcess.size() > 0)
		{
			uint32_t reg = SYMREG_NONE;

			for (vector<uint32_t>::iterator i = toProcess.begin(); i != toProcess.end(); i++)
			{
				if (pruned[*i].size() < realRegs.size())
				{
					// This node has less edges than the number of real registers, so it is proven that at
					// least one real register will be available
					reg = *i;
					toProcess.erase(i);
					break;
				}
			}

			if (reg == SYMREG_NONE)
			{
				// No potential nodes, pick a node and speculatively allocate.  Pick the one with the smallest
				// sum of the number of uses and definitions, as this has the smallest spill cost (in terms
				// of size, not speed)
				size_t i = 0;
				reg = toProcess[0];

				size_t cost = m_regDefs[reg].size();
				for (vector<size_t>::iterator j = m_regDefs[reg].begin(); j != m_regDefs[reg].end(); j++)
					cost += m_defUseChains[*j].size();

				for (size_t j = 1; j < toProcess.size(); j++)
				{
					size_t curCost = m_regDefs[toProcess[j]].size();
					for (vector<size_t>::iterator k = m_regDefs[toProcess[j]].begin(); k != m_regDefs[toProcess[j]].end(); k++)
						curCost += m_defUseChains[*k].size();

					if (curCost < cost)
					{
						i = j;
						reg = toProcess[j];
						cost = curCost;
					}
				}

				toProcess.erase(toProcess.begin() + i);
			}

			// Add the register to the stack for assignment
			assignmentStack.push(reg);

			// Remove the node from the graph and continue allocation
			for (uint32_t i = 0; i < (uint32_t)m_symRegClass.size(); i++)
				pruned[i].erase(reg);
		}

		while (!assignmentStack.empty())
		{
			uint32_t reg = assignmentStack.top();
			assignmentStack.pop();

			// Get the list of available registers for assignment
			vector<uint32_t> available;
			for (vector<uint32_t>::iterator i = realRegs.begin(); i != realRegs.end(); i++)
			{
				// Check edge nodes for conflicting assignments
				bool valid = true;
				for (set<uint32_t>::iterator j = m_regInterference[reg].begin(); j != m_regInterference[reg].end(); j++)
				{
					// Check for native register conflicts
					if (*i == *j)
					{
						valid = false;
						break;
					}

					// Check for interference between symbolic registers
					if ((!SYMREG_IS_SPECIAL_REG(*j)) && (m_symRegAssignment[*j] == *i))
					{
						valid = false;
						break;
					}
				}

				if (!valid)
				{
					// Register not available
					continue;
				}

				// Register available, add to set of available registers
				available.push_back(*i);

				if (!m_settings.polymorph)
				{
					// Not generating polymorphic code, so exit early once a register is found
					break;
				}
			}

			if (available.size() == 0)
			{
				// Register allocation failed, need to spill registers to reduce register pressure
				if (m_settings.internalDebug)
					fprintf(stderr, "Spilling reg%d\n", reg);
				spill[reg] = true;
				spillCount++;

				// Do not assign a register, just continue processing to find other registers that
				// might need to get spilled
				continue;
			}

			// Register is available, allocate it now
			uint32_t choice;
			if (m_settings.polymorph)
				choice = available[rand() % available.size()];
			else
				choice = available[0];
			m_symRegAssignment[reg] = choice;
		}

		if (spillCount > 0)
		{
			// Registers were spilled, need to emit spill code
			fprintf(stderr, "error: out of registers\n");
			return false;
		}

		// Determine set of callee saved registers that are clobbered
		m_clobberedCalleeSavedRegs.clear();
		for (vector<uint32_t>::iterator i = calleeSavedRegs.begin(); i != calleeSavedRegs.end(); i++)
		{
			for (size_t j = 0; j < m_symRegClass.size(); j++)
			{
				if (m_symRegAssignment[j] == *i)
				{
					m_clobberedCalleeSavedRegs.push_back(*i);
					break;
				}
			}
		}

		// Register allocation complete, replace symbolic registers with the final register choices
		for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		{
			for (vector<SymInstr*>::iterator j = (*i)->GetInstructions().begin(); j != (*i)->GetInstructions().end(); j++)
			{
				for (vector<SymInstrOperand>::iterator k = (*j)->GetOperands().begin(); k != (*j)->GetOperands().end(); k++)
				{
					if (k->type != SYMOPERAND_REG)
						continue;
					if (k->reg == SYMREG_NONE)
						continue;

					uint32_t newReg;
					if (k->reg >= SYMREG_MIN_SPECIAL_REG)
						newReg = GetSpecialRegisterAssignment(k->reg);
					else if (k->reg < m_symRegClass.size())
						newReg = m_symRegAssignment[k->reg];

					if (newReg == SYMREG_NONE)
					{
						fprintf(stderr, "error: register assignment for ");
						PrintRegister(k->reg);
						fprintf(stderr, " is not valid\n");
						return false;
					}

					k->reg = newReg;
				}
			}
		}

		// Symbolic registers are no longer used, free unused information
		for (vector<SymInstrBlock*>::const_iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
			(*i)->ResetDataFlowInfo(0, 0);
		m_symRegClass.clear();
		m_symRegVar.clear();
		m_symRegAssignment.clear();
		m_regDefs.clear();
		m_regInterference.clear();
		m_exitReachingDefs.Reset(0, false);
		m_defUseChains.clear();

		// Replace instructions with the final forms.  For example, this step will generate code for saving and
		// restoring callee saved registers onto the stack frame.
		for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		{
			for (size_t j = 0; j < (*i)->GetInstructions().size(); j++)
			{
				vector<SymInstr*> replacement;
				if ((*i)->GetInstructions()[j]->UpdateInstruction(this, m_settings, replacement))
				{
					(*i)->ReplaceInstruction(j, replacement);
					j += replacement.size() - 1;
				}
			}
		}

		AdjustStackFrame();
		return true;
	}
}


void SymInstrFunction::Print()
{
	for (size_t i = 0; i < m_symRegClass.size(); i++)
	{
		fprintf(stderr, "\treg%d: ", (int)i);
		PrintRegisterClass(m_symRegClass[i]);
		if (m_symRegAssignment[i] != SYMREG_NONE)
		{
			fprintf(stderr, " => ");
			PrintRegister(m_symRegAssignment[i]);
		}
		if (m_regDefs.find(i) != m_regDefs.end())
		{
			fprintf(stderr, " (defs: ");
			for (vector<size_t>::iterator j = m_regDefs[i].begin(); j != m_regDefs[i].end(); j++)
			{
				fprintf(stderr, "%d at %d:%d", (int)*j, (int)m_defLocs[*j].first->GetIndex(), (int)m_defLocs[*j].second);

				if (m_defUseChains[*j].size() != 0)
				{
					fprintf(stderr, " {uses ");
					for (vector< pair<SymInstrBlock*, size_t> >::iterator k = m_defUseChains[*j].begin();
						k != m_defUseChains[*j].end(); k++)
					{
						fprintf(stderr, "%d:%d", (int)k->first->GetIndex(), (int)k->second);
						if ((k + 1) != m_defUseChains[*j].end())
							fprintf(stderr, ",");
					}
					fprintf(stderr, "}");
				}

				if ((j + 1) != m_regDefs[i].end())
					fprintf(stderr, ", ");
			}
			fprintf(stderr, ")");
		}
		if ((i < m_regInterference.size()) && (m_regInterference[i].size() != 0))
		{
			fprintf(stderr, " (interference: ");
			for (set<uint32_t>::iterator j = m_regInterference[i].begin(); j != m_regInterference[i].end(); j++)
			{
				if (j != m_regInterference[i].begin())
					fprintf(stderr, ", ");
				PrintRegister(*j);
			}
			fprintf(stderr, ")");
		}
		fprintf(stderr, "\n");
	}

	for (size_t i = 0; i < m_stackVarOffsets.size(); i++)
		fprintf(stderr, "\tstack%d: %lld\n", (int)i, (long long)m_stackVarOffsets[i]);

	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		(*i)->Print(this);
}


void SymInstrFunction::PrintRegister(uint32_t reg)
{
	switch (reg)
	{
	case SYMREG_SP:
		fprintf(stderr, "sp");
		break;
	case SYMREG_BP:
		fprintf(stderr, "bp");
		break;
	case SYMREG_LR:
		fprintf(stderr, "lr");
		break;
	case SYMREG_IP:
		fprintf(stderr, "ip");
		break;
	case SYMREG_NONE:
		fprintf(stderr, "none");
		break;
	default:
		fprintf(stderr, "reg%u", reg);
		break;
	}
}

