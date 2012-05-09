#ifndef __SYMINSTR_H__
#define __SYMINSTR_H__

#include "ILBlock.h"
#include "Function.h"

#define SYMREG_SP   0xfffffff0
#define SYMREG_BP   0xfffffff1
#define SYMREG_LR   0xfffffff2
#define SYMREG_IP   0xfffffff3
#define SYMREG_ANY  0xfffffffe
#define SYMREG_NONE 0xffffffff

#define SYMFLAG_WRITES_FLAGS    1
#define SYMFLAG_CLOBBERS_FLAGS  2
#define SYMFLAG_USES_FLAGS      4
#define SYMFLAG_MEMORY_BARRIER  8
#define SYMFLAG_CONTROL_FLOW    0x10


enum SymInstrOperandType
{
	SYMOPERAND_REG,
	SYMOPERAND_IMMED,
	SYMOPERAND_STACK_VAR,
	SYMOPERAND_GLOBAL_VAR,
	SYMOPERAND_BLOCK
};

enum SymInstrOperandAccess
{
	SYMOPERAND_READ,
	SYMOPERAND_WRITE,
	SYMOPERAND_READ_WRITE
};

class SymInstrBlock;

struct SymInstrOperand
{
	SymInstrOperandType type;
	SymInstrOperandAccess access;
	uint32_t reg;
	int64_t immed;
	Function* func;
	ILBlock* block;

	void Print();
};


class SymInstr
{
protected:
	uint32_t m_operation;
	uint32_t m_flags;
	std::vector<SymInstrOperand> m_operands;

public:
	SymInstr();
	virtual ~SymInstr();

	uint32_t GetOperation() const { return m_operation; }
	uint32_t GetFlags() const { return m_flags; }
	bool IsFlagSet(uint32_t flag) const { return (m_flags & flag) != 0; }
	const std::vector<SymInstrOperand>& GetOperands() const { return m_operands; }

	void SetOperation(uint32_t op) { m_operation = op; }
	void EnableFlag(uint32_t flag) { m_flags |= flag; }
	void SetFlags(uint32_t flags) { m_flags = flags; }

	void AddRegisterOperand(uint32_t reg, SymInstrOperandAccess access);
	void AddReadRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_READ); }
	void AddWriteRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_WRITE); }
	void AddReadWriteRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_READ_WRITE); }
	void AddImmediateOperand(int64_t immed);
	void AddStackVarOperand(uint32_t var, int64_t offset);
	void AddGlobalVarOperand(int64_t dataOffset);
	void AddBlockOperand(Function* func, ILBlock* block);

	virtual void Print() = 0;
};


class SymInstrBlock
{
protected:
	size_t m_index;
	std::vector<SymInstr*> m_instrs;
	std::vector<SymInstrBlock*> m_entryBlocks;
	std::vector<SymInstrBlock*> m_exitBlocks;

public:
	SymInstrBlock(size_t i);
	virtual ~SymInstrBlock();

	size_t GetIndex() const { return m_index; }
	const std::vector<SymInstr*>& GetInstructions() const { return m_instrs; }
	void AddInstruction(SymInstr* instr) { m_instrs.push_back(instr); }

	const std::vector<SymInstrBlock*>& GetEntryBlocks() const { return m_entryBlocks; }
	const std::vector<SymInstrBlock*>& GetExitBlocks() const { return m_exitBlocks; }
	void AddEntryBlock(SymInstrBlock* block) { m_entryBlocks.push_back(block); }
	void AddExitBlock(SymInstrBlock* block) { m_exitBlocks.push_back(block); }

	virtual void Print();
};


class SymInstrFunction
{
protected:
	std::vector<SymInstrBlock*> m_blocks;
	std::vector<uint32_t> m_symRegClass;
	std::vector<uint32_t> m_symRegVar;
	std::vector<uint32_t> m_symRegAssignment;
	std::vector<int64_t> m_stackVarOffsets;

public:
	SymInstrFunction();
	virtual ~SymInstrFunction();

	SymInstrBlock* AddBlock(ILBlock* il);
	void AddExitBlock(SymInstrBlock* block, SymInstrBlock* exitBlock);

	uint32_t AddRegister(uint32_t cls, uint32_t var = SYMREG_NONE);
	void AssignRegister(uint32_t reg, uint32_t native);
	const std::vector<uint32_t>& GetRegisters() const { return m_symRegClass; }
	uint32_t GetRegisterClass(uint32_t i) const { return m_symRegClass[i]; }

	uint32_t AddStackVar(int64_t offset);
	const std::vector<int64_t>& GetStackVars() const { return m_stackVarOffsets; }

	virtual void PrintRegisterClass(uint32_t cls) = 0;
	virtual void Print();
};


#endif

