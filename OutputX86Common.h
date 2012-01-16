#ifdef OUTPUT_CLASS_NAME

#include <map>
#include "Output.h"
#include "asmx86.h"


class OUTPUT_CLASS_NAME: public Output
{
	struct X86MemoryReference
	{
		asmx86::OperandType base, index;
		uint8_t scale;
		ssize_t offset;
	};

	enum OperandReferenceType
	{
		OPERANDREF_REG,
		OPERANDREF_MEM,
		OPERANDREF_IMMED
	};

	struct OperandReference
	{
		OperandReferenceType type;
		size_t width;
		union
		{
			struct
			{
				asmx86::OperandType reg;
				asmx86::OperandType highReg;
			};
			X86MemoryReference mem;
			int64_t immed;
		};

		bool operator==(const OperandReference& ref) const;
		bool operator!=(const OperandReference& ref) const;
	};

	enum ConditionalJumpType
	{
		CONDJUMP_OVERFLOW = 0,
		CONDJUMP_NOT_OVERFLOW,
		CONDJUMP_BELOW,
		CONDJUMP_ABOVE_EQUAL,
		CONDJUMP_EQUAL,
		CONDJUMP_NOT_EQUAL,
		CONDJUMP_BELOW_EQUAL,
		CONDJUMP_ABOVE,
		CONDJUMP_NEGATIVE,
		CONDJUMP_POSITIVE,
		CONDJUMP_PARITY_EVEN,
		CONDJUMP_PARITY_ODD,
		CONDJUMP_LESS_THAN,
		CONDJUMP_GREATER_EQUAL,
		CONDJUMP_LESS_EQUAL,
		CONDJUMP_GREATER_THAN
	};

	std::map<Variable*, int32_t> m_stackFrame;
	uint32_t m_stackFrameSize;
	bool m_framePointerEnabled;
	asmx86::OperandType m_framePointer;

	size_t m_temporaryCount;
	bool m_reserved[4];

	asmx86::OperandType GetRegisterOfSize(asmx86::OperandType base, size_t size);
	bool IsRegisterValid(asmx86::OperandType reg);
	asmx86::OperandType AllocateTemporaryRegister(OutputBlock* out, size_t size);
	void ReserveRegister(asmx86::OperandType reg);

	static void LeaOverflowHandler(OutputBlock* out, size_t start, size_t offset);
	static void ConditionalJumpOverflowHandler(OutputBlock* out, size_t start, size_t offset);
	static void UnconditionalJumpOverflowHandler(OutputBlock* out, size_t start, size_t offset);

	bool AccessVariableStorage(OutputBlock* out, const ILParameter& param, X86MemoryReference& ref);
	bool LoadCodePointer(OutputBlock* out, ILBlock* block, OperandReference& ref);
	bool PrepareLoad(OutputBlock* out, const ILParameter& param, OperandReference& ref);
	bool PrepareStore(OutputBlock* out, const ILParameter& param, OperandReference& ref);

	bool Move(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Add(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Sub(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool And(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Or(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Xor(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool ShiftLeft(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool ShiftRightUnsigned(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool ShiftRightSigned(OutputBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Neg(OutputBlock* out, const OperandReference& dest);
	bool Not(OutputBlock* out, const OperandReference& dest);
	bool Increment(OutputBlock* out, const OperandReference& dest);
	bool Decrement(OutputBlock* out, const OperandReference& dest);
	void ConditionalJump(OutputBlock* out, ConditionalJumpType type, ILBlock* trueBlock, ILBlock* falseBlock);
	void UnconditionalJump(OutputBlock* out, ILBlock* block);

	bool GenerateAssign(OutputBlock* out, const ILInstruction& instr);
	bool GenerateAddressOf(OutputBlock* out, const ILInstruction& instr);
	bool GenerateAddressOfMember(OutputBlock* out, const ILInstruction& instr);
	bool GenerateDeref(OutputBlock* out, const ILInstruction& instr);
	bool GenerateDerefMember(OutputBlock* out, const ILInstruction& instr);
	bool GenerateDerefAssign(OutputBlock* out, const ILInstruction& instr);
	bool GenerateDerefMemberAssign(OutputBlock* out, const ILInstruction& instr);
	bool GenerateArrayIndex(OutputBlock* out, const ILInstruction& instr);
	bool GenerateArrayIndexAssign(OutputBlock* out, const ILInstruction& instr);
	bool GeneratePtrAdd(OutputBlock* out, const ILInstruction& instr);
	bool GeneratePtrSub(OutputBlock* out, const ILInstruction& instr);
	bool GeneratePtrDiff(OutputBlock* out, const ILInstruction& instr);
	bool GenerateAdd(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSub(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSignedMult(OutputBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedMult(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSignedDiv(OutputBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedDiv(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSignedMod(OutputBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedMod(OutputBlock* out, const ILInstruction& instr);
	bool GenerateAnd(OutputBlock* out, const ILInstruction& instr);
	bool GenerateOr(OutputBlock* out, const ILInstruction& instr);
	bool GenerateXor(OutputBlock* out, const ILInstruction& instr);
	bool GenerateShl(OutputBlock* out, const ILInstruction& instr);
	bool GenerateShr(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSar(OutputBlock* out, const ILInstruction& instr);
	bool GenerateNeg(OutputBlock* out, const ILInstruction& instr);
	bool GenerateNot(OutputBlock* out, const ILInstruction& instr);
	bool GenerateIfTrue(OutputBlock* out, const ILInstruction& instr);
	bool GenerateIfLessThan(OutputBlock* out, const ILInstruction& instr);
	bool GenerateIfLessThanEqual(OutputBlock* out, const ILInstruction& instr);
	bool GenerateIfBelow(OutputBlock* out, const ILInstruction& instr);
	bool GenerateIfBelowEqual(OutputBlock* out, const ILInstruction& instr);
	bool GenerateIfEqual(OutputBlock* out, const ILInstruction& instr);
	bool GenerateGoto(OutputBlock* out, const ILInstruction& instr);
	bool GenerateCall(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSignedConvert(OutputBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedConvert(OutputBlock* out, const ILInstruction& instr);
	bool GenerateReturn(OutputBlock* out, const ILInstruction& instr);
	bool GenerateReturnVoid(OutputBlock* out, const ILInstruction& instr);
	bool GenerateAlloca(OutputBlock* out, const ILInstruction& instr);
	bool GenerateMemcpy(OutputBlock* out, const ILInstruction& instr);
	bool GenerateMemset(OutputBlock* out, const ILInstruction& instr);
	bool GenerateSyscall(OutputBlock* out, const ILInstruction& instr);
	bool GenerateRdtsc(OutputBlock* out, const ILInstruction& instr);
	bool GenerateRdtscLow(OutputBlock* out, const ILInstruction& instr);
	bool GenerateRdtscHigh(OutputBlock* out, const ILInstruction& instr);

	bool GenerateCodeBlock(OutputBlock* out, ILBlock* block);

public:
	OUTPUT_CLASS_NAME(const Settings& settings);

	virtual bool GenerateCode(Function* func);
};


#endif

