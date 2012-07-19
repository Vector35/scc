// Copyright (c) 2012 Rusty Wagner
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

#ifndef __OUTPUTQUARK_H__

#include <map>
#include "Output.h"
#include "QuarkSymInstr.h"
#include "asmquark.h"


class OutputQuark: public Output
{
	enum OperandReferenceType
	{
		OPERANDREF_REG,
		OPERANDREF_MEM,
		OPERANDREF_IMMED
	};

	enum MemoryReferenceType
	{
		MEMORYREF_STACK_VAR,
		MEMORYREF_GLOBAL_VAR
	};

	struct OperandReference
	{
		OperandReferenceType type;
		size_t width;
		bool sign;
		union
		{
			struct
			{
				uint32_t reg;
				uint32_t highReg;
			};
			struct
			{
				MemoryReferenceType memType;
				uint32_t base;
				uint32_t var;
				int32_t offset;
				uint32_t scratch;
			};
			int64_t immed;
		};

		bool operator==(const OperandReference& ref) const;
		bool operator!=(const OperandReference& ref) const;
	};

	Function* m_func;
	QuarkSymInstrFunction* m_symFunc;
	std::map<Variable*, int32_t> m_stackFrame;
	std::map<Variable*, int32_t> m_stackVar;
	std::map<Variable*, uint32_t> m_varReg;
	uint32_t m_stackFrameSize;
	bool m_framePointerEnabled;
	ILBlock* m_currentBlock;

	bool IsSigned11Bit(int64_t imm);
	bool IsSigned17Bit(int64_t imm);
	bool IsPowerOfTwo(int32_t imm, uint32_t& shiftCount);

	void LoadImm(SymInstrBlock* out, uint32_t dest, int32_t imm);
	void AddImm(SymInstrBlock* out, uint32_t dest, uint32_t src, int32_t imm);
	void SubImm(SymInstrBlock* out, uint32_t dest, uint32_t src, int32_t imm);

	bool AccessVariableStorage(SymInstrBlock* out, const ILParameter& param, OperandReference& ref);
	bool Load(SymInstrBlock* out, const ILParameter& param, OperandReference& ref, bool forceReg = false);
	bool PrepareStore(SymInstrBlock* out, const ILParameter& param, OperandReference& ref);
	bool LoadIntoRegister(SymInstrBlock* out, const OperandReference& ref, OperandReference& reg);
	bool GetDestRegister(SymInstrBlock* out, const OperandReference& dest, OperandReference& reg);
	bool Move(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src, bool enforceSize = false);

	void UnconditionalJump(SymInstrBlock* out, ILBlock* block, bool canOmit = true);
	void ConditionalJump(SymInstrBlock* out, ILBlock* block, int cond, bool value);

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
	bool GenerateSyscall(SymInstrBlock* out, const ILInstruction& instr, bool twoDest);
	bool GenerateNextArg(SymInstrBlock* out, const ILInstruction& instr);
	bool GeneratePrevArg(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateByteSwap(SymInstrBlock* out, const ILInstruction& instr);
	bool GenerateBreakpoint(SymInstrBlock* out, const ILInstruction& instr);

	bool GenerateCodeBlock(SymInstrBlock* out, ILBlock* block);

public:
	OutputQuark(const Settings& settings);

	virtual bool GenerateCode(Function* func);
};


#endif

