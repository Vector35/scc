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


void SymInstrBlock::Print()
{
	fprintf(stderr, "%d:\n", (int)m_index);
	for (vector<SymInstr*>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
	{
		fprintf(stderr, "\t");
		(*i)->Print();
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


SymInstrBlock* SymInstrFunction::AddBlock(ILBlock* il)
{
	SymInstrBlock* block = new SymInstrBlock(il->GetIndex());
	m_blocks.push_back(block);
	return block;
}


void SymInstrFunction::AddExitBlock(SymInstrBlock* block, SymInstrBlock* exitBlock)
{
	block->AddExitBlock(exitBlock);
	exitBlock->AddEntryBlock(block);
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


void SymInstrFunction::Print()
{
	for (size_t i = 0; i < m_symRegClass.size(); i++)
	{
		fprintf(stderr, "\treg%d: ", (int)i);
		PrintRegisterClass(m_symRegClass[i]);
		fprintf(stderr, "\n");
	}

	for (size_t i = 0; i < m_stackVarOffsets.size(); i++)
		fprintf(stderr, "\tstack%d: %lld\n", (int)i, m_stackVarOffsets[i]);

	for (vector<SymInstrBlock*>::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
		(*i)->Print();
}

