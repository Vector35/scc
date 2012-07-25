#ifndef __SYMINSTR_H__
#define __SYMINSTR_H__

#include "ILBlock.h"
#include "Function.h"
#include "BitVector.h"

#define SYMREG_MIN_SPECIAL_REG 0xffffff00
#define SYMREG_SP   0xfffffff0
#define SYMREG_BP   0xfffffff1
#define SYMREG_LR   0xfffffff2
#define SYMREG_IP   0xfffffff3
#define SYMREG_NONE 0xffffffff

#define SYMREG_NATIVE_REG(r) ((r) | 0x80000000)

#define SYMREG_IS_SPECIAL_REG(r) (((r) & 0x80000000) != 0)

#define SYMFLAG_WRITES_FLAGS    1
#define SYMFLAG_CLOBBERS_FLAGS  2
#define SYMFLAG_USES_FLAGS      4
#define SYMFLAG_MEMORY_BARRIER  8
#define SYMFLAG_CONTROL_FLOW    0x10
#define SYMFLAG_CALL            0x20
#define SYMFLAG_COPY            0x40
#define SYMFLAG_STACK           0x80

#define SYMVAR_FRAME_SIZE       0xfffffff0
#define SYMVAR_NEG_FRAME_SIZE   0xfffffff1


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
	SYMOPERAND_READ_WRITE,
	SYMOPERAND_TEMPORARY,
	SYMOPERAND_IGNORED
};

class SymInstrBlock;
class SymInstrFunction;

struct SymInstrOperand
{
	SymInstrOperandType type;
	SymInstrOperandAccess access;
	uint32_t reg;
	int64_t immed;
	Function* func;
	ILBlock* block;
	size_t dataFlowBit;
	std::vector<size_t> useDefChain;

	SymInstrOperand();
	void Print(SymInstrFunction* func);
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
	std::vector<SymInstrOperand>& GetOperands() { return m_operands; }

	void SetOperation(uint32_t op) { m_operation = op; }
	void EnableFlag(uint32_t flag) { m_flags |= flag; }
	void SetFlags(uint32_t flags) { m_flags = flags; }

	void AddRegisterOperand(uint32_t reg, SymInstrOperandAccess access);
	void AddReadRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_READ); }
	void AddWriteRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_WRITE); }
	void AddReadWriteRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_READ_WRITE); }
	void AddTemporaryRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_TEMPORARY); }
	void AddIgnoredRegisterOperand(uint32_t reg) { AddRegisterOperand(reg, SYMOPERAND_IGNORED); }
	void AddImmediateOperand(int64_t immed);
	void AddStackVarOperand(uint32_t var, int64_t offset);
	void AddGlobalVarOperand(int64_t dataOffset);
	void AddBlockOperand(Function* func, ILBlock* block);

	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out) = 0;
	virtual void Print(SymInstrFunction* func) = 0;
};


class SymInstrBlock
{
protected:
	size_t m_index;
	std::vector<SymInstr*> m_instrs;
	std::set<SymInstrBlock*> m_entryBlocks;
	std::set<SymInstrBlock*> m_exitBlocks;

	BitVector m_defPreserve, m_defGenerate, m_defReachIn, m_defReachOut;
	BitVector m_liveDef, m_liveUse, m_liveIn, m_liveOut;

public:
	SymInstrBlock(size_t i);
	virtual ~SymInstrBlock();

	size_t GetIndex() const { return m_index; }
	std::vector<SymInstr*>& GetInstructions() { return m_instrs; }
	void AddInstruction(SymInstr* instr) { m_instrs.push_back(instr); }
	void ReplaceInstruction(size_t i, const std::vector<SymInstr*>& replacement);
	void InsertInstructions(size_t i, const std::vector<SymInstr*>& code);

	const std::set<SymInstrBlock*>& GetEntryBlocks() const { return m_entryBlocks; }
	const std::set<SymInstrBlock*>& GetExitBlocks() const { return m_exitBlocks; }
	void AddEntryBlock(SymInstrBlock* block) { m_entryBlocks.insert(block); }
	void AddExitBlock(SymInstrBlock* block) { m_exitBlocks.insert(block); }

	void ResetDataFlowInfo(size_t defs, size_t regs);
	BitVector& GetPreservedDefinitions() { return m_defPreserve; }
	BitVector& GetGeneratedDefinitions() { return m_defGenerate; }
	BitVector& GetReachingDefinitionsInput() { return m_defReachIn; }
	BitVector& GetReachingDefinitionsOutput() { return m_defReachOut; }
	BitVector& GetLiveDefinitions() { return m_liveDef; }
	BitVector& GetLiveUses() { return m_liveUse; }
	BitVector& GetLiveInput() { return m_liveIn; }
	BitVector& GetLiveOutput() { return m_liveOut; }

