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

#include "OutputQuark.h"
#include "Function.h"
#include "Struct.h"

#define NONE (-1)
#define UNSAFE_STACK_PIVOT 0x1000

#define DEFAULT_STACK_POINTER 0
#define DEFAULT_FRAME_POINTER 29
#define LR                    30
#define IP                    31

#define EMIT(op) (out->WriteUInt32(QUARK_EMIT(op)))
#define EMIT_1(op, a) (out->WriteUInt32(QUARK_EMIT_1(op, a)))
#define EMIT_1I(op, a) (out->WriteUInt32(QUARK_EMIT_1I(op, a)))
#define EMIT_1R(op, a) (out->WriteUInt32(QUARK_EMIT_1R(op, a)))
#define EMIT_2(op, a, b) (out->WriteUInt32(QUARK_EMIT_2(op, a, b)))
#define EMIT_2I(op, a, b) (out->WriteUInt32(QUARK_EMIT_2I(op, a, b)))
#define EMIT_2F(op, a, b) (out->WriteUInt32(QUARK_EMIT_2F(op, a, b)))
#define EMIT_2R(op, a, b, s) (out->WriteUInt32(QUARK_EMIT_2R(op, a, b, s)))
#define EMIT_3(op, a, b, c) (out->WriteUInt32(QUARK_EMIT_3(op, a, b, c)))
#define EMIT_3I(op, a, b, c) (out->WriteUInt32(QUARK_EMIT_3I(op, a, b, c)))
#define EMIT_3F(op, a, b, c) (out->WriteUInt32(QUARK_EMIT_3F(op, a, b, c)))
#define EMIT_3R(op, a, b, c, s) (out->WriteUInt32(QUARK_EMIT_3R(op, a, b, c, s)))
#define EMIT_4(op, a, b, c, d) (out->WriteUInt32(QUARK_EMIT_4(op, a, b, c, d)))
#define EMIT_COND(op, cc) (out->WriteUInt32(QUARK_EMIT_COND(op, cc)))
#define EMIT_COND_1(op, cc, a) (out->WriteUInt32(QUARK_EMIT_COND_1(op, cc, a)))
#define EMIT_COND_1I(op, cc, a) (out->WriteUInt32(QUARK_EMIT_COND_1I(op, cc, a)))
#define EMIT_COND_1R(op, cc, a) (out->WriteUInt32(QUARK_EMIT_COND_1R(op, cc, a)))
#define EMIT_COND_2(op, cc, a, b) (out->WriteUInt32(QUARK_EMIT_COND_2(op, cc, a, b)))
#define EMIT_COND_2I(op, cc, a, b) (out->WriteUInt32(QUARK_EMIT_COND_2I(op, cc, a, b)))
#define EMIT_COND_2F(op, cc, a, b) (out->WriteUInt32(QUARK_EMIT_COND_2F(op, cc, a, b)))
#define EMIT_COND_2R(op, cc, a, b, s) (out->WriteUInt32(QUARK_EMIT_COND_2R(op, cc, a, b, s)))
#define EMIT_COND_3(op, cc, a, b, c) (out->WriteUInt32(QUARK_EMIT_COND_3(op, cc, a, b, c)))
#define EMIT_COND_3I(op, cc, a, b, c) (out->WriteUInt32(QUARK_EMIT_COND_3I(op, cc, a, b, c)))
#define EMIT_COND_3F(op, cc, a, b, c) (out->WriteUInt32(QUARK_EMIT_COND_3F(op, cc, a, b, c)))
#define EMIT_COND_3R(op, cc, a, b, c, s) (out->WriteUInt32(QUARK_EMIT_COND_3R(op, cc, a, b, c, s)))
#define EMIT_COND_4(op, cc, a, b, c, d) (out->WriteUInt32(QUARK_EMIT_COND_4(op, cc, a, b, c, d)))

using namespace std;


OutputQuark::OutputQuark(const Settings& settings): Output(settings)
{
}


int OutputQuark::AllocateTemporaryRegister(OutputBlock* out)
{
	size_t possibleRegs[32];
	size_t possibleRegCount = 0;

	for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
	{
		if ((!m_alloc[i]) && (!m_reserved[i]))
			possibleRegs[possibleRegCount++] = i;
	}

	if (possibleRegCount == 0)
		return NONE;

	if (m_settings.polymorph)
	{
		size_t choice = rand() % possibleRegCount;
		m_alloc[possibleRegs[choice]] = true;
		return m_temporaryRegisters[possibleRegs[choice]];
	}
	else
	{
		m_alloc[possibleRegs[0]] = true;
		return m_temporaryRegisters[possibleRegs[0]];
	}
}


