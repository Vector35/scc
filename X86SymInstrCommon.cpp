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

#ifdef X86_SYMINSTR_PREFIX

#include <stdio.h>
#include "X86SymInstrCommon.h"
#include "asmx86.h"
#include "Struct.h"

using namespace std;
using namespace asmx86;


#define X86_REG_OF_SIZE(r, size) GetOperandOfSize(r, size)
#ifdef OUTPUT32
#define X86_NATIVE_REG(r) X86_REG_OF_SIZE(r, 32)
#define X86_NATIVE_SIZE 32
#else
#define X86_NATIVE_REG(r) X86_REG_OF_SIZE(r, 64)
#define X86_NATIVE_SIZE 64
#endif


#define X86_ADD_MEM_OP \
	AddReadRegisterOperand(base); \
	AddReadRegisterOperand(index); \
	AddImmediateOperand(scale); \
	AddImmediateOperand(offset);
#define X86_MEM_OP(start) X86_MEM_INDEX(X86_NATIVE_REG(m_operands[start].reg), X86_NATIVE_REG(m_operands[(start) + 1].reg), \
	(uint32_t)m_operands[(start) + 2].immed, m_operands[(start) + 3].immed)


#define X86_PRINT_REG_OP(i, size) PrintNativeOperand(GetOperandOfSize(m_operands[i].reg, size))
#define X86_PRINT_MEM_OP(start, size) \
	switch (size) \
	{ \
	case 8: \
		fprintf(stderr, "byte ["); \
		break; \
	case 16: \
		fprintf(stderr, "word ["); \
		break; \
	case 32: \
		fprintf(stderr, "dword ["); \
		break; \
	case 64: \
		fprintf(stderr, "qword ["); \
		break; \
	default: \
		fprintf(stderr, "["); \
		break; \
	} \
	if (m_operands[start].reg != SYMREG_NONE) \
	{ \
		m_operands[start].Print(func); \
		if (m_operands[(start) + 1].reg != SYMREG_NONE) \
		{ \
			fprintf(stderr, " + "); \
			m_operands[(start) + 1].Print(func); \
			if (m_operands[(start) + 2].immed != 1) \
				fprintf(stderr, "*%d", (int)m_operands[(start) + 2].immed); \
		} \
		if (m_operands[(start) + 3].immed != 0) \
			fprintf(stderr, " + %lld", m_operands[(start) + 3].immed); \
	} \
	else if (m_operands[(start) + 1].reg != SYMREG_NONE) \
	{ \
		fprintf(stderr, " + "); \
		m_operands[(start) + 1].Print(func); \
		if (m_operands[(start) + 2].immed != 1) \
			fprintf(stderr, "*%d", (int)m_operands[(start) + 2].immed); \
		if (m_operands[(start) + 3].immed != 0) \
			fprintf(stderr, " + %lld", m_operands[(start) + 3].immed); \
	} \
	else \
	{ \
		fprintf(stderr, "0x%llx", m_operands[(start) + 3].immed); \
	} \
	fprintf(stderr, "]");


#define X86_IMPLEMENT_NO_OPERANDS(name) \
X86_SYMINSTR_CLASS(name)::X86_SYMINSTR_CLASS(name)() \
{ \
} \
bool X86_SYMINSTR_CLASS(name)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT(name); \
	return true; \
} \
void X86_SYMINSTR_CLASS(name)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name); \
} \
SymInstr* X86_SYMINSTR_NAME(name)() { return new X86_SYMINSTR_CLASS(name)(); }


