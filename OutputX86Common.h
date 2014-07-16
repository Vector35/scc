// Copyright (c) 2011-2012 Rusty Wagner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifdef OUTPUT_CLASS_NAME

#include <map>
#include "Output.h"
#include "SymInstr.h"


#ifdef WIN32
typedef ptrdiff_t ssize_t;
#endif

#ifdef OUTPUT32
class X86Sym_Function;
#else
class X64Sym_Function;
#endif


class OUTPUT_CLASS_NAME: public Output
{
	struct X86MemoryReference
	{
		uint32_t base, index;
		uint8_t scale;
		uint32_t var;
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
				uint32_t reg;
				uint32_t highReg;
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

	enum RegisterUsageType
	{
		USAGE_NORMAL,
		USAGE_INDEX
	};

	struct IncomingParameterCopy
	{
		Variable* var;
		uint32_t incomingReg;
		uint32_t stackVar;
	};

	Function* m_func;
#ifdef OUTPUT32
	X86Sym_Function* m_symFunc;
#else
	X64Sym_Function* m_symFunc;
#endif
	std::map<Variable*, int32_t> m_stackFrame;
	std::map<Variable*, int32_t> m_stackVar;
	std::map<Variable*, uint32_t> m_varReg;
	bool m_framePointerEnabled;
	bool m_normalStack;
	ILBlock* m_currentBlock;
	uint32_t m_varargStart;

	uint32_t GetRegisterByName(const std::string& name);

	void GetDataAddressFromInstructionPointer(SymInstrBlock* out, uint32_t reg, int64_t offset);
	void GetCodeAddressFromInstructionPointer(SymInstrBlock* out, uint32_t reg, Function* func, ILBlock* block);
	bool AccessVariableStorage(SymInstrBlock* out, const ILParameter& param, OperandReference& ref);
	bool LoadCodePointer(SymInstrBlock* out, Function* func, ILBlock* block, OperandReference& ref);
	bool PrepareLoad(SymInstrBlock* out, const ILParameter& param, OperandReference& ref);
	bool PrepareStore(SymInstrBlock* out, const ILParameter& param, OperandReference& ref);

	bool Move(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Add(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Sub(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool And(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Or(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Xor(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool ShiftLeft(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool ShiftRightUnsigned(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool ShiftRightSigned(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src);
	bool Neg(SymInstrBlock* out, const OperandReference& dest);
	bool Not(SymInstrBlock* out, const OperandReference& dest);
	bool Increment(SymInstrBlock* out, const OperandReference& dest);
	bool Decrement(SymInstrBlock* out, const OperandReference& dest);
	void ConditionalJump(SymInstrBlock* out, ConditionalJumpType type, ILBlock* trueBlock, ILBlock* falseBlock);
	void UnconditionalJump(SymInstrBlock* out, ILBlock* block, bool canOmit = true);
#ifdef OUTPUT32
	bool Mult64(SymInstrBlock* out, const OperandReference& dest, const OperandReference& a, const OperandReference& b);
#endif

	bool GenerateAssign(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateAddressOf(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateAddressOfMember(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateDeref(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateDerefMember(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateDerefAssign(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateDerefMemberAssign(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateArrayIndex(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateArrayIndexAssign(SymInstrBlock* out, const ILInstruction& instr);
	bool GeneratePtrAdd(SymInstrBlock* out, const ILInstruction& instr);
	bool GeneratePtrSub(SymInstrBlock* out, const ILInstruction& instr);
	bool GeneratePtrDiff(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateAdd(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSub(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSignedMult(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedMult(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSignedDiv(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedDiv(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSignedMod(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedMod(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateAnd(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateOr(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateXor(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateShl(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateShr(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSar(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateNeg(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateNot(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateIfTrue(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateIfLessThan(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateIfLessThanEqual(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateIfBelow(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateIfBelowEqual(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateIfEqual(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateGoto(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateCall(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSignedConvert(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateUnsignedConvert(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateReturn(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateReturnVoid(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateAlloca(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateMemcpy(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateMemset(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateStrlen(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateSyscall(SymInstrBlock* out, const ILInstruction& instr, bool twoDest);
	bool GenerateRdtsc(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateRdtscLow(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateRdtscHigh(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateInitialVararg(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateNextArg(SymInstrBlock* out, const ILInstruction& instr);
	bool GeneratePrevArg(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateByteSwap(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateBreakpoint(SymInstrBlock* out, const ILInstruction& instr);

	bool GenerateCodeBlock(SymInstrBlock* out, ILBlock* block);

public:
	OUTPUT_CLASS_NAME(const Settings& settings, Function* startFunc);

	virtual bool GenerateCode(Function* func);
	virtual TreeNode* GenerateCall(TreeBlock* block, TreeNode* func, size_t fixedParams, const std::vector< Ref<TreeNode> >& params,
		TreeNodeType resultType);
	virtual TreeNode* GenerateSyscall(TreeBlock* block, TreeNode* num, const std::vector< Ref<TreeNode> >& params,
		TreeNodeType resultType);
};


#endif

