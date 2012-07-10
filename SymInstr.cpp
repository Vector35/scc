#include <stdio.h>
#include <stdlib.h>
#include "SymInstr.h"
#include "Struct.h"

using namespace std;


void SymInstrOperand::Print()
{
	switch (type)
	{
	case SYMOPERAND_REG:
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
		case SYMREG_ANY:
			fprintf(stderr, "any");
			break;
		default:
			fprintf(stderr, "reg%u", reg);
			break;
		}
		break;
	case SYMOPERAND_IMMED:
		fprintf(stderr, "%lld", immed);
		break;
	case SYMOPERAND_STACK_VAR:
		fprintf(stderr, "stack%d", reg);
		if (immed != 0)
			fprintf(stderr, "+%lld", immed);
		break;
	case SYMOPERAND_GLOBAL_VAR:
		fprintf(stderr, "global%lld", immed);
		break;
	case SYMOPERAND_BLOCK:
		fprintf(stderr, "%s:%d", func->GetName().c_str(), (int)block->GetIndex());
		break;
	}

	if ((access != SYMOPERAND_READ) && (!SYMREG_IS_SPECIAL_REG(reg)))
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


SymInstr::SymInstr()
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

	if ((operand.access != SYMOPERAND_READ) && (reg == SYMREG_IP))
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


SymInstrBlock::SymInstrBlock(size_t i): m_index(i)
{
}


SymInstrBlock::~SymInstrBlock()
{
	for (vector<SymInstr*>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		delete *i;
}


void SymInstrBlock::ResetDataFlowInfo(size_t bits)
{
	m_defPreserve.Reset(bits, true);
	m_defGenerate.Reset(bits, false);
	m_defReachIn.Reset(bits, false);
	m_defReachOut.Reset(bits, false);
}


void SymInstrBlock::Print()
{
	fprintf(stderr, "%d: entry ", (int)m_index);
	for (set<SymInstrBlock*>::iterator i = m_entryBlocks.begin(); i != m_entryBlocks.end(); i++)
		fprintf(stderr, "%d ", (int)(*i)->m_index);
	fprintf(stderr, ", exit ");
	for (set<SymInstrBlock*>::iterator i = m_exitBlocks.begin(); i != m_exitBlocks.end(); i++)
		fprintf(stderr, "%d ", (int)(*i)->m_index);
	fprintf(stderr, "\n");
	for (size_t i = 0; i < m_instrs.size(); i++)
	{
		fprintf(stderr, "\t[%d] ", (int)i);
		m_instrs[i]->Print();
		fprintf(stderr, "\n");
	}
}


SymInstrFunction::SymInstrFunction()
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
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
				{
					regDefs[k->reg].push_back(bitCount);
					defs.push_back(pair<SymInstrBlock*, size_t>(*i, j));
					k->dataFlowBit = bitCount++;
				}
			}
		}
	}

	for (vector<SymInstrBlock*>::const_iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		(*i)->ResetDataFlowInfo(bitCount);
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
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
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
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_WRITE) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
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
				if ((k->type == SYMOPERAND_REG) && (k->access != SYMOPERAND_READ) && (!SYMREG_IS_SPECIAL_REG(k->reg)))
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

	m_regDefs = regDefs;
	m_defLocs = defs;
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
	PerformDataFlowAnalysis();

	return true;
}


void SymInstrFunction::Print()
{
	for (size_t i = 0; i < m_symRegClass.size(); i++)
	{
		fprintf(stderr, "\treg%d: ", (int)i);
		PrintRegisterClass(m_symRegClass[i]);
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
		fprintf(stderr, "\n");
	}

	for (size_t i = 0; i < m_stackVarOffsets.size(); i++)
		fprintf(stderr, "\tstack%d: %lld\n", (int)i, m_stackVarOffsets[i]);

	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		(*i)->Print();
}