#define X86_IMPLEMENT_1OP_RM_NATIVE(name, access, flags) \
X86_SYMINSTR_CLASS_OP(name, R)::X86_SYMINSTR_CLASS_OP(name, R)(uint32_t a) \
{ \
	Add ## access ## RegisterOperand(a); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_OP(name, R)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_R(name, X86_NATIVE_REG(m_operands[0].reg)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_OP(name, R)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, X86_NATIVE_SIZE); \
} \
X86_SYMINSTR_CLASS_OP(name, M)::X86_SYMINSTR_CLASS_OP(name, M)(X86_MEM_OP_PARAM) \
{ \
	X86_ADD_MEM_OP; \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_OP(name, M)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_M(name, X86_MEM_OP(0)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_OP(name, M)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, X86_NATIVE_SIZE); \
} \
SymInstr* X86_SYMINSTR_NAME_OP(name, R)(uint32_t a) { return new X86_SYMINSTR_CLASS_OP(name, R)(a); } \
SymInstr* X86_SYMINSTR_NAME_OP(name, M)(X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_OP(name, M)(X86_MEM_OP_PASS); }


#define X86_IMPLEMENT_1OP_RM_SIZE(name, size, access, flags) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(uint32_t a) \
{ \
	Add ## access ## RegisterOperand(a); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_R(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM) \
{ \
	X86_ADD_MEM_OP; \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_M(name ## _ ## size, X86_MEM_OP(0)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, R)(uint32_t a) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(a); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PASS); }


#define X86_IMPLEMENT_2OP_RM_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) \
{ \
	Add ## firstAccess ## RegisterOperand(a); \
	Add ## secondAccess ## RegisterOperand(b); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RR(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_REG_OF_SIZE(m_operands[1].reg, size)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, size); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)(uint32_t a, X86_MEM_OP_PARAM) \
{ \
	Add ## firstAccess ## RegisterOperand(a); \
	X86_ADD_MEM_OP; \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RM(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_MEM_OP(1)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_MEM_OP(1, size); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b) \
{ \
	X86_ADD_MEM_OP; \
	Add ## secondAccess ## RegisterOperand(b); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_MR(name ## _ ## size, X86_MEM_OP(0), X86_REG_OF_SIZE(m_operands[4].reg, size)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(4, size); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RM)(uint32_t a, X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)(a, X86_MEM_OP_PASS); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PASS, b); } \


#ifdef OUTPUT32
#define X86_IMPLEMENT_1OP_RM(name, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 8, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 16, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 32, access, flags)
#define X86_IMPLEMENT_2OP_RM(name, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 8, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 16, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 32, firstAccess, secondAccess, flags)
#else
#define X86_IMPLEMENT_1OP_RM(name, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 8, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 16, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 32, access, flags) \
	X86_IMPLEMENT_1OP_RM_SIZE(name, 64, access, flags)
#define X86_IMPLEMENT_2OP_RM(name, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 8, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 16, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 32, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 64, firstAccess, secondAccess, flags)
#endif


X86_IMPLEMENT_NO_OPERANDS(cld)
X86_IMPLEMENT_NO_OPERANDS(std)
X86_IMPLEMENT_NO_OPERANDS(nop)
X86_IMPLEMENT_NO_OPERANDS(int3)

X86_IMPLEMENT_1OP_RM(dec, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_RM(inc, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_RM(neg, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_RM(not, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_RM_NATIVE(pop, Write, SYMFLAG_MEMORY_BARRIER)
X86_IMPLEMENT_1OP_RM_NATIVE(push, Read, SYMFLAG_MEMORY_BARRIER)

X86_IMPLEMENT_2OP_RM(adc, ReadWrite, Read, SYMFLAG_USES_FLAGS | SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(add, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(and, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(cmp, Read, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(sbb, ReadWrite, Read, SYMFLAG_USES_FLAGS | SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(sub, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(test, Read, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(or, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM(xchg, ReadWrite, ReadWrite, 0)
X86_IMPLEMENT_2OP_RM(xor, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)


OperandType X86_SYMINSTR_NAME(Instr)::GetOperandOfSize(uint32_t reg, uint32_t size)
{
	reg &= 0x7fffffff;
	switch (size)
	{
	case 8:
		switch (reg)
		{
		case REG_AL: case REG_AH: case REG_AX: case REG_EAX: case REG_RAX:  return REG_AL;
		case REG_CL: case REG_CH: case REG_CX: case REG_ECX: case REG_RCX:  return REG_CL;
		case REG_DL: case REG_DH: case REG_DX: case REG_EDX: case REG_RDX:  return REG_DL;
		case REG_BL: case REG_BH: case REG_BX: case REG_EBX: case REG_RBX:  return REG_BL;
		case REG_SPL: case REG_SP: case REG_ESP: case REG_RSP:  return REG_SPL;
		case REG_BPL: case REG_BP: case REG_EBP: case REG_RBP:  return REG_BPL;
		case REG_SIL: case REG_SI: case REG_ESI: case REG_RSI:  return REG_SIL;
		case REG_DIL: case REG_DI: case REG_EDI: case REG_RDI:  return REG_DIL;
		case REG_R8B: case REG_R8W: case REG_R8D: case REG_R8:  return REG_R8B;
		case REG_R9B: case REG_R9W: case REG_R9D: case REG_R9:  return REG_R9B;
		case REG_R10B: case REG_R10W: case REG_R10D: case REG_R10:  return REG_R10B;
		case REG_R11B: case REG_R11W: case REG_R11D: case REG_R11:  return REG_R11B;
		case REG_R12B: case REG_R12W: case REG_R12D: case REG_R12:  return REG_R12B;
		case REG_R13B: case REG_R13W: case REG_R13D: case REG_R13:  return REG_R13B;
		case REG_R14B: case REG_R14W: case REG_R14D: case REG_R14:  return REG_R14B;
		case REG_R15B: case REG_R15W: case REG_R15D: case REG_R15:  return REG_R15B;
		default:  return NONE;
		}
	case 16:
		switch (reg)
		{
		case REG_AL: case REG_AH: case REG_AX: case REG_EAX: case REG_RAX:  return REG_AX;
		case REG_CL: case REG_CH: case REG_CX: case REG_ECX: case REG_RCX:  return REG_CX;
		case REG_DL: case REG_DH: case REG_DX: case REG_EDX: case REG_RDX:  return REG_DX;
		case REG_BL: case REG_BH: case REG_BX: case REG_EBX: case REG_RBX:  return REG_BX;
		case REG_SPL: case REG_SP: case REG_ESP: case REG_RSP:  return REG_SP;
		case REG_BPL: case REG_BP: case REG_EBP: case REG_RBP:  return REG_BP;
		case REG_SIL: case REG_SI: case REG_ESI: case REG_RSI:  return REG_SI;
		case REG_DIL: case REG_DI: case REG_EDI: case REG_RDI:  return REG_DI;
		case REG_R8B: case REG_R8W: case REG_R8D: case REG_R8:  return REG_R8W;
		case REG_R9B: case REG_R9W: case REG_R9D: case REG_R9:  return REG_R9W;
		case REG_R10B: case REG_R10W: case REG_R10D: case REG_R10:  return REG_R10W;
		case REG_R11B: case REG_R11W: case REG_R11D: case REG_R11:  return REG_R11W;
		case REG_R12B: case REG_R12W: case REG_R12D: case REG_R12:  return REG_R12W;
		case REG_R13B: case REG_R13W: case REG_R13D: case REG_R13:  return REG_R13W;
		case REG_R14B: case REG_R14W: case REG_R14D: case REG_R14:  return REG_R14W;
		case REG_R15B: case REG_R15W: case REG_R15D: case REG_R15:  return REG_R15W;
		default:  return NONE;
		}
	case 32:
		switch (reg)
		{
		case REG_AL: case REG_AH: case REG_AX: case REG_EAX: case REG_RAX:  return REG_EAX;
		case REG_CL: case REG_CH: case REG_CX: case REG_ECX: case REG_RCX:  return REG_ECX;
		case REG_DL: case REG_DH: case REG_DX: case REG_EDX: case REG_RDX:  return REG_EDX;
		case REG_BL: case REG_BH: case REG_BX: case REG_EBX: case REG_RBX:  return REG_EBX;
		case REG_SPL: case REG_SP: case REG_ESP: case REG_RSP:  return REG_ESP;
		case REG_BPL: case REG_BP: case REG_EBP: case REG_RBP:  return REG_EBP;
		case REG_SIL: case REG_SI: case REG_ESI: case REG_RSI:  return REG_ESI;
		case REG_DIL: case REG_DI: case REG_EDI: case REG_RDI:  return REG_EDI;
		case REG_R8B: case REG_R8W: case REG_R8D: case REG_R8:  return REG_R8D;
		case REG_R9B: case REG_R9W: case REG_R9D: case REG_R9:  return REG_R9D;
		case REG_R10B: case REG_R10W: case REG_R10D: case REG_R10:  return REG_R10D;
		case REG_R11B: case REG_R11W: case REG_R11D: case REG_R11:  return REG_R11D;
		case REG_R12B: case REG_R12W: case REG_R12D: case REG_R12:  return REG_R12D;
		case REG_R13B: case REG_R13W: case REG_R13D: case REG_R13:  return REG_R13D;
		case REG_R14B: case REG_R14W: case REG_R14D: case REG_R14:  return REG_R14D;
		case REG_R15B: case REG_R15W: case REG_R15D: case REG_R15:  return REG_R15D;
		default:  return NONE;
		}
	case 64:
		switch (reg)
		{
		case REG_AL: case REG_AH: case REG_AX: case REG_EAX: case REG_RAX:  return REG_RAX;
		case REG_CL: case REG_CH: case REG_CX: case REG_ECX: case REG_RCX:  return REG_RCX;
		case REG_DL: case REG_DH: case REG_DX: case REG_EDX: case REG_RDX:  return REG_RDX;
		case REG_BL: case REG_BH: case REG_BX: case REG_EBX: case REG_RBX:  return REG_RBX;
		case REG_SPL: case REG_SP: case REG_ESP: case REG_RSP:  return REG_RSP;
		case REG_BPL: case REG_BP: case REG_EBP: case REG_RBP:  return REG_RBP;
		case REG_SIL: case REG_SI: case REG_ESI: case REG_RSI:  return REG_RSI;
		case REG_DIL: case REG_DI: case REG_EDI: case REG_RDI:  return REG_RDI;
		case REG_R8B: case REG_R8W: case REG_R8D: case REG_R8:  return REG_R8;
		case REG_R9B: case REG_R9W: case REG_R9D: case REG_R9:  return REG_R9;
		case REG_R10B: case REG_R10W: case REG_R10D: case REG_R10:  return REG_R10;
		case REG_R11B: case REG_R11W: case REG_R11D: case REG_R11:  return REG_R11;
		case REG_R12B: case REG_R12W: case REG_R12D: case REG_R12:  return REG_R12;
		case REG_R13B: case REG_R13W: case REG_R13D: case REG_R13:  return REG_R13;
		case REG_R14B: case REG_R14W: case REG_R14D: case REG_R14:  return REG_R14;
		case REG_R15B: case REG_R15W: case REG_R15D: case REG_R15:  return REG_R15;
		default:  return NONE;
		}
	default:
		return NONE;
	}
}


void X86_SYMINSTR_NAME(Instr)::PrintNativeOperand(OperandType reg)
{
	switch (reg)
	{
	case REG_AL:  fprintf(stderr, "al"); break;
	case REG_CL:  fprintf(stderr, "cl"); break;
	case REG_DL:  fprintf(stderr, "dl"); break;
	case REG_BL:  fprintf(stderr, "bl"); break;
	case REG_AH:  fprintf(stderr, "ah"); break;
	case REG_CH:  fprintf(stderr, "ch"); break;
	case REG_DH:  fprintf(stderr, "dh"); break;
	case REG_BH:  fprintf(stderr, "bh"); break;
	case REG_SPL:  fprintf(stderr, "spl"); break;
	case REG_BPL:  fprintf(stderr, "bpl"); break;
	case REG_SIL:  fprintf(stderr, "sil"); break;
	case REG_DIL:  fprintf(stderr, "dil"); break;
	case REG_R8B:  fprintf(stderr, "r8b"); break;
	case REG_R9B:  fprintf(stderr, "r9b"); break;
	case REG_R10B:  fprintf(stderr, "r10b"); break;
	case REG_R11B:  fprintf(stderr, "r11b"); break;
	case REG_R12B:  fprintf(stderr, "r12b"); break;
	case REG_R13B:  fprintf(stderr, "r13b"); break;
	case REG_R14B:  fprintf(stderr, "r14b"); break;
	case REG_R15B:  fprintf(stderr, "r15b"); break;
	case REG_AX:  fprintf(stderr, "ax"); break;
	case REG_CX:  fprintf(stderr, "cx"); break;
	case REG_DX:  fprintf(stderr, "dx"); break;
	case REG_BX:  fprintf(stderr, "bx"); break;
	case REG_SP:  fprintf(stderr, "sp"); break;
	case REG_BP:  fprintf(stderr, "bp"); break;
	case REG_SI:  fprintf(stderr, "si"); break;
	case REG_DI:  fprintf(stderr, "di"); break;
	case REG_R8W:  fprintf(stderr, "r8w"); break;
	case REG_R9W:  fprintf(stderr, "r9w"); break;
	case REG_R10W:  fprintf(stderr, "r10w"); break;
	case REG_R11W:  fprintf(stderr, "r11w"); break;
	case REG_R12W:  fprintf(stderr, "r12w"); break;
	case REG_R13W:  fprintf(stderr, "r13w"); break;
	case REG_R14W:  fprintf(stderr, "r14w"); break;
	case REG_R15W:  fprintf(stderr, "r15w"); break;
	case REG_EAX:  fprintf(stderr, "eax"); break;
	case REG_ECX:  fprintf(stderr, "ecx"); break;
	case REG_EDX:  fprintf(stderr, "edx"); break;
	case REG_EBX:  fprintf(stderr, "ebx"); break;
	case REG_ESP:  fprintf(stderr, "esp"); break;
	case REG_EBP:  fprintf(stderr, "ebp"); break;
	case REG_ESI:  fprintf(stderr, "esi"); break;
	case REG_EDI:  fprintf(stderr, "edi"); break;
	case REG_R8D:  fprintf(stderr, "r8d"); break;
	case REG_R9D:  fprintf(stderr, "r9d"); break;
	case REG_R10D:  fprintf(stderr, "r10d"); break;
	case REG_R11D:  fprintf(stderr, "r11d"); break;
	case REG_R12D:  fprintf(stderr, "r12d"); break;
	case REG_R13D:  fprintf(stderr, "r13d"); break;
	case REG_R14D:  fprintf(stderr, "r14d"); break;
	case REG_R15D:  fprintf(stderr, "r15d"); break;
	case REG_RAX:  fprintf(stderr, "rax"); break;
	case REG_RCX:  fprintf(stderr, "rcx"); break;
	case REG_RDX:  fprintf(stderr, "rdx"); break;
	case REG_RBX:  fprintf(stderr, "rbx"); break;
	case REG_RSP:  fprintf(stderr, "rsp"); break;
	case REG_RBP:  fprintf(stderr, "rbp"); break;
	case REG_RSI:  fprintf(stderr, "rsi"); break;
	case REG_RDI:  fprintf(stderr, "rdi"); break;
	case REG_R8:  fprintf(stderr, "r8"); break;
	case REG_R9:  fprintf(stderr, "r9"); break;
	case REG_R10:  fprintf(stderr, "r10"); break;
	case REG_R11:  fprintf(stderr, "r11"); break;
	case REG_R12:  fprintf(stderr, "r12"); break;
	case REG_R13:  fprintf(stderr, "r13"); break;
	case REG_R14:  fprintf(stderr, "r14"); break;
	case REG_R15:  fprintf(stderr, "r15"); break;
	default:  fprintf(stderr, "<%d>", (int)reg); break;
	}
}


#endif