void OutputQuark::ReserveRegisters(OutputBlock* out, ...)
{
	int regs[32];
	size_t regCount;
	va_list va;
	va_start(va, out);

	// Process register list and mark them as reserved
	for (regCount = 0; regCount < 16; regCount++)
	{
		int reg = va_arg(va, int);
		if (reg == NONE)
			break;
		regs[regCount] = reg;

		for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
		{
			if (m_temporaryRegisters[i] == reg)
			{
				m_reserved[i] = true;
				break;
			}
		}
	}

	// Check for register collisions and handle them
	if (out)
	{
		for (size_t i = 0; i < regCount; i++)
		{
			if (regs[i] == m_stackPointer)
			{
				// Caller needs register that currently holds the stack pointer, relocate it to another register
				int temp = AllocateTemporaryRegister(out);
				EMIT_2(xchg, temp, m_stackPointer);
				m_stackPointer = temp;
			}
			else if (regs[i] == m_framePointer)
			{
				// Caller needs register that currently holds the frame pointer, relocate it to another register
				int temp = AllocateTemporaryRegister(out);
				EMIT_2(xchg, temp, m_framePointer);
				m_framePointer = temp;
			}
		}
	}
}


void OutputQuark::ClearReservedRegisters(OutputBlock* out)
{
	if (m_stackPointer != m_origStackPointer)
	{
		// Stack pointer was relocated, move it back
		EMIT_2(xchg, m_stackPointer, m_origStackPointer);
	}

	if (m_framePointer != m_origFramePointer)
	{
		// Frame pointer was relocated, move it back
		EMIT_2(xchg, m_framePointer, m_origFramePointer);
	}

	m_stackPointer = m_origStackPointer;
	m_framePointer = m_origFramePointer;

	for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
		m_reserved[i] = false;
}


int OutputQuark::GetRegisterByName(const std::string& name)
{
	if (name[0] != 'r')
		return NONE;
	return atoi(&name.c_str()[1]);
}


bool OutputQuark::IsSigned10Bit(int32_t imm)
{
	if (imm < -512)
		return false;
	if (imm > 511)
		return false;
	return true;
}


bool OutputQuark::IsSigned16Bit(int32_t imm)
{
	if (imm < -32768)
		return false;
	if (imm > 32767)
		return false;
	return true;
}


void OutputQuark::LoadImm(OutputBlock* out, int dest, int32_t imm)
{
	if (IsSigned16Bit(imm))
	{
		EMIT_2(ldi, dest, imm);
	}
	else
	{
		EMIT_2(ldi, dest, imm);
		EMIT_2(ldih, dest, imm >> 16);
	}
}


void OutputQuark::AddImm(OutputBlock* out, int dest, int src, int32_t imm)
{
	if ((imm == 0) && (dest == src))
		return;

	if (IsSigned10Bit(imm))
	{
		EMIT_3I(add, dest, src, imm);
	}
	else
	{
		int temp = AllocateTemporaryRegister(out);
		LoadImm(out, temp, imm);
		EMIT_3R(add, dest, src, temp, 0);
	}
}


void OutputQuark::SubImm(OutputBlock* out, int dest, int src, int32_t imm)
{
	if ((imm == 0) && (dest == src))
		return;

	if (IsSigned10Bit(imm))
	{
		EMIT_3I(sub, dest, src, imm);
	}
	else
	{
		int temp = AllocateTemporaryRegister(out);
		LoadImm(out, temp, imm);
		EMIT_3R(sub, dest, src, temp, 0);
	}
}


bool OutputQuark::Load(OutputBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
		return false;
	case ILPARAM_MEMBER:
		return false;
	case ILPARAM_INT:
		ref.type = OPERANDREF_IMMED;
		ref.immed = param.integerValue;
		return true;
	case ILPARAM_BOOL:
		ref.type = OPERANDREF_IMMED;
		ref.immed = param.boolValue ? 1 : 0;
		return true;
	case ILPARAM_FUNC:
		return false;
	case ILPARAM_UNDEFINED:
		ref.type = OPERANDREF_REG;
		ref.width = param.GetWidth();
		ref.reg = 1;
		return true;
	default:
		return false;
	}
}