	bool IsRegisterLiveAtDefinition(uint32_t reg, size_t instr);

	bool EmitCode(SymInstrFunction* func, OutputBlock* out);

	virtual void Print(SymInstrFunction* func);
};


class SymInstrFunction
{
protected:
	Settings m_settings;
	Function* m_function;

	std::vector<SymInstrBlock*> m_blocks;
	std::map<ILBlock*, SymInstrBlock*> m_blockMap;
	std::vector<uint32_t> m_symRegClass;
	std::vector<ILParameterType> m_symRegType;
	std::vector<uint32_t> m_symRegVar;
	std::vector<int64_t> m_symRegOffset;
	std::vector<uint32_t> m_symRegAssignment;
	std::vector<int64_t> m_stackVarOffsets;
	std::vector<bool> m_stackVarIsParam;
	std::vector<size_t> m_stackVarWidths;
	std::vector<ILParameterType> m_stackVarTypes;
	int64_t m_stackFrameSize;

	std::set<SymInstrBlock*> m_exitBlocks;
	BitVector m_exitReachingDefs;
	std::map< uint32_t, std::vector<size_t> > m_regDefs;
	std::vector< std::pair<SymInstrBlock*, size_t> > m_defLocs;
	std::vector< std::vector< std::pair<SymInstrBlock*, size_t> > > m_defUseChains;
	std::vector< std::set<uint32_t> > m_regInterference;
	std::set<uint32_t> m_alreadySpilled;

	std::vector<uint32_t> m_clobberedCalleeSavedRegs;

	SymInstrBlock* AddBlock(ILBlock* il);
	void AddExitBlock(SymInstrBlock* block, SymInstrBlock* exitBlock);

	void PerformDataFlowAnalysis();
	void SplitRegisters();
	bool SpillRegister(uint32_t reg);

public:
	SymInstrFunction(const Settings& settings, Function* func);
	virtual ~SymInstrFunction();

	void InitializeBlocks(Function* func);
	SymInstrBlock* GetBlock(ILBlock* block) const;

	uint32_t AddRegister(uint32_t cls, ILParameterType type = ILTYPE_VOID, int64_t offset = 0, uint32_t var = SYMREG_NONE);
	void AssignRegister(uint32_t reg, uint32_t native);
	const std::vector<uint32_t>& GetRegisters() const { return m_symRegClass; }
	uint32_t GetRegisterClass(uint32_t i) const { return m_symRegClass[i]; }

	uint32_t AddStackVar(int64_t offset, bool param, size_t width, ILParameterType type);
	int64_t GetStackVarOffset(uint32_t var) const;
	const std::vector<size_t>& GetStackVarWidths() const { return m_stackVarWidths; }
	const std::vector<ILParameterType>& GetStackVarTypes() const { return m_stackVarTypes; }

	const std::vector<uint32_t>& GetClobberedCalleeSavedRegisters() const { return m_clobberedCalleeSavedRegs; }
	const Settings& GetSettings() const { return m_settings; }
	Function* GetFunction() const { return m_function; }

	bool AllocateRegisters();

	virtual std::vector<uint32_t> GetCallerSavedRegisters() = 0;
	virtual std::vector<uint32_t> GetCalleeSavedRegisters() = 0;
	virtual std::set<uint32_t> GetRegisterClassInterferences(uint32_t cls) = 0;
	virtual bool IsRegisterClassFixed(uint32_t cls) = 0;
	virtual uint32_t GetFixedRegisterForClass(uint32_t cls) = 0;
	virtual uint32_t GetSpecialRegisterAssignment(uint32_t reg) = 0;
	virtual bool DoesRegisterClassConflictWithSpecialRegisters(uint32_t cls) = 0;
	virtual size_t GetNativeSize() = 0;

	virtual void LayoutStackFrame() = 0;

	virtual bool GenerateSpillLoad(uint32_t reg, uint32_t var, int64_t offset,
		ILParameterType type, std::vector<SymInstr*>& code) = 0;
	virtual bool GenerateSpillStore(uint32_t reg, uint32_t var, int64_t offset,
		ILParameterType type, std::vector<SymInstr*>& code) = 0;

	virtual void PrintRegisterClass(uint32_t cls) = 0;
	virtual void PrintRegister(uint32_t reg);
	virtual void Print();
};


#endif

