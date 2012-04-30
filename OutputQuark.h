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
#include "asmquark.h"


class OutputQuark: public Output
{
	enum OperandReferenceType
	{
		OPERANDREF_REG,
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
				int reg;
				int highReg;
			};
			int64_t immed;
		};

		bool operator==(const OperandReference& ref) const;
		bool operator!=(const OperandReference& ref) const;
	};

	struct MemoryReference
	{
		int base;
		int32_t offset;
	};

	std::map<Variable*, int32_t> m_stackFrame;
	uint32_t m_stackFrameSize;
	bool m_framePointerEnabled;
	int m_stackPointer, m_origStackPointer;
	int m_framePointer, m_origFramePointer;
	ILBlock* m_currentBlock;

	int m_temporaryRegisters[32];
	size_t m_maxTemporaryRegisters;
	bool m_alloc[32];
	bool m_reserved[32];

	int AllocateTemporaryRegister(OutputBlock* out);
	void ReserveRegisters(OutputBlock* out, ...);
	void ClearReservedRegisters(OutputBlock* out);
	int GetRegisterByName(const std::string& name);

	bool IsSigned11Bit(int64_t imm);
	bool IsSigned17Bit(int64_t imm);
	bool IsPowerOfTwo(int32_t imm, uint32_t& shiftCount);

	void LoadImm(OutputBlock* out, int dest, int32_t imm);
	void AddImm(OutputBlock* out, int dest, int src, int32_t imm);
	void SubImm(OutputBlock* out, int dest, int src, int32_t imm);

	static void RelativeLoadOverflowHandler(OutputBlock* out, Relocation& reloc);

	bool AccessVariableStorage(OutputBlock* out, const ILParameter& param, MemoryReference& ref);
	bool Load(OutputBlock* out, const ILParameter& param, OperandReference& ref, bool forceReg = false);
	bool Store(OutputBlock* out, const ILParameter& param, OperandReference ref);
	bool Move(OutputBlock* out, const OperandReference& dest, const OperandReference& src);

	void UnconditionalJump(OutputBlock* out, ILBlock* block, bool canOmit = true);
	void ConditionalJump(OutputBlock* out, ILBlock* block, int cond, bool value);

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
	bool GenerateSyscall(OutputBlock* out, const ILInstruction& instr);
	bool GenerateNextArg(OutputBlock* out, const ILInstruction& instr);
	bool GeneratePrevArg(OutputBlock* out, const ILInstruction& instr);
	bool GenerateByteSwap(OutputBlock* out, const ILInstruction& instr);

	bool GenerateCodeBlock(OutputBlock* out, ILBlock* block);

public:
	OutputQuark(const Settings& settings);

	virtual bool GenerateCode(Function* func);
};


#endif