bool OutputQuark::Store(OutputBlock* out, const ILParameter& param, const OperandReference& ref)
{
	return true;
	switch (param.cls)
	{
	case ILPARAM_VAR:
		return false;
	case ILPARAM_MEMBER:
		return false;
	default:
		return false;
	}
}


bool OutputQuark::Move(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	if (src.type == OPERANDREF_IMMED)
	{
		LoadImm(out, dest.reg, (int32_t)src.immed);
		if (dest.width == 8)
			LoadImm(out, dest.highReg, (int32_t)(src.immed >> 32));
	}
	else
	{
		if (dest.reg != src.reg)
			EMIT_2R(mov, dest.reg, src.reg, 0);
		if ((dest.width == 8) && (dest.highReg != src.highReg))
			EMIT_2R(mov, dest.highReg, src.highReg, 0);
	}
	return true;
}


bool OutputQuark::GenerateAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateAddressOf(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateAddressOfMember(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateDeref(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateDerefMember(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateDerefAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateDerefMemberAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateArrayIndex(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateArrayIndexAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GeneratePtrAdd(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GeneratePtrSub(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GeneratePtrDiff(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateAdd(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSub(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSignedMult(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateUnsignedMult(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSignedDiv(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateUnsignedDiv(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSignedMod(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateUnsignedMod(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateAnd(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateOr(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateXor(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateShl(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateShr(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSar(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateNeg(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateNot(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateIfTrue(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateIfLessThan(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateIfLessThanEqual(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateIfBelow(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateIfBelowEqual(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateIfEqual(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateGoto(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateCall(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSignedConvert(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateUnsignedConvert(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateReturn(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateReturnVoid(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateAlloca(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateMemcpy(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateMemset(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateStrlen(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateSyscall(OutputBlock* out, const ILInstruction& instr)
{
	const int regs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, NONE};
	int savedRegs[2];
	size_t savedRegCount = 0;
	bool savedFramePointer = false;

	if (m_stackPointer != DEFAULT_STACK_POINTER)
		EMIT_2(xchg, m_stackPointer, 0);

	ReserveRegisters(NULL, 0, DEFAULT_FRAME_POINTER, NONE);

	size_t regIndex = 0;
	for (size_t i = 2; i < instr.params.size(); i++)
	{
		if (regs[regIndex] == NONE)
			return false;
		if (instr.params[i].cls == ILPARAM_UNDEFINED)
			continue;
		ReserveRegisters(NULL, regs[regIndex], NONE);

		if (instr.params[i].GetWidth() == 8)
		{
			if (regs[regIndex + 1] == NONE)
				return false;
			ReserveRegisters(NULL, regs[regIndex + 1], NONE);
		}

		if ((regs[regIndex] == m_framePointer) && (!savedFramePointer))
		{
			if (m_settings.stackGrowsUp)
			{
				EMIT_3I(add, DEFAULT_STACK_POINTER, DEFAULT_STACK_POINTER, 4);
				EMIT_3I(stw, m_framePointer, DEFAULT_STACK_POINTER, 0);
			}
			else
			{
				EMIT_3I(stwu, m_framePointer, DEFAULT_STACK_POINTER, -4);
			}

			savedRegs[savedRegCount++] = m_framePointer;
			savedFramePointer = true;

			if (m_framePointer != DEFAULT_FRAME_POINTER)
			{
				EMIT_2R(mov, DEFAULT_FRAME_POINTER, m_framePointer, 0);
				m_framePointer = DEFAULT_FRAME_POINTER;
			}
		}

		OperandReference cur;
		if (!Load(out, instr.params[i], cur))
			return false;

		int reg, highReg;
		if (instr.params[i].GetWidth() == 8)
		{
			reg = regs[regIndex++];
			highReg = regs[regIndex++];
		}
		else
		{
			reg = regs[regIndex++];
			highReg = NONE;
		}

		OperandReference dest;
		dest.type = OPERANDREF_REG;
		dest.width = cur.width;
		dest.reg = reg;
		dest.highReg = highReg;

		if (!Move(out, dest, cur))
			return false;
	}

	OperandReference num;
	if (!Load(out, instr.params[1], num))
		return false;

	if (num.type == OPERANDREF_REG)
		EMIT_1R(syscall, num.reg);
	else
		EMIT_1I(syscall, num.immed);

	m_framePointer = m_origFramePointer;

	for (size_t i = 0; i < savedRegCount; i++)
	{
		int reg = savedRegs[(savedRegCount - 1) - i];

		if (m_settings.stackGrowsUp)
		{
			EMIT_3I(ldw, reg, DEFAULT_STACK_POINTER, 0);
			EMIT_3I(sub, DEFAULT_STACK_POINTER, DEFAULT_STACK_POINTER, 4);
		}
		else
		{
			EMIT_3I(ldwu, reg, DEFAULT_STACK_POINTER, 0);
		}
	}

	if (m_stackPointer != DEFAULT_STACK_POINTER)
		EMIT_2(xchg, m_stackPointer, 0);

	OperandReference result;
	result.type = OPERANDREF_REG;
	result.width = 4;
	result.reg = 1;
	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateNextArg(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GeneratePrevArg(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateByteSwap(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateCodeBlock(OutputBlock* out, ILBlock* block)
{
	m_currentBlock = block;

	vector<ILInstruction>::iterator i;
	for (i = block->GetInstructions().begin(); i != block->GetInstructions().end(); i++)
	{
		memset(m_alloc, 0, sizeof(m_alloc));
		ClearReservedRegisters(out);

		bool end = false;
		switch (i->operation)
		{
		case ILOP_ASSIGN:
			if (!GenerateAssign(out, *i))
				goto fail;
			break;
		case ILOP_ADDRESS_OF:
			if (!GenerateAddressOf(out, *i))
				goto fail;
			break;
		case ILOP_ADDRESS_OF_MEMBER:
			if (!GenerateAddressOfMember(out, *i))
				goto fail;
			break;
		case ILOP_DEREF:
			if (!GenerateDeref(out, *i))
				goto fail;
			break;
		case ILOP_DEREF_MEMBER:
			if (!GenerateDerefMember(out, *i))
				goto fail;
			break;
		case ILOP_DEREF_ASSIGN:
			if (!GenerateDerefAssign(out, *i))
				goto fail;
			break;
		case ILOP_DEREF_MEMBER_ASSIGN:
			if (!GenerateDerefMemberAssign(out, *i))
				goto fail;
			break;
		case ILOP_ARRAY_INDEX:
			if (!GenerateArrayIndex(out, *i))
				goto fail;
			break;
		case ILOP_ARRAY_INDEX_ASSIGN:
			if (!GenerateArrayIndexAssign(out, *i))
				goto fail;
			break;
		case ILOP_PTR_ADD:
			if (!GeneratePtrAdd(out, *i))
				goto fail;
			break;
		case ILOP_PTR_SUB:
			if (!GeneratePtrSub(out, *i))
				goto fail;
			break;
		case ILOP_PTR_DIFF:
			if (!GeneratePtrDiff(out, *i))
				goto fail;
			break;
		case ILOP_ADD:
			if (!GenerateAdd(out, *i))
				goto fail;
			break;
		case ILOP_SUB:
			if (!GenerateSub(out, *i))
				goto fail;
			break;
		case ILOP_SMULT:
			if (!GenerateSignedMult(out, *i))
				goto fail;
			break;
		case ILOP_UMULT:
			if (!GenerateUnsignedMult(out, *i))
				goto fail;
			break;
		case ILOP_SDIV:
			if (!GenerateSignedDiv(out, *i))
				goto fail;
			break;
		case ILOP_UDIV:
			if (!GenerateUnsignedDiv(out, *i))
				goto fail;
			break;
		case ILOP_SMOD:
			if (!GenerateSignedMod(out, *i))
				goto fail;
			break;
		case ILOP_UMOD:
			if (!GenerateUnsignedMod(out, *i))
				goto fail;
			break;
		case ILOP_AND:
			if (!GenerateAnd(out, *i))
				goto fail;
			break;
		case ILOP_OR:
			if (!GenerateOr(out, *i))
				goto fail;
			break;
		case ILOP_XOR:
			if (!GenerateXor(out, *i))
				goto fail;
			break;
		case ILOP_SHL:
			if (!GenerateShl(out, *i))
				goto fail;
			break;
		case ILOP_SHR:
			if (!GenerateShr(out, *i))
				goto fail;
			break;
		case ILOP_SAR:
			if (!GenerateSar(out, *i))
				goto fail;
			break;
		case ILOP_NEG:
			if (!GenerateNeg(out, *i))
				goto fail;
			break;
		case ILOP_NOT:
			if (!GenerateNot(out, *i))
				goto fail;
			break;
		case ILOP_IF_TRUE:
			if (!GenerateIfTrue(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_IF_LESS_THAN:
			if (!GenerateIfLessThan(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_IF_LESS_EQUAL:
			if (!GenerateIfLessThanEqual(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_IF_BELOW:
			if (!GenerateIfBelow(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_IF_BELOW_EQUAL:
			if (!GenerateIfBelowEqual(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_IF_EQUAL:
			if (!GenerateIfEqual(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_GOTO:
			if (!GenerateGoto(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_CALL:
			if (!GenerateCall(out, *i))
				goto fail;
			break;
		case ILOP_NORETURN:
			end = true;
			break;
		case ILOP_SCONVERT:
			if (!GenerateSignedConvert(out, *i))
				goto fail;
			break;
		case ILOP_UCONVERT:
			if (!GenerateUnsignedConvert(out, *i))
				goto fail;
			break;
		case ILOP_RETURN:
			if (!GenerateReturn(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_RETURN_VOID:
			if (!GenerateReturnVoid(out, *i))
				goto fail;
			end = true;
			break;
		case ILOP_ALLOCA:
			if (!GenerateAlloca(out, *i))
				goto fail;
			break;
		case ILOP_MEMCPY:
			if (!GenerateMemcpy(out, *i))
				goto fail;
			break;
		case ILOP_MEMSET:
			if (!GenerateMemset(out, *i))
				goto fail;
			break;
		case ILOP_STRLEN:
			if (!GenerateStrlen(out, *i))
				goto fail;
			break;
		case ILOP_SYSCALL:
			if (!GenerateSyscall(out, *i))
				goto fail;
			break;
		case ILOP_NEXT_ARG:
			if (!GenerateNextArg(out, *i))
				goto fail;
			break;
		case ILOP_PREV_ARG:
			if (!GeneratePrevArg(out, *i))
				goto fail;
			break;
		case ILOP_BYTESWAP:
			if (!GenerateByteSwap(out, *i))
				goto fail;
			break;
		default:
			goto fail;
		}

		if (end)
			break;
	}

	return true;

fail:
	fprintf(stderr, "error: unable to generate code for IL: ");
	i->Print();
	fprintf(stderr, "\n");
	return false;
}


bool OutputQuark::GenerateCode(Function* func)
{
	// Determine what the stack and frame pointers should be
	m_framePointer = DEFAULT_FRAME_POINTER;
	m_stackPointer = DEFAULT_STACK_POINTER;
	if (func->IsVariableSizedStackFrame())
		m_framePointerEnabled = true;
	else
		m_framePointerEnabled = false;

	if (m_settings.stackReg.size() != 0)
		m_stackPointer = GetRegisterByName(m_settings.stackReg);
	if (m_settings.frameReg.size() != 0)
		m_framePointer = GetRegisterByName(m_settings.frameReg);

	if (m_stackPointer == NONE)
	{
		fprintf(stderr, "error: invalid stack pointer register\n");
		return false;
	}

	if (m_framePointer == NONE)
	{
		fprintf(stderr, "error: invalid frame pointer register\n");
		return false;
	}

	if (m_stackPointer == m_framePointer)
	{
		fprintf(stderr, "error: stack pointer and frame pointer cannot be the same register\n");
		return false;
	}

	m_origStackPointer = m_stackPointer;
	m_origFramePointer = m_framePointer;

	// Determine which registers can be used as temporaries
	for (int i = 1; i < 29; i++)
		m_temporaryRegisters[i - 1] = i;
	m_maxTemporaryRegisters = 28;

	for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
	{
		if ((m_temporaryRegisters[i] == m_stackPointer) ||
			(m_temporaryRegisters[i] == m_framePointer))
		{
			memmove(&m_temporaryRegisters[i], &m_temporaryRegisters[i + 1], sizeof(int) *
				((m_maxTemporaryRegisters - i) - 1));
			m_maxTemporaryRegisters--;
			i--;
		}
	}

	// Generate stack frame
	uint32_t offset = 0;
	m_stackFrame.clear();
	for (vector< Ref<Variable> >::const_iterator i = func->GetVariables().begin(); i != func->GetVariables().end(); i++)
	{
		if ((*i)->IsParameter())
			continue;

		if ((offset & ((*i)->GetType()->GetAlignment() - 1)) != 0)
			offset += (uint32_t)((*i)->GetType()->GetAlignment() - (offset & ((*i)->GetType()->GetAlignment() - 1)));
		m_stackFrame[*i] = offset;
		offset += (uint32_t)((*i)->GetType()->GetWidth());
	}

	// Ensure stack is aligned to natural boundary
	if (offset & 3)
		offset += 4 - (offset & 3);

	if (m_settings.stackGrowsUp)
	{
		for (vector< Ref<Variable> >::const_iterator i = func->GetVariables().begin(); i != func->GetVariables().end(); i++)
		{
			if ((*i)->IsParameter())
				continue;

#ifdef OUTPUT32
			m_stackFrame[*i] -= offset - 4;
#else
			m_stackFrame[*i] -= offset - 8;
#endif
		}
	}

	m_stackFrameSize = offset;
	if (m_stackFrameSize != 0)
		m_framePointerEnabled = true;

	if (m_framePointerEnabled)
	{
		// Adjust variable offsets to be relative to the frame pointer (negative offsets)
		for (map<Variable*, int32_t>::iterator i = m_stackFrame.begin(); i != m_stackFrame.end(); i++)
		{
			if (m_settings.stackGrowsUp)
				i->second += m_stackFrameSize;
			else
				i->second -= m_stackFrameSize;
		}
	}

	// Generate parameter offsets
	offset = 0;

	if ((func->GetName() == "_start") && (m_settings.unsafeStack))
		offset += UNSAFE_STACK_PIVOT;

	for (size_t i = 0; i < func->GetParameters().size(); i++)
	{
		// Find variable object for this parameter
		vector< Ref<Variable> >::const_iterator var = func->GetVariables().end();
		for (vector< Ref<Variable> >::const_iterator j = func->GetVariables().begin(); j != func->GetVariables().end(); j++)
		{
			if ((*j)->IsParameter() && ((*j)->GetParameterIndex() == i))
			{
				var = j;
				break;
			}
		}

		if (var == func->GetVariables().end())
		{
			// Variable not named, so it won't be referenced
			continue;
		}

		// Allocate stack space for this parameter
		if (m_framePointerEnabled)
			m_stackFrame[*var] = offset + 8;
		else
			m_stackFrame[*var] = offset + m_stackFrameSize + 4;

		if (m_settings.stackGrowsUp)
		{
			m_stackFrame[*var] = -m_stackFrame[*var];
			size_t paramSize = ((*var)->GetType()->GetWidth() + 3) & (~3);
			m_stackFrame[*var] += 4 - paramSize;
		}

		// Adjust offset for next parameter
		offset += (*var)->GetType()->GetWidth();
		if (offset & 3)
			offset += 4 - (offset & 3);
	}

	bool first = true;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		OutputBlock* out = new OutputBlock;
		out->code = NULL;
		out->len = 0;
		out->maxLen = 0;

		if (first)
		{
			memset(m_alloc, 0, sizeof(m_alloc));
			ClearReservedRegisters(out);

			if ((func->GetName() == "_start") && m_settings.unsafeStack)
			{
				// This is the start function, and we can't assume we have a safe stack (the code may be
				// at or near the stack pointer), pivot the stack to make it safe
				SubImm(out, m_stackPointer, m_stackPointer, UNSAFE_STACK_PIVOT);
			}

			// Generate function prologue
			if ((m_stackPointer != DEFAULT_STACK_POINTER) && (func->GetName() == "_start"))
			{
				// If using alternate stack pointer, and this is the _start function, initialize stack pointer
				size_t stackAdjust = m_settings.stackGrowsUp ? -0x10000 : 0;
				AddImm(out, m_stackPointer, 0, stackAdjust);
				EMIT_2R(mov, m_framePointer, m_stackPointer, 0);
			}
			else if (m_framePointerEnabled)
			{
				if (m_settings.stackGrowsUp)
				{
					EMIT_3I(stw, m_framePointer, m_stackPointer, 4);
					EMIT_3I(add, m_framePointer, m_stackPointer, 4);
					AddImm(out, m_stackPointer, m_stackPointer, 4 + m_stackFrameSize);
				}
				else
				{
					EMIT_3I(stwu, m_framePointer, m_stackPointer, -4);
					EMIT_2R(mov, m_framePointer, m_stackPointer, 0);
					SubImm(out, m_stackPointer, m_stackPointer, m_stackFrameSize);
				}
			}

			first = false;
		}

		if (!GenerateCodeBlock(out, *i))
			return false;

		(*i)->SetOutputBlock(out);
	}

	return true;
}

