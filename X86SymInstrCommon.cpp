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
#define X86_NATIVE_SIZE_INSTR_NAME(i) i ## _32
#else
#define X86_NATIVE_REG(r) X86_REG_OF_SIZE(r, 64)
#define X86_NATIVE_SIZE 64
#define X86_NATIVE_SIZE_INSTR_NAME(i) i ## _64
#endif


#define X86_ADD_MEM_OP \
	AddReadRegisterOperand(base); \
	AddReadRegisterOperand(index); \
	AddImmediateOperand(scale); \
	AddImmediateOperand(offset);
#define X86_MEM_OP(start) X86_MEM_INDEX(X86_NATIVE_REG(m_operands[start].reg), X86_NATIVE_REG(m_operands[(start) + 1].reg), \
	(uint32_t)m_operands[(start) + 2].immed, m_operands[(start) + 3].immed)


#define X86_PRINT_REG_OP(i, size) \
	if (!SYMREG_IS_SPECIAL_REG(m_operands[i].reg)) \
		m_operands[i].Print(func); \
	else if (m_operands[i].reg < SYMREG_MIN_SPECIAL_REG) \
	 	X86_SYMINSTR_NAME(PrintNativeOperand)(GetOperandOfSize(m_operands[i].reg, size)); \
	else \
		m_operands[i].Print(func);
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
			fprintf(stderr, " + %lld", (long long)m_operands[(start) + 3].immed); \
	} \
	else if (m_operands[(start) + 1].reg != SYMREG_NONE) \
	{ \
		fprintf(stderr, " + "); \
		m_operands[(start) + 1].Print(func); \
		if (m_operands[(start) + 2].immed != 1) \
			fprintf(stderr, "*%d", (int)m_operands[(start) + 2].immed); \
		if (m_operands[(start) + 3].immed != 0) \
			fprintf(stderr, " + %lld", (long long)m_operands[(start) + 3].immed); \
	} \
	else \
	{ \
		fprintf(stderr, "0x%llx", (long long)m_operands[(start) + 3].immed); \
	} \
	fprintf(stderr, "]");


#define X86_IMPLEMENT_NO_OPERANDS(name, flags) \
X86_SYMINSTR_CLASS(name)::X86_SYMINSTR_CLASS(name)() \
{ \
	EnableFlag(flags); \
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


#define X86_IMPLEMENT_1OP_MODRM_NATIVE(name, access, flags) \
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


#define X86_IMPLEMENT_1OP_MODRM_NATIVE_IMM(name, access, flags) \
X86_IMPLEMENT_1OP_MODRM_NATIVE(name, access, flags) \
X86_SYMINSTR_CLASS_OP(name, I)::X86_SYMINSTR_CLASS_OP(name, I)(int64_t a) \
{ \
	AddImmediateOperand(a); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_OP(name, I)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_I(name, m_operands[0].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_OP(name, I)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " %lld", (long long)m_operands[0].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_OP(name, I)(int64_t a) { return new X86_SYMINSTR_CLASS_OP(name, I)(a); }


#define X86_IMPLEMENT_1OP_R_SIZE(name, size, access, flags) \
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
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, R)(uint32_t a) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(a); }


#define X86_IMPLEMENT_1OP_M_SIZE(name, size, access, flags) \
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
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PASS); }


#define X86_IMPLEMENT_1OP_MODRM_SIZE(name, size, access, flags) \
X86_IMPLEMENT_1OP_R_SIZE(name, size, access, flags) \
X86_IMPLEMENT_1OP_M_SIZE(name, size, access, flags)


#define X86_IMPLEMENT_SETCC(name) \
X86_SYMINSTR_CLASS_OP(name, R)::X86_SYMINSTR_CLASS_OP(name, R)(uint32_t a) \
{ \
	AddWriteRegisterOperand(a); \
	EnableFlag(SYMFLAG_USES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_OP(name, R)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_R(name, X86_REG_OF_SIZE(m_operands[0].reg, 8)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_OP(name, R)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, 8); \
} \
SymInstr* X86_SYMINSTR_NAME_OP(name, R)(uint32_t a) { return new X86_SYMINSTR_CLASS_OP(name, R)(a); }


#define X86_IMPLEMENT_2OP_II_NATIVE(name, flags) \
X86_SYMINSTR_CLASS_OP(name, II)::X86_SYMINSTR_CLASS_OP(name, II)(int64_t a, int64_t b) \
{ \
	AddImmediateOperand(a); \
	AddImmediateOperand(b); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_OP(name, II)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_II(name, (uint16_t)m_operands[0].immed, (uint8_t)m_operands[1].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_OP(name, II)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " %lld, %lld", (long long)m_operands[0].immed, (long long)m_operands[1].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_OP(name, II)(int64_t a, int64_t b) { return new X86_SYMINSTR_CLASS_OP(name, II)(a, b); }


#define X86_IMPLEMENT_2OP_RR_SIZE(name, size, firstAccess, secondAccess, flags) \
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
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(a, b); }


#define X86_IMPLEMENT_2OP_RM_SIZE(name, size, firstAccess, secondAccess, flags) \
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
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RM)(uint32_t a, X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)(a, X86_MEM_OP_PASS); }


#define X86_IMPLEMENT_2OP_MR_SIZE(name, size, firstAccess, secondAccess, flags) \
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
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PASS, b); }


#define X86_IMPLEMENT_2OP_MODRM_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_IMPLEMENT_2OP_RR_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_IMPLEMENT_2OP_RM_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_IMPLEMENT_2OP_MR_SIZE(name, size, firstAccess, secondAccess, flags)


#define X86_IMPLEMENT_2OP_IMM_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(uint32_t a, int64_t b) \
{ \
	Add ## firstAccess ## RegisterOperand(a); \
	AddImmediateOperand(b); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RI(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), (int ## size ## _t)m_operands[1].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[1].immed); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b) \
{ \
	X86_ADD_MEM_OP; \
	AddImmediateOperand(b); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_MI(name ## _ ## size, X86_MEM_OP(0), (int ## size ## _t)m_operands[4].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[4].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RI)(uint32_t a, int64_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PASS, b); }


#define X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_IMPLEMENT_2OP_MODRM_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_IMPLEMENT_2OP_IMM_SIZE(name, size, firstAccess, secondAccess, flags)


#define X86_IMPLEMENT_MOV_SIZE(name, size) \
X86_IMPLEMENT_2OP_RM_SIZE(name, size, Write, Read, SYMFLAG_COPY) \
X86_IMPLEMENT_2OP_MR_SIZE(name, size, Write, Read, SYMFLAG_COPY) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) \
{ \
	AddWriteRegisterOperand(a); \
	AddReadRegisterOperand(b); \
	EnableFlag(SYMFLAG_COPY); \
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
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::UpdateInstruction(SymInstrFunction* func, const Settings& settings, \
	vector<SymInstr*>& replacement) \
{ \
	if (m_operands[0].reg == m_operands[1].reg) \
		return true; \
	return false; \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(uint32_t a, int64_t b) \
{ \
	AddWriteRegisterOperand(a); \
	AddImmediateOperand(b); \
	if (b == 0) \
		EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	bool normalStack = true; \
	if ((func->GetSettings().stackPointer != SYMREG_NATIVE_REG(REG_ESP)) || func->GetSettings().stackGrowsUp) \
		normalStack = false; \
	if (m_operands[1].immed == 0) \
	{ \
		if (size >= 32) \
			EMIT_RR(xor_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_REG_OF_SIZE(m_operands[0].reg, 32)); \
		else \
			EMIT_RR(xor_ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_REG_OF_SIZE(m_operands[0].reg, size)); \
	} \
	else if (normalStack && (m_operands[1].immed >= -0x80) && (m_operands[1].immed <= 0x7f)) \
	{ \
		EMIT_I(push, (int ## size ## _t)m_operands[1].immed); \
		EMIT_R(pop, X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE)); \
	} \
	else if ((size == 64) && (m_operands[1].immed >= 0) && (m_operands[1].immed <= 0xffffffff)) \
	{ \
		EMIT_RI(name ## _32, X86_REG_OF_SIZE(m_operands[0].reg, 32), (int32_t)m_operands[1].immed); \
	} \
	else \
	{ \
		EMIT_RI(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), (int ## size ## _t)m_operands[1].immed); \
	} \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[1].immed); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b) \
{ \
	X86_ADD_MEM_OP; \
	AddImmediateOperand(b); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	bool normalStack = true; \
	if ((func->GetSettings().stackPointer != SYMREG_NATIVE_REG(REG_ESP)) || func->GetSettings().stackGrowsUp) \
		normalStack = false; \
	if (normalStack && (size == X86_NATIVE_SIZE) && (m_operands[4].immed >= -0x80) && (m_operands[4].immed <= 0x7f)) \
	{ \
		EMIT_I(push, (int ## size ## _t)m_operands[4].immed); \
		EMIT_M(pop, X86_MEM_OP(0)); \
	} \
	else \
	{ \
		EMIT_MI(name ## _ ## size, X86_MEM_OP(0), (int ## size ## _t)m_operands[4].immed); \
	} \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[4].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RI)(uint32_t a, int64_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PASS, b); }


#define X86_IMPLEMENT_3OP_RRI_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c) \
{ \
	Add ## firstAccess ## RegisterOperand(a); \
	Add ## secondAccess ## RegisterOperand(b); \
	AddImmediateOperand(c); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RRI(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_REG_OF_SIZE(m_operands[1].reg, size), \
		(int ## size ## _t)m_operands[2].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[2].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)(a, b, c); }


#define X86_IMPLEMENT_3OP_RMI_SIZE(name, size, firstAccess, secondAccess, flags) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI)(uint32_t a, X86_MEM_OP_PARAM, int64_t c) \
{ \
	Add ## firstAccess ## RegisterOperand(a); \
	X86_ADD_MEM_OP; \
	AddImmediateOperand(c); \
	EnableFlag(flags); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RMI(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_MEM_OP(1), (int ## size ## _t)m_operands[5].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_MEM_OP(1, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[5].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RMI)(uint32_t a, X86_MEM_OP_PARAM, int64_t c) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI)(a, X86_MEM_OP_PASS, c); }


#define X86_IMPLEMENT_SHIFT_SIZE(name, size) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) \
{ \
	AddReadWriteRegisterOperand(a); \
	AddReadRegisterOperand(b); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RR(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_REG_OF_SIZE(m_operands[1].reg, 8)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, 8); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(uint32_t a, int64_t b) \
{ \
	AddReadWriteRegisterOperand(a); \
	AddImmediateOperand(b); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RI(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), (uint8_t)m_operands[1].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[1].immed); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b) \
{ \
	X86_ADD_MEM_OP; \
	AddReadRegisterOperand(b); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_MR(name ## _ ## size, X86_MEM_OP(0), X86_REG_OF_SIZE(m_operands[4].reg, 8)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(4, 8); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b) \
{ \
	X86_ADD_MEM_OP; \
	AddImmediateOperand(b); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_MI(name ## _ ## size, X86_MEM_OP(0), (uint8_t)m_operands[4].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[4].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RI)(uint32_t a, int64_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PASS, b); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PASS, b); }


#define X86_IMPLEMENT_MOVEXT_SIZE(name, destSize, srcSize) \
X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR)::X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR)(uint32_t a, uint32_t b) \
{ \
	AddWriteRegisterOperand(a); \
	AddReadRegisterOperand(b); \
} \
bool X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RR(name ## _ ## destSize ## _ ## srcSize, X86_REG_OF_SIZE(m_operands[0].reg, destSize), \
		X86_REG_OF_SIZE(m_operands[1].reg, srcSize)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, destSize); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, srcSize); \
} \
X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM)::X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM)(uint32_t a, X86_MEM_OP_PARAM) \
{ \
	AddWriteRegisterOperand(a); \
	X86_ADD_MEM_OP; \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
} \
bool X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RM(name ## _ ## destSize ## _ ## srcSize, X86_REG_OF_SIZE(m_operands[0].reg, destSize), X86_MEM_OP(1)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, destSize); \
	fprintf(stderr, ", "); \
	X86_PRINT_MEM_OP(1, srcSize); \
} \
SymInstr* X86_SYMINSTR_NAME_2SIZE_OP(name, destSize, srcSize, RR)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR)(a, b); } \
SymInstr* X86_SYMINSTR_NAME_2SIZE_OP(name, destSize, srcSize, RM)(uint32_t a, X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM)(a, X86_MEM_OP_PASS); }


#define X86_IMPLEMENT_MULDIV8(name) \
X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R)::X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R)(uint32_t a, uint32_t eax) \
{ \
	AddReadRegisterOperand(a); \
	AddReadWriteRegisterOperand(eax); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_R(name ## _8, X86_REG_OF_SIZE(m_operands[0].reg, 8)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, 8); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M)::X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M)(X86_MEM_OP_PARAM, uint32_t eax) \
{ \
	X86_ADD_MEM_OP; \
	AddReadWriteRegisterOperand(eax); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_M(name ## _8, X86_MEM_OP(0)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, 8); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(4, 8); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, 8, R)(uint32_t a, uint32_t eax) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R)(a, eax); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, 8, M)(X86_MEM_OP_PARAM, uint32_t eax) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M)(X86_MEM_OP_PASS, eax); }


#define X86_IMPLEMENT_MULDIV_SIZE(name, size) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(uint32_t a, uint32_t eax, uint32_t edx) \
{ \
	AddReadRegisterOperand(a); \
	AddReadWriteRegisterOperand(eax); \
	AddReadWriteRegisterOperand(edx); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
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
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(2, size); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM, uint32_t eax, uint32_t edx) \
{ \
	X86_ADD_MEM_OP; \
	AddReadWriteRegisterOperand(eax); \
	AddReadWriteRegisterOperand(edx); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
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
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(4, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(5, size); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, R)(uint32_t a, uint32_t eax, uint32_t edx) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(a, eax, edx); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM, uint32_t eax, uint32_t edx) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PASS, eax, edx); }


#define X86_IMPLEMENT_SHIFT_DOUBLE_SIZE(name, size) \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR)(uint32_t a, uint32_t b, uint32_t c) \
{ \
	AddReadWriteRegisterOperand(a); \
	AddReadRegisterOperand(b); \
	AddReadRegisterOperand(c); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RRR(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_REG_OF_SIZE(m_operands[1].reg, size), \
		X86_REG_OF_SIZE(m_operands[2].reg, 8)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(2, 8); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR)(X86_MEM_OP_PARAM, uint32_t b, uint32_t c) \
{ \
	X86_ADD_MEM_OP; \
	AddReadRegisterOperand(b); \
	AddReadRegisterOperand(c); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_MRR(name ## _ ## size, X86_MEM_OP(0), X86_REG_OF_SIZE(m_operands[4].reg, size), \
		X86_REG_OF_SIZE(m_operands[5].reg, 8)); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(4, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(5, 8); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c) \
{ \
	AddReadWriteRegisterOperand(a); \
	AddReadRegisterOperand(b); \
	AddImmediateOperand(c); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_RRI(name ## _ ## size, X86_REG_OF_SIZE(m_operands[0].reg, size), X86_REG_OF_SIZE(m_operands[1].reg, size), \
		(uint8_t)m_operands[2].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_REG_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(1, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[2].immed); \
} \
X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI)::X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI)(X86_MEM_OP_PARAM, uint32_t b, int64_t c) \
{ \
	X86_ADD_MEM_OP; \
	AddReadRegisterOperand(b); \
	AddImmediateOperand(c); \
	EnableFlag(SYMFLAG_MEMORY_BARRIER); \
	EnableFlag(SYMFLAG_WRITES_FLAGS); \
} \
bool X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI)::EmitInstruction(SymInstrFunction* func, OutputBlock* out) \
{ \
	EMIT_MRI(name ## _ ## size, X86_MEM_OP(0), X86_REG_OF_SIZE(m_operands[4].reg, size), \
		(uint8_t)m_operands[5].immed); \
	return true; \
} \
void X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI)::Print(SymInstrFunction* func) \
{ \
	fprintf(stderr, #name " "); \
	X86_PRINT_MEM_OP(0, size); \
	fprintf(stderr, ", "); \
	X86_PRINT_REG_OP(4, size); \
	fprintf(stderr, ", %lld", (long long)m_operands[5].immed); \
} \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RRR)(uint32_t a, uint32_t b, uint32_t c) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR)(a, b, c); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MRR)(X86_MEM_OP_PARAM, uint32_t b, uint32_t c) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR)(X86_MEM_OP_PASS, b, c); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)(a, b, c); } \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MRI)(X86_MEM_OP_PARAM, uint32_t b, int64_t c) { return new X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI)(X86_MEM_OP_PASS, b, c); }


#ifdef OUTPUT32
#define X86_IMPLEMENT_1OP_MODRM(name, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 8, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 16, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 32, access, flags)
#define X86_IMPLEMENT_2OP_MODRM(name, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 8, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 16, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 32, firstAccess, secondAccess, flags)
#define X86_IMPLEMENT_2OP_MODRM_IMM(name, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 8, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 16, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 32, firstAccess, secondAccess, flags)
#define X86_IMPLEMENT_MOV(name) \
	X86_IMPLEMENT_MOV_SIZE(name, 8) \
	X86_IMPLEMENT_MOV_SIZE(name, 16) \
	X86_IMPLEMENT_MOV_SIZE(name, 32)
#define X86_IMPLEMENT_CMOV(name) \
	X86_IMPLEMENT_2OP_RR_SIZE(name, 16, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 16, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RR_SIZE(name, 32, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 32, ReadWrite, Read, SYMFLAG_USES_FLAGS)
#define X86_IMPLEMENT_SHIFT(name) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 8) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 16) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 32)
#define X86_IMPLEMENT_MOVEXT(name) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 16, 8) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 32, 8) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 32, 16)
#define X86_IMPLEMENT_MULDIV(name) \
	X86_IMPLEMENT_MULDIV8(name) \
	X86_IMPLEMENT_MULDIV_SIZE(name, 16) \
	X86_IMPLEMENT_MULDIV_SIZE(name, 32)
#define X86_IMPLEMENT_SHIFT_DOUBLE(name) \
	X86_IMPLEMENT_SHIFT_DOUBLE_SIZE(name, 16) \
	X86_IMPLEMENT_SHIFT_DOUBLE_SIZE(name, 32)
#else
#define X86_IMPLEMENT_1OP_MODRM(name, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 8, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 16, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 32, access, flags) \
	X86_IMPLEMENT_1OP_MODRM_SIZE(name, 64, access, flags)
#define X86_IMPLEMENT_2OP_MODRM(name, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 8, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 16, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 32, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_SIZE(name, 64, firstAccess, secondAccess, flags)
#define X86_IMPLEMENT_2OP_MODRM_IMM(name, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 8, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 16, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 32, firstAccess, secondAccess, flags) \
	X86_IMPLEMENT_2OP_MODRM_IMM_SIZE(name, 64, firstAccess, secondAccess, flags)
#define X86_IMPLEMENT_MOV(name) \
	X86_IMPLEMENT_MOV_SIZE(name, 8) \
	X86_IMPLEMENT_MOV_SIZE(name, 16) \
	X86_IMPLEMENT_MOV_SIZE(name, 32) \
	X86_IMPLEMENT_MOV_SIZE(name, 64)
#define X86_IMPLEMENT_CMOV(name) \
	X86_IMPLEMENT_2OP_RR_SIZE(name, 16, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 16, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RR_SIZE(name, 32, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 32, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RR_SIZE(name, 64, ReadWrite, Read, SYMFLAG_USES_FLAGS) \
	X86_IMPLEMENT_2OP_RM_SIZE(name, 64, ReadWrite, Read, SYMFLAG_USES_FLAGS)
#define X86_IMPLEMENT_SHIFT(name) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 8) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 16) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 32) \
	X86_IMPLEMENT_SHIFT_SIZE(name, 64)
#define X86_IMPLEMENT_MOVEXT(name) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 16, 8) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 32, 8) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 32, 16) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 64, 8) \
	X86_IMPLEMENT_MOVEXT_SIZE(name, 64, 16)
#define X86_IMPLEMENT_MULDIV(name) \
	X86_IMPLEMENT_MULDIV8(name) \
	X86_IMPLEMENT_MULDIV_SIZE(name, 16) \
	X86_IMPLEMENT_MULDIV_SIZE(name, 32) \
	X86_IMPLEMENT_MULDIV_SIZE(name, 64)
#define X86_IMPLEMENT_SHIFT_DOUBLE(name) \
	X86_IMPLEMENT_SHIFT_DOUBLE_SIZE(name, 16) \
	X86_IMPLEMENT_SHIFT_DOUBLE_SIZE(name, 32) \
	X86_IMPLEMENT_SHIFT_DOUBLE_SIZE(name, 64)
#endif


static void X86_SYMINSTR_NAME(PrintNativeOperand)(OperandType reg)
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
	case REG_RIP:  fprintf(stderr, "rip"); break;
	default:  fprintf(stderr, "<%d>", (int)reg); break;
	}
}


X86_IMPLEMENT_NO_OPERANDS(cld, 0)
X86_IMPLEMENT_NO_OPERANDS(std, 0)
X86_IMPLEMENT_NO_OPERANDS(nop, 0)
X86_IMPLEMENT_NO_OPERANDS(int3, SYMFLAG_CONTROL_FLOW)
X86_IMPLEMENT_NO_OPERANDS(retn, SYMFLAG_CONTROL_FLOW)
X86_IMPLEMENT_NO_OPERANDS(leave, SYMFLAG_MEMORY_BARRIER | SYMFLAG_STACK)

X86_IMPLEMENT_1OP_MODRM(dec, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_MODRM(inc, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_MODRM(neg, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_MODRM(not, ReadWrite, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_1OP_MODRM_NATIVE(pop, Write, SYMFLAG_MEMORY_BARRIER | SYMFLAG_STACK)
X86_IMPLEMENT_1OP_MODRM_NATIVE_IMM(push, Read, SYMFLAG_MEMORY_BARRIER | SYMFLAG_STACK)
X86_IMPLEMENT_1OP_MODRM_NATIVE(jmpn, Read, SYMFLAG_CONTROL_FLOW)

X86_IMPLEMENT_1OP_R_SIZE(bswap, 32, ReadWrite, 0)
#ifdef OUTPUT64
X86_IMPLEMENT_1OP_R_SIZE(bswap, 64, ReadWrite, 0)
#endif

X86_IMPLEMENT_SETCC(seto)
X86_IMPLEMENT_SETCC(setno)
X86_IMPLEMENT_SETCC(setb)
X86_IMPLEMENT_SETCC(setae)
X86_IMPLEMENT_SETCC(setz)
X86_IMPLEMENT_SETCC(setnz)
X86_IMPLEMENT_SETCC(setbe)
X86_IMPLEMENT_SETCC(seta)
X86_IMPLEMENT_SETCC(sets)
X86_IMPLEMENT_SETCC(setns)
X86_IMPLEMENT_SETCC(setp)
X86_IMPLEMENT_SETCC(setnp)
X86_IMPLEMENT_SETCC(setl)
X86_IMPLEMENT_SETCC(setge)
X86_IMPLEMENT_SETCC(setle)
X86_IMPLEMENT_SETCC(setg)

X86_IMPLEMENT_CMOV(cmovo)
X86_IMPLEMENT_CMOV(cmovno)
X86_IMPLEMENT_CMOV(cmovb)
X86_IMPLEMENT_CMOV(cmovae)
X86_IMPLEMENT_CMOV(cmovz)
X86_IMPLEMENT_CMOV(cmovnz)
X86_IMPLEMENT_CMOV(cmovbe)
X86_IMPLEMENT_CMOV(cmova)
X86_IMPLEMENT_CMOV(cmovs)
X86_IMPLEMENT_CMOV(cmovns)
X86_IMPLEMENT_CMOV(cmovp)
X86_IMPLEMENT_CMOV(cmovnp)
X86_IMPLEMENT_CMOV(cmovl)
X86_IMPLEMENT_CMOV(cmovge)
X86_IMPLEMENT_CMOV(cmovle)
X86_IMPLEMENT_CMOV(cmovg)

X86_IMPLEMENT_2OP_II_NATIVE(enter, SYMFLAG_MEMORY_BARRIER | SYMFLAG_STACK)

X86_IMPLEMENT_2OP_MODRM_IMM(adc, ReadWrite, Read, SYMFLAG_USES_FLAGS | SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM_IMM(add, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM_IMM(and, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM_IMM(cmp, Read, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_MOV(mov)
X86_IMPLEMENT_2OP_MODRM_IMM(sbb, ReadWrite, Read, SYMFLAG_USES_FLAGS | SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM_IMM(sub, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM_IMM(test, Read, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM_IMM(or, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_MODRM(xchg, ReadWrite, ReadWrite, 0)
X86_IMPLEMENT_2OP_MODRM_IMM(xor, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)

X86_IMPLEMENT_MULDIV(div)
X86_IMPLEMENT_MULDIV(mul)
X86_IMPLEMENT_MULDIV(idiv)
X86_IMPLEMENT_MULDIV(imul)
X86_IMPLEMENT_2OP_RR_SIZE(imul, 16, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM_SIZE(imul, 16, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_3OP_RRI_SIZE(imul, 16, Write, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_3OP_RMI_SIZE(imul, 16, Write, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RR_SIZE(imul, 32, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM_SIZE(imul, 32, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_3OP_RRI_SIZE(imul, 32, Write, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_3OP_RMI_SIZE(imul, 32, Write, Read, SYMFLAG_WRITES_FLAGS)
#ifdef OUTPUT64
X86_IMPLEMENT_2OP_RR_SIZE(imul, 64, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_2OP_RM_SIZE(imul, 64, ReadWrite, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_3OP_RRI_SIZE(imul, 64, Write, Read, SYMFLAG_WRITES_FLAGS)
X86_IMPLEMENT_3OP_RMI_SIZE(imul, 64, Write, Read, SYMFLAG_WRITES_FLAGS)
#endif

X86_IMPLEMENT_SHIFT(shl)
X86_IMPLEMENT_SHIFT(shr)
X86_IMPLEMENT_SHIFT(sar)
X86_IMPLEMENT_SHIFT(rol)
X86_IMPLEMENT_SHIFT(ror)

X86_IMPLEMENT_SHIFT_DOUBLE(shld)
X86_IMPLEMENT_SHIFT_DOUBLE(shrd)

X86_IMPLEMENT_MOVEXT(movsx)
X86_IMPLEMENT_MOVEXT(movzx)
#ifdef OUTPUT64
X86_IMPLEMENT_MOVEXT_SIZE(movsxd, 64, 32)
#endif


X86_SYMINSTR_CLASS_OP(lea, RM)::X86_SYMINSTR_CLASS_OP(lea, RM)(uint32_t a, X86_MEM_OP_PARAM)
{
	AddWriteRegisterOperand(a);
	X86_ADD_MEM_OP;
}


bool X86_SYMINSTR_CLASS_OP(lea, RM)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT_RM(X86_NATIVE_SIZE_INSTR_NAME(lea), X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE), X86_MEM_OP(1));
	return true;
}


void X86_SYMINSTR_CLASS_OP(lea, RM)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "lea ");
	X86_PRINT_REG_OP(0, X86_NATIVE_SIZE);
	fprintf(stderr, ", ");
	X86_PRINT_MEM_OP(1, 0);
}


bool X86_SYMINSTR_CLASS_OP(lea, RM)::UpdateInstruction(SymInstrFunction* func, const Settings& settings,
	vector<SymInstr*>& replacement)
{
	if ((m_operands[0].reg == m_operands[1].reg) && (m_operands[2].reg == SYMREG_NONE) && (m_operands[4].immed == 0))
		return true;
	return false;
}


X86_SYMINSTR_CLASS_SIZE_OP(rdtsc, 32, RR)::X86_SYMINSTR_CLASS_SIZE_OP(rdtsc, 32, RR)(uint32_t a, uint32_t b)
{
	AddWriteRegisterOperand(a);
	AddWriteRegisterOperand(b);
}


bool X86_SYMINSTR_CLASS_SIZE_OP(rdtsc, 32, RR)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(rdtsc);
	return true;
}


void X86_SYMINSTR_CLASS_SIZE_OP(rdtsc, 32, RR)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "rdtsc ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 32);
}


X86_SYMINSTR_CLASS(cwd)::X86_SYMINSTR_CLASS(cwd)(uint32_t eax, uint32_t edx)
{
	AddReadRegisterOperand(eax);
	AddWriteRegisterOperand(edx);
}


bool X86_SYMINSTR_CLASS(cwd)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(cwd);
	return true;
}


void X86_SYMINSTR_CLASS(cwd)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "cwd ");
	X86_PRINT_REG_OP(0, 16);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 16);
}


X86_SYMINSTR_CLASS(cdq)::X86_SYMINSTR_CLASS(cdq)(uint32_t eax, uint32_t edx)
{
	AddReadRegisterOperand(eax);
	AddWriteRegisterOperand(edx);
}


bool X86_SYMINSTR_CLASS(cdq)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(cdq);
	return true;
}


void X86_SYMINSTR_CLASS(cdq)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "cdq ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 32);
}


#ifdef OUTPUT64
X86_SYMINSTR_CLASS(cqo)::X86_SYMINSTR_CLASS(cqo)(uint32_t eax, uint32_t edx)
{
	AddReadRegisterOperand(eax);
	AddWriteRegisterOperand(edx);
}


bool X86_SYMINSTR_CLASS(cqo)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(cqo);
	return true;
}


void X86_SYMINSTR_CLASS(cqo)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "cqo ");
	X86_PRINT_REG_OP(0, 64);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 64);
}
#endif


X86_SYMINSTR_CLASS(rep_movsb)::X86_SYMINSTR_CLASS(rep_movsb)(uint32_t edi, uint32_t esi, uint32_t ecx)
{
	AddReadWriteRegisterOperand(edi);
	AddReadWriteRegisterOperand(esi);
	AddReadWriteRegisterOperand(ecx);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
}


bool X86_SYMINSTR_CLASS(rep_movsb)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(rep);
	EMIT(movsb);
	return true;
}


void X86_SYMINSTR_CLASS(rep_movsb)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "rep movsb ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(2, 32);
}


X86_SYMINSTR_CLASS(rep_stosb)::X86_SYMINSTR_CLASS(rep_stosb)(uint32_t edi, uint32_t eax, uint32_t ecx)
{
	AddReadWriteRegisterOperand(edi);
	AddReadRegisterOperand(eax);
	AddReadWriteRegisterOperand(ecx);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
}


bool X86_SYMINSTR_CLASS(rep_stosb)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(rep);
	EMIT(stosb);
	return true;
}


void X86_SYMINSTR_CLASS(rep_stosb)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "rep stosb ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(2, 32);
}


X86_SYMINSTR_CLASS(repne_scasb)::X86_SYMINSTR_CLASS(repne_scasb)(uint32_t edi, uint32_t eax, uint32_t ecx)
{
	AddReadWriteRegisterOperand(edi);
	AddReadRegisterOperand(eax);
	AddReadWriteRegisterOperand(ecx);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
}


bool X86_SYMINSTR_CLASS(repne_scasb)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(repne);
	EMIT(scasb);
	return true;
}


void X86_SYMINSTR_CLASS(repne_scasb)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "repne scasb ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(1, 32);
	fprintf(stderr, ", ");
	X86_PRINT_REG_OP(2, 32);
}


X86_SYMINSTR_CLASS(CallPopDataAddr)::X86_SYMINSTR_CLASS(CallPopDataAddr)(uint32_t dest, int64_t offset)
{
	AddWriteRegisterOperand(dest);
	AddGlobalVarOperand(offset);
}


bool X86_SYMINSTR_CLASS(CallPopDataAddr)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
	buffer[0] = 0xe8;
	*(uint32_t*)(&buffer[1]) = 0;
	out->FinishWrite(5);
	size_t capturedOffset = out->len;
	EMIT_R(pop, X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE));
	size_t leaOffset = out->len;
	EMIT_RM(lea_32, X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE),
		X86_MEM(X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE), 1));

	*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

	Relocation reloc;
	reloc.type = DATA_RELOC_RELATIVE_8;
	reloc.overflow = LeaOverflowHandler;
	reloc.instruction = leaOffset;
	reloc.offset = out->len - 1;
	reloc.dataOffset = m_operands[1].immed;
	out->relocs.push_back(reloc);

	return true;
}


void X86_SYMINSTR_CLASS(CallPopDataAddr)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "call/pop ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(CallPopCodeAddr)::X86_SYMINSTR_CLASS(CallPopCodeAddr)(uint32_t dest, Function* func, ILBlock* block)
{
	AddWriteRegisterOperand(dest);
	AddBlockOperand(func, block);
}


bool X86_SYMINSTR_CLASS(CallPopCodeAddr)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
	buffer[0] = 0xe8;
	*(uint32_t*)(&buffer[1]) = 0;
	out->FinishWrite(5);
	size_t capturedOffset = out->len;
	EMIT_R(pop, X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE));
	size_t leaOffset = out->len;
	EMIT_RM(lea_32, X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE),
		X86_MEM(X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE), 1));

	*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_8;
	reloc.overflow = LeaOverflowHandler;
	reloc.instruction = leaOffset;
	reloc.offset = out->len - 1;
	reloc.target = m_operands[1].block;
	out->relocs.push_back(reloc);

	return true;
}


void X86_SYMINSTR_CLASS(CallPopCodeAddr)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "call/pop ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(FstenvDataAddr)::X86_SYMINSTR_CLASS(FstenvDataAddr)(uint32_t dest, int64_t offset)
{
	AddWriteRegisterOperand(dest);
	AddGlobalVarOperand(offset);
}


bool X86_SYMINSTR_CLASS(FstenvDataAddr)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	size_t capturedOffset = out->len;
	EMIT(fnop);
#ifdef OUTPUT32
	EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), func->GetSettings().stackGrowsUp ? 4 : -28));
	EMIT_RM(mov_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
		func->GetSettings().stackGrowsUp ? 16 : -16));
	size_t leaOffset = out->len;
	EMIT_RM(lea_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_MEM(X86_REG_OF_SIZE(m_operands[0].reg, 32), 1));
#else
	EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), func->GetSettings().stackGrowsUp ? 8 : -28));
	EMIT_RM(mov_64, X86_REG_OF_SIZE(m_operands[0].reg, 64), X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
		func->GetSettings().stackGrowsUp ? 20 : -16));
	size_t leaOffset = out->len;
	EMIT_RM(lea_64, X86_REG_OF_SIZE(m_operands[0].reg, 64), X86_MEM(X86_REG_OF_SIZE(m_operands[0].reg, 64), 1));
#endif

	*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

	Relocation reloc;
	reloc.type = DATA_RELOC_RELATIVE_8;
	reloc.overflow = LeaOverflowHandler;
	reloc.instruction = leaOffset;
	reloc.offset = out->len - 1;
	reloc.dataOffset = m_operands[1].immed;
	out->relocs.push_back(reloc);

	return true;
}


void X86_SYMINSTR_CLASS(FstenvDataAddr)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "fstenv ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(FstenvCodeAddr)::X86_SYMINSTR_CLASS(FstenvCodeAddr)(uint32_t dest, Function* func, ILBlock* block)
{
	AddWriteRegisterOperand(dest);
	AddBlockOperand(func, block);
}


bool X86_SYMINSTR_CLASS(FstenvCodeAddr)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	size_t capturedOffset = out->len;
	EMIT(fnop);
#ifdef OUTPUT32
	EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), func->GetSettings().stackGrowsUp ? 4 : -28));
	EMIT_RM(mov_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
		func->GetSettings().stackGrowsUp ? 16 : -16));
	size_t leaOffset = out->len;
	EMIT_RM(lea_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_MEM(X86_REG_OF_SIZE(m_operands[0].reg, 32), 1));
#else
	EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), func->GetSettings().stackGrowsUp ? 8 : -28));
	EMIT_RM(mov_64, X86_REG_OF_SIZE(m_operands[0].reg, 64), X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
		func->GetSettings().stackGrowsUp ? 20 : -16));
	size_t leaOffset = out->len;
	EMIT_RM(lea_64, X86_REG_OF_SIZE(m_operands[0].reg, 64), X86_MEM(X86_REG_OF_SIZE(m_operands[0].reg, 64), 1));
#endif

	*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_8;
	reloc.overflow = LeaOverflowHandler;
	reloc.instruction = leaOffset;
	reloc.offset = out->len - 1;
	reloc.target = m_operands[1].block;
	out->relocs.push_back(reloc);

	return true;
}


void X86_SYMINSTR_CLASS(FstenvCodeAddr)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "fstenv ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(MovDataPtrBaseRelative)::X86_SYMINSTR_CLASS(MovDataPtrBaseRelative)(uint32_t dest, uint32_t base, int64_t offset)
{
	AddWriteRegisterOperand(dest);
	AddReadRegisterOperand(base);
	AddGlobalVarOperand(offset);
}


bool X86_SYMINSTR_CLASS(MovDataPtrBaseRelative)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
#ifdef OUTPUT32
	size_t leaOffset = out->len;
	EMIT_RM(lea_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_MEM(X86_REG_OF_SIZE(m_operands[1].reg, 32), 1));
	*(int8_t*)((size_t)out->code + out->len - 1) = 0;
	Relocation reloc;
	reloc.type = DATA_RELOC_BASE_RELATIVE_8;
	reloc.overflow = BaseRelativeLeaOverflowHandler;
	reloc.instruction = leaOffset;
	reloc.offset = out->len - 1;
	reloc.dataOffset = m_operands[2].immed;
	out->relocs.push_back(reloc);
#else
	EMIT_RM(lea_64, X86_REG_OF_SIZE(m_operands[0].reg, 64), X86_MEM(REG_RIP, 0));
	Relocation reloc;
	reloc.type = DATA_RELOC_RELATIVE_32;
	reloc.offset = out->len - 4;
	reloc.dataOffset = m_operands[2].immed;
	out->relocs.push_back(reloc);
#endif
	return true;
}


void X86_SYMINSTR_CLASS(MovDataPtrBaseRelative)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "mov ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", rel:");
	m_operands[1].Print(func);
	fprintf(stderr, " ");
	m_operands[2].Print(func);
}


X86_SYMINSTR_CLASS(MovCodePtrBaseRelative)::X86_SYMINSTR_CLASS(MovCodePtrBaseRelative)(uint32_t dest, uint32_t base,
	Function* func, ILBlock* block)
{
	AddWriteRegisterOperand(dest);
	AddReadRegisterOperand(base);
	AddBlockOperand(func, block);
}


bool X86_SYMINSTR_CLASS(MovCodePtrBaseRelative)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
#ifdef OUTPUT32
	size_t leaOffset = out->len;
	EMIT_RM(lea_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), X86_MEM(X86_REG_OF_SIZE(m_operands[1].reg, 32), 1));
	*(int8_t*)((size_t)out->code + out->len - 1) = 0;
	Relocation reloc;
	reloc.type = CODE_RELOC_BASE_RELATIVE_8;
	reloc.overflow = BaseRelativeLeaOverflowHandler;
	reloc.instruction = leaOffset;
	reloc.offset = out->len - 1;
	reloc.target = m_operands[2].block;
	out->relocs.push_back(reloc);
#else
	EMIT_RM(lea_64, X86_REG_OF_SIZE(m_operands[0].reg, 64), X86_MEM(REG_RIP, 0));

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32;
	reloc.offset = out->len - 4;
	reloc.target = m_operands[2].block;
	out->relocs.push_back(reloc);
#endif
	return true;
}


void X86_SYMINSTR_CLASS(MovCodePtrBaseRelative)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "mov ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", rel: ");
	m_operands[1].Print(func);
	fprintf(stderr, " ");
	m_operands[2].Print(func);
}


X86_SYMINSTR_CLASS(MovDataPtrAbsolute)::X86_SYMINSTR_CLASS(MovDataPtrAbsolute)(uint32_t dest, int64_t offset)
{
	AddWriteRegisterOperand(dest);
	AddGlobalVarOperand(offset);
}


bool X86_SYMINSTR_CLASS(MovDataPtrAbsolute)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
#ifdef OUTPUT32
	EMIT_RI(mov_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), 0);
	Relocation reloc;
	reloc.type = DATA_RELOC_ABSOLUTE_32;
	reloc.offset = out->len - 4;
	reloc.dataOffset = m_operands[1].immed;
	out->relocs.push_back(reloc);
	return true;
#else
	return false;
#endif
}


void X86_SYMINSTR_CLASS(MovDataPtrAbsolute)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "mov ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(MovCodePtrAbsolute)::X86_SYMINSTR_CLASS(MovCodePtrAbsolute)(uint32_t dest, Function* func, ILBlock* block)
{
	AddWriteRegisterOperand(dest);
	AddBlockOperand(func, block);
}


bool X86_SYMINSTR_CLASS(MovCodePtrAbsolute)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
#ifdef OUTPUT32
	EMIT_RI(mov_32, X86_REG_OF_SIZE(m_operands[0].reg, 32), 0);
	Relocation reloc;
	reloc.type = CODE_RELOC_ABSOLUTE_32;
	reloc.offset = out->len - 4;
	reloc.target = m_operands[1].block;
	out->relocs.push_back(reloc);
	return true;
#else
	return false;
#endif
}


void X86_SYMINSTR_CLASS(MovCodePtrAbsolute)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "mov ");
	X86_PRINT_REG_OP(0, 32);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(CondJump)::X86_SYMINSTR_CLASS(CondJump)(uint8_t type, Function* func, ILBlock* block)
{
	AddImmediateOperand(type);
	AddBlockOperand(func, block);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	EnableFlag(SYMFLAG_USES_FLAGS);
}


bool X86_SYMINSTR_CLASS(CondJump)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
	buffer[0] = 0x70 + (uint8_t)m_operands[0].immed;
	buffer[1] = 0;
	out->FinishWrite(2);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_8;
	reloc.overflow = ConditionalJumpOverflowHandler;
	reloc.instruction = out->len - 2;
	reloc.offset = out->len - 1;
	reloc.target = m_operands[1].block;
	out->relocs.push_back(reloc);

	return true;
}


void X86_SYMINSTR_CLASS(CondJump)::Print(SymInstrFunction* func)
{
	switch (m_operands[0].immed)
	{
	case 0:
		fprintf(stderr, "jo ");
		break;
	case 1:
		fprintf(stderr, "jno ");
		break;
	case 2:
		fprintf(stderr, "jb ");
		break;
	case 3:
		fprintf(stderr, "jae ");
		break;
	case 4:
		fprintf(stderr, "jz ");
		break;
	case 5:
		fprintf(stderr, "jnz ");
		break;
	case 6:
		fprintf(stderr, "jbe ");
		break;
	case 7:
		fprintf(stderr, "ja ");
		break;
	case 8:
		fprintf(stderr, "js ");
		break;
	case 9:
		fprintf(stderr, "jns ");
		break;
	case 0xa:
		fprintf(stderr, "jp ");
		break;
	case 0xb:
		fprintf(stderr, "jnp ");
		break;
	case 0xc:
		fprintf(stderr, "jl ");
		break;
	case 0xd:
		fprintf(stderr, "jge ");
		break;
	case 0xe:
		fprintf(stderr, "jle ");
		break;
	case 0xf:
		fprintf(stderr, "jg ");
		break;
	default:
		fprintf(stderr, "condjump%lld ", (long long)m_operands[0].immed);
		break;
	}

	m_operands[1].Print(func);
}


X86_SYMINSTR_CLASS(JumpRelative)::X86_SYMINSTR_CLASS(JumpRelative)(Function* func, ILBlock* block)
{
	AddBlockOperand(func, block);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
}


bool X86_SYMINSTR_CLASS(JumpRelative)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
	buffer[0] = 0xeb;
	buffer[1] = 0;
	out->FinishWrite(2);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_8;
	reloc.overflow = UnconditionalJumpOverflowHandler;
	reloc.instruction = out->len - 2;
	reloc.offset = out->len - 1;
	reloc.target = m_operands[0].block;
	out->relocs.push_back(reloc);

	return true;
}


void X86_SYMINSTR_CLASS(JumpRelative)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "jmpn ");
	m_operands[0].Print(func);
}


X86_SYMINSTR_CLASS(CallDirect)::X86_SYMINSTR_CLASS(CallDirect)(Function* func, ILBlock* block,
	uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch)
{
	AddBlockOperand(func, block);
	AddWriteRegisterOperand(retVal);
	AddWriteRegisterOperand(retValHigh);
	AddReadRegisterOperand(key);
	AddTemporaryRegisterOperand(scratch);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	EnableFlag(SYMFLAG_CALL);
}


bool X86_SYMINSTR_CLASS(CallDirect)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	bool normalStack = true;
	if ((func->GetSettings().stackPointer != SYMREG_NATIVE_REG(REG_ESP)) || func->GetSettings().stackGrowsUp)
		normalStack = false;

	if (func->GetSettings().encodePointers || (!normalStack))
	{
		OperandType retAddr = X86_REG_OF_SIZE(m_operands[4].reg, X86_NATIVE_SIZE);

		// Generate code to get return address and add a relocation for it
#ifdef OUTPUT32
		Relocation reloc;
		if (func->GetSettings().basePointer != SYMREG_NONE)
		{
			size_t leaOffset = out->len;
			EMIT_RM(lea_32, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().basePointer, 32), 1));
			*(int8_t*)((size_t)out->code + out->len - 1) = 0;
			reloc.type = CODE_RELOC_BASE_RELATIVE_8;
			reloc.overflow = BaseRelativeLeaOverflowHandler;
			reloc.instruction = leaOffset;
			reloc.offset = out->len - 1;
			reloc.target = NULL;
		}
		else if (func->GetSettings().positionIndependent)
		{
			size_t capturedOffset, leaOffset;

			if (normalStack)
			{
				// Normal stack, use call/pop method
				uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
				buffer[0] = 0xe8;
				*(uint32_t*)(&buffer[1]) = 0;
				out->FinishWrite(5);
				capturedOffset = out->len;
				EMIT_R(pop, retAddr);
				leaOffset = out->len;
				EMIT_RM(lea_32, retAddr, X86_MEM(retAddr, 1));
			}
			else
			{
				// Not a normal stack, must use fstenv method
				capturedOffset = out->len;
				EMIT(fnop);
#ifdef OUTPUT32
				EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
					func->GetSettings().stackGrowsUp ? 4 : -28));
				EMIT_RM(mov_32, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
					func->GetSettings().stackGrowsUp ? 16 : -16));
				leaOffset = out->len;
				EMIT_RM(lea_32, retAddr, X86_MEM(retAddr, 1));
#else
				EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
					func->GetSettings().stackGrowsUp ? 8 : -28));
				EMIT_RM(mov_64, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
					func->GetSettings().stackGrowsUp ? 20 : -16));
				leaOffset = out->len;
				EMIT_RM(lea_64, retAddr, X86_MEM(retAddr, 1));
#endif
			}

			*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

			reloc.type = CODE_RELOC_RELATIVE_8;
			reloc.offset = out->len - 1;
			reloc.target = NULL;
		}
		else
		{
			EMIT_RI(mov_32, retAddr, 0);
			reloc.type = CODE_RELOC_ABSOLUTE_32;
			reloc.offset = out->len - 4;
			reloc.target = NULL;
		}
#else
		EMIT_RM(lea_64, retAddr, X86_MEM(REG_RIP, 0));

		Relocation reloc;
		reloc.type = CODE_RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = NULL;
#endif

		size_t beforeLen = out->len;

		if (func->GetSettings().encodePointers)
		{
			OperandType key = X86_REG_OF_SIZE(m_operands[3].reg, X86_NATIVE_SIZE);
#ifdef OUTPUT32
			EMIT_RR(xor_32, retAddr, key);
#else
			EMIT_RR(xor_64, retAddr, key);
#endif
		}

		// Push return address and jump to function
		if (normalStack)
			EMIT_R(push, retAddr);
		else
		{
#ifdef OUTPUT32
			EMIT_RM(lea_32, X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
				X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), func->GetSettings().stackGrowsUp ? 4 : -4));
			EMIT_MR(mov_32, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), 0), retAddr);
#else
			EMIT_RM(lea_64, X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
				X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), func->GetSettings().stackGrowsUp ? 8 : -8));
			EMIT_MR(mov_64, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), 0), retAddr);
#endif
		}

		uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
		buffer[0] = 0xeb;
		buffer[1] = 0;
		out->FinishWrite(2);

		Relocation callReloc;
		callReloc.type = CODE_RELOC_RELATIVE_8;
		callReloc.overflow = UnconditionalJumpOverflowHandler;
		callReloc.instruction = out->len - 2;
		callReloc.offset = out->len - 1;
		callReloc.target = m_operands[0].block;
		out->relocs.push_back(callReloc);

		// Fix up relocation to point to return address
		size_t afterLen = out->len;
		reloc.start = beforeLen;
		reloc.end = afterLen;
#ifdef OUTPUT32
		if (func->GetSettings().positionIndependent)
			*(int8_t*)((size_t)out->code + reloc.offset) += (int8_t)(afterLen - beforeLen);
		else
			*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
		out->relocs.push_back(reloc);
#else
		*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
		out->relocs.push_back(reloc);
#endif
	}
	else
	{
		// Normal call
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
		buffer[0] = 0xe8;
		*(uint32_t*)(&buffer[1]) = 0;
		out->FinishWrite(5);

		Relocation reloc;
		reloc.type = CODE_RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = m_operands[0].block;
		out->relocs.push_back(reloc);
	}

	return true;
}


void X86_SYMINSTR_CLASS(CallDirect)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "calln ");
	m_operands[0].Print(func);
	fprintf(stderr, ", ret ");
	X86_PRINT_REG_OP(1, X86_NATIVE_SIZE);
	if (m_operands[2].reg != SYMREG_NONE)
	{
		fprintf(stderr, ", ret ");
		X86_PRINT_REG_OP(2, X86_NATIVE_SIZE);
	}
	fprintf(stderr, ", key ");
	X86_PRINT_REG_OP(3, X86_NATIVE_SIZE);
	fprintf(stderr, ", scratch ");
	X86_PRINT_REG_OP(4, X86_NATIVE_SIZE);
}


X86_SYMINSTR_CLASS(CallIndirectReg)::X86_SYMINSTR_CLASS(CallIndirectReg)(uint32_t func,
	uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch)
{
	AddReadRegisterOperand(func);
	AddWriteRegisterOperand(retVal);
	AddWriteRegisterOperand(retValHigh);
	AddReadRegisterOperand(key);
	AddTemporaryRegisterOperand(scratch);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	EnableFlag(SYMFLAG_CALL);
}


bool X86_SYMINSTR_CLASS(CallIndirectReg)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	bool normalStack = true;
	if ((func->GetSettings().stackPointer != SYMREG_NATIVE_REG(REG_ESP)) || func->GetSettings().stackGrowsUp)
		normalStack = false;

	if (func->GetSettings().encodePointers || (!normalStack))
	{
		// Encoded pointer call or call with alternate stack pointer, push
		// return address then jump to function
		OperandType retAddr = X86_REG_OF_SIZE(m_operands[4].reg, X86_NATIVE_SIZE);

		// Generate code to get return address and add a relocation for it
#ifdef OUTPUT32
		Relocation reloc;
		if (func->GetSettings().basePointer != SYMREG_NONE)
		{
			size_t leaOffset = out->len;
			EMIT_RM(lea_32, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().basePointer, 32), 1));
			*(int8_t*)((size_t)out->code + out->len - 1) = 0;
			reloc.type = CODE_RELOC_BASE_RELATIVE_8;
			reloc.overflow = BaseRelativeLeaOverflowHandler;
			reloc.instruction = leaOffset;
			reloc.offset = out->len - 1;
			reloc.target = NULL;
		}
		else if (func->GetSettings().positionIndependent)
		{
			size_t capturedOffset, leaOffset;

			if (normalStack)
			{
				// Normal stack, use call/pop method
				uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
				buffer[0] = 0xe8;
				*(uint32_t*)(&buffer[1]) = 0;
				out->FinishWrite(5);
				capturedOffset = out->len;
				EMIT_R(pop, retAddr);
				leaOffset = out->len;
				EMIT_RM(lea_32, retAddr, X86_MEM(retAddr, 1));
			}
			else
			{
				// Not a normal stack, must use fstenv method
				capturedOffset = out->len;
				EMIT(fnop);
#ifdef OUTPUT32
				EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
					func->GetSettings().stackGrowsUp ? 4 : -28));
				EMIT_RM(mov_32, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
					func->GetSettings().stackGrowsUp ? 16 : -16));
				leaOffset = out->len;
				EMIT_RM(lea_32, retAddr, X86_MEM(retAddr, 1));
#else
				EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
					func->GetSettings().stackGrowsUp ? 8 : -28));
				EMIT_RM(mov_64, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
					func->GetSettings().stackGrowsUp ? 20 : -16));
				leaOffset = out->len;
				EMIT_RM(lea_64, retAddr, X86_MEM(retAddr, 1));
#endif
			}

			*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

			reloc.type = CODE_RELOC_RELATIVE_8;
			reloc.offset = out->len - 1;
			reloc.target = NULL;
		}
		else
		{
			EMIT_RI(mov_32, retAddr, 0);
			reloc.type = CODE_RELOC_ABSOLUTE_32;
			reloc.offset = out->len - 4;
			reloc.target = NULL;
		}
#else
		EMIT_RM(lea_64, retAddr, X86_MEM(REG_RIP, 0));

		Relocation reloc;
		reloc.type = CODE_RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = NULL;
#endif

		size_t beforeLen = out->len;

		if (func->GetSettings().encodePointers)
		{
			// Encode return address
			OperandType key = X86_REG_OF_SIZE(m_operands[3].reg, X86_NATIVE_SIZE);
#ifdef OUTPUT32
			EMIT_RR(xor_32, retAddr, key);
#else
			EMIT_RR(xor_64, retAddr, key);
#endif
		}

		// Push return address and jump to destination
		if (normalStack)
			EMIT_R(push, retAddr);
		else
		{
#ifdef OUTPUT32
			EMIT_RM(lea_32, X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
				X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), func->GetSettings().stackGrowsUp ? 4 : -4));
			EMIT_MR(mov_32, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), 0), retAddr);
#else
			EMIT_RM(lea_64, X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
				X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), func->GetSettings().stackGrowsUp ? 8 : -8));
			EMIT_MR(mov_64, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), 0), retAddr);
#endif
		}

		OperandType funcReg = X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE);

		if (func->GetSettings().encodePointers)
		{
			// Decode pointer before calling
			OperandType key = X86_REG_OF_SIZE(m_operands[3].reg, X86_NATIVE_SIZE);
#ifdef OUTPUT32
			EMIT_RR(mov_32, retAddr, funcReg);
			EMIT_RR(xor_32, retAddr, key);
#else
			EMIT_RR(mov_64, retAddr, funcReg);
			EMIT_RR(xor_64, retAddr, key);
#endif
			funcReg = retAddr;
		}

		EMIT_R(jmpn, funcReg);

		// Fix up relocation to point to return address
		size_t afterLen = out->len;
		reloc.start = beforeLen;
		reloc.end = afterLen;
#ifdef OUTPUT32
		if (func->GetSettings().positionIndependent)
			*(int8_t*)((size_t)out->code + reloc.offset) += (int8_t)(afterLen - beforeLen);
		else
			*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
		out->relocs.push_back(reloc);
#else
		*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
		out->relocs.push_back(reloc);
#endif
	}
	else
	{
		OperandType func = X86_REG_OF_SIZE(m_operands[0].reg, X86_NATIVE_SIZE);
		EMIT_R(calln, func);
	}

	return true;
}


void X86_SYMINSTR_CLASS(CallIndirectReg)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "calln ");
	X86_PRINT_REG_OP(0, X86_NATIVE_SIZE);
	fprintf(stderr, ", ret ");
	X86_PRINT_REG_OP(1, X86_NATIVE_SIZE);
	if (m_operands[2].reg != SYMREG_NONE)
	{
		fprintf(stderr, ", ret ");
		X86_PRINT_REG_OP(2, X86_NATIVE_SIZE);
	}
	fprintf(stderr, ", key ");
	X86_PRINT_REG_OP(3, X86_NATIVE_SIZE);
	fprintf(stderr, ", scratch ");
	X86_PRINT_REG_OP(4, X86_NATIVE_SIZE);
}


X86_SYMINSTR_CLASS(CallIndirectMem)::X86_SYMINSTR_CLASS(CallIndirectMem)(X86_MEM_OP_PARAM,
	uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch)
{
	X86_ADD_MEM_OP;
	AddWriteRegisterOperand(retVal);
	AddWriteRegisterOperand(retValHigh);
	AddReadRegisterOperand(key);
	AddTemporaryRegisterOperand(scratch);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	EnableFlag(SYMFLAG_CALL);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
}


bool X86_SYMINSTR_CLASS(CallIndirectMem)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	bool normalStack = true;
	if ((func->GetSettings().stackPointer != SYMREG_NATIVE_REG(REG_ESP)) || func->GetSettings().stackGrowsUp)
		normalStack = false;

	if (func->GetSettings().encodePointers || (!normalStack))
	{
		// Encoded pointer call or call with alternate stack pointer, push
		// return address then jump to function
		OperandType retAddr = X86_REG_OF_SIZE(m_operands[7].reg, X86_NATIVE_SIZE);

		// Generate code to get return address and add a relocation for it
#ifdef OUTPUT32
		Relocation reloc;
		if (func->GetSettings().basePointer != SYMREG_NONE)
		{
			size_t leaOffset = out->len;
			EMIT_RM(lea_32, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().basePointer, 32), 1));
			*(int8_t*)((size_t)out->code + out->len - 1) = 0;
			reloc.type = CODE_RELOC_BASE_RELATIVE_8;
			reloc.overflow = BaseRelativeLeaOverflowHandler;
			reloc.instruction = leaOffset;
			reloc.offset = out->len - 1;
			reloc.target = NULL;
		}
		else if (func->GetSettings().positionIndependent)
		{
			size_t capturedOffset, leaOffset;

			if (normalStack)
			{
				// Normal stack, use call/pop method
				uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
				buffer[0] = 0xe8;
				*(uint32_t*)(&buffer[1]) = 0;
				out->FinishWrite(5);
				capturedOffset = out->len;
				EMIT_R(pop, retAddr);
				leaOffset = out->len;
				EMIT_RM(lea_32, retAddr, X86_MEM(retAddr, 1));
			}
			else
			{
				// Not a normal stack, must use fstenv method
				capturedOffset = out->len;
				EMIT(fnop);
#ifdef OUTPUT32
				EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
					func->GetSettings().stackGrowsUp ? 4 : -28));
				EMIT_RM(mov_32, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
					func->GetSettings().stackGrowsUp ? 16 : -16));
				leaOffset = out->len;
				EMIT_RM(lea_32, retAddr, X86_MEM(retAddr, 1));
#else
				EMIT_M(fstenv, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
					func->GetSettings().stackGrowsUp ? 8 : -28));
				EMIT_RM(mov_64, retAddr, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
					func->GetSettings().stackGrowsUp ? 20 : -16));
				leaOffset = out->len;
				EMIT_RM(lea_64, retAddr, X86_MEM(retAddr, 1));
#endif
			}

			*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);

			reloc.type = CODE_RELOC_RELATIVE_8;
			reloc.offset = out->len - 1;
			reloc.target = NULL;
		}
		else
		{
			EMIT_RI(mov_32, retAddr, 0);
			reloc.type = CODE_RELOC_ABSOLUTE_32;
			reloc.offset = out->len - 4;
			reloc.target = NULL;
		}
#else
		EMIT_RM(lea_64, retAddr, X86_MEM(REG_RIP, 0));

		Relocation reloc;
		reloc.type = CODE_RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = NULL;
#endif

		size_t beforeLen = out->len;

		if (func->GetSettings().encodePointers)
		{
			// Encode return address
			OperandType key = X86_REG_OF_SIZE(m_operands[6].reg, X86_NATIVE_SIZE);
#ifdef OUTPUT32
			EMIT_RR(xor_32, retAddr, key);
#else
			EMIT_RR(xor_64, retAddr, key);
#endif
		}

		// Push return address and jump to destination
		if (normalStack)
			EMIT_R(push, retAddr);
		else
		{
#ifdef OUTPUT32
			EMIT_RM(lea_32, X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32),
				X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), func->GetSettings().stackGrowsUp ? 4 : -4));
			EMIT_MR(mov_32, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 32), 0), retAddr);
#else
			EMIT_RM(lea_64, X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64),
				X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), func->GetSettings().stackGrowsUp ? 8 : -8));
			EMIT_MR(mov_64, X86_MEM(X86_REG_OF_SIZE(func->GetSettings().stackPointer, 64), 0), retAddr);
#endif
		}

		if (func->GetSettings().encodePointers)
		{
			// Decode pointer before calling
			OperandType key = X86_REG_OF_SIZE(m_operands[6].reg, X86_NATIVE_SIZE);
#ifdef OUTPUT32
			EMIT_RM(mov_32, retAddr, X86_MEM_OP(0));
			EMIT_RR(xor_32, retAddr, key);
#else
			EMIT_RM(mov_64, retAddr, X86_MEM_OP(0));
			EMIT_RR(xor_64, retAddr, key);
#endif
			EMIT_R(jmpn, retAddr);
		}
		else
		{
			EMIT_M(jmpn, X86_MEM_OP(0));
		}

		// Fix up relocation to point to return address
		size_t afterLen = out->len;
		reloc.start = beforeLen;
		reloc.end = afterLen;
#ifdef OUTPUT32
		if (func->GetSettings().positionIndependent)
			*(int8_t*)((size_t)out->code + reloc.offset) += (int8_t)(afterLen - beforeLen);
		else
			*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
		out->relocs.push_back(reloc);
#else
		*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
		out->relocs.push_back(reloc);
#endif
	}
	else
	{
		EMIT_M(calln, X86_MEM_OP(0));
	}

	return true;
}


void X86_SYMINSTR_CLASS(CallIndirectMem)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "calln ");
	X86_PRINT_MEM_OP(0, X86_NATIVE_SIZE);
	fprintf(stderr, ", ret ");
	X86_PRINT_REG_OP(4, X86_NATIVE_SIZE);
	if (m_operands[5].reg != SYMREG_NONE)
	{
		fprintf(stderr, ", ret ");
		X86_PRINT_REG_OP(5, X86_NATIVE_SIZE);
	}
	fprintf(stderr, ", key ");
	X86_PRINT_REG_OP(6, X86_NATIVE_SIZE);
	fprintf(stderr, ", scratch ");
	X86_PRINT_REG_OP(7, X86_NATIVE_SIZE);
}


X86_SYMINSTR_CLASS(Syscall)::X86_SYMINSTR_CLASS(Syscall)(uint32_t eax, uint32_t edx, uint32_t ecx, const vector<uint32_t> readRegs)
{
	AddWriteRegisterOperand(eax);
	AddWriteRegisterOperand(edx);
	AddWriteRegisterOperand(ecx);
	for (vector<uint32_t>::const_iterator i = readRegs.begin(); i != readRegs.end(); i++)
		AddReadRegisterOperand(*i);
}


bool X86_SYMINSTR_CLASS(Syscall)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT(syscall);
	return true;
}


void X86_SYMINSTR_CLASS(Syscall)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "syscall ");
	for (size_t i = 0; i < m_operands.size(); i++)
	{
		if (i != 0)
			fprintf(stderr, ", ");
		X86_PRINT_REG_OP(i, 32);
	}
}


X86_SYMINSTR_CLASS(SyscallInt80)::X86_SYMINSTR_CLASS(SyscallInt80)(uint32_t eax, uint32_t edx, uint32_t ecx,
	const vector<uint32_t> readRegs)
{
	AddWriteRegisterOperand(eax);
	AddWriteRegisterOperand(edx);
	AddWriteRegisterOperand(ecx);
	for (vector<uint32_t>::const_iterator i = readRegs.begin(); i != readRegs.end(); i++)
		AddReadRegisterOperand(*i);
}


bool X86_SYMINSTR_CLASS(SyscallInt80)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	EMIT_I(int, 0x80);
	return true;
}


void X86_SYMINSTR_CLASS(SyscallInt80)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "int80 ");
	for (size_t i = 0; i < m_operands.size(); i++)
	{
		if (i != 0)
			fprintf(stderr, ", ");
		X86_PRINT_REG_OP(i, 32);
	}
}


X86_SYMINSTR_CLASS(SyscallCorrectErrorCode)::X86_SYMINSTR_CLASS(SyscallCorrectErrorCode)(uint32_t eax)
{
	AddReadWriteRegisterOperand(eax);
}


bool X86_SYMINSTR_CLASS(SyscallCorrectErrorCode)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	// If carry flag is set, syscall failed, so place negative of error code into result
	uint8_t* errorCode = (uint8_t*)out->PrepareWrite(4);
	errorCode[0] = 0x73; // jnc noerror
	errorCode[1] = 0x02;
	errorCode[2] = 0xf7; // neg eax
	errorCode[3] = 0xd8;
	out->FinishWrite(4);
	return true;
}


void X86_SYMINSTR_CLASS(SyscallCorrectErrorCode)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "errorcode ");
	X86_PRINT_REG_OP(0, 32);
}


X86_SYMINSTR_CLASS(SymReturn)::X86_SYMINSTR_CLASS(SymReturn)(uint32_t a, uint32_t b)
{
	AddReadRegisterOperand(a);
	if (b != SYMREG_NONE)
		AddReadRegisterOperand(b);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
}


bool X86_SYMINSTR_CLASS(SymReturn)::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	return true;
}


void X86_SYMINSTR_CLASS(SymReturn)::Print(SymInstrFunction* func)
{
	fprintf(stderr, "symreturn ");
	X86_PRINT_REG_OP(0, X86_NATIVE_SIZE);
	if (m_operands.size() > 1)
	{
		fprintf(stderr, ", ");
		X86_PRINT_REG_OP(1, X86_NATIVE_SIZE);
	}
}


bool X86_SYMINSTR_CLASS(SymReturn)::UpdateInstruction(SymInstrFunction* func, const Settings& settings, vector<SymInstr*>& replacement)
{
	// This pseudo-instruction is only present for data flow analysis, replace with nothing
	return true;
}


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
		case REG_RIP:  return REG_RIP;
		default:  return NONE;
		}
	default:
		return NONE;
	}
}


void X86_SYMINSTR_NAME(Instr)::LeaOverflowHandler(OutputBlock* out, Relocation& reloc)
{
	size_t start = reloc.instruction;
	size_t offset = reloc.offset;
	uint8_t* instr = (uint8_t*)((size_t)out->code + start);

#ifdef OUTPUT64
	// Skip REX prefix
	if ((instr[0] & 0xf0) == 0x40)
	{
		start++;
		instr++;
	}
#endif

	// Use 32-bit displacement instead of 8-bit displacement
	uint8_t modrm = (instr[1] & 0x3f) | 0x80;

	uint8_t newInstr[15];
	newInstr[0] = instr[0];
	newInstr[1] = modrm;
	memcpy(&newInstr[2], &instr[2], (offset - start) - 2);
	*(uint32_t*)&newInstr[offset - start] = instr[offset - start] + 3;

	out->ReplaceInstruction(start, (offset - start) + 1, newInstr, (offset - start) + 4, offset - start);

	if (reloc.type == CODE_RELOC_RELATIVE_8)
		reloc.type = CODE_RELOC_RELATIVE_32;
	else if (reloc.type == DATA_RELOC_RELATIVE_8)
		reloc.type = DATA_RELOC_RELATIVE_32;
}


void X86_SYMINSTR_NAME(Instr)::BaseRelativeLeaOverflowHandler(OutputBlock* out, Relocation& reloc)
{
	size_t start = reloc.instruction;
	size_t offset = reloc.offset;
	uint8_t* instr = (uint8_t*)((size_t)out->code + start);

#ifdef OUTPUT64
	// Skip REX prefix
	if ((instr[0] & 0xf0) == 0x40)
	{
		start++;
		instr++;
	}
#endif

	// Use 32-bit displacement instead of 8-bit displacement
	uint8_t modrm = (instr[1] & 0x3f) | 0x80;

	uint8_t newInstr[15];
	newInstr[0] = instr[0];
	newInstr[1] = modrm;
	memcpy(&newInstr[2], &instr[2], (offset - start) - 2);
	*(uint32_t*)&newInstr[offset - start] = instr[offset - start];

	out->ReplaceInstruction(start, (offset - start) + 1, newInstr, (offset - start) + 4, offset - start);

	if (reloc.type == CODE_RELOC_BASE_RELATIVE_8)
		reloc.type = CODE_RELOC_BASE_RELATIVE_32;
	else if (reloc.type == DATA_RELOC_BASE_RELATIVE_8)
		reloc.type = DATA_RELOC_BASE_RELATIVE_32;
}


void X86_SYMINSTR_NAME(Instr)::ConditionalJumpOverflowHandler(OutputBlock* out, Relocation& reloc)
{
	size_t start = reloc.instruction;
	uint8_t* instr = (uint8_t*)((size_t)out->code + start);
	uint8_t newInstr[6];
	newInstr[0] = 0x0f;
	newInstr[1] = 0x80 + (instr[0] & 0x0f);
	*(uint32_t*)&newInstr[2] = instr[1];
	out->ReplaceInstruction(start, 2, newInstr, 6, 2);

	reloc.type = CODE_RELOC_RELATIVE_32;
}


void X86_SYMINSTR_NAME(Instr)::UnconditionalJumpOverflowHandler(OutputBlock* out, Relocation& reloc)
{
	size_t start = reloc.instruction;
	uint8_t* instr = (uint8_t*)((size_t)out->code + start);
	uint8_t newInstr[5];
	newInstr[0] = 0xe9;
	*(uint32_t*)&newInstr[1] = instr[1];
	out->ReplaceInstruction(start, 2, newInstr, 5, 1);

	reloc.type = CODE_RELOC_RELATIVE_32;
}


X86_SYMINSTR_NAME(Function)::X86_SYMINSTR_NAME(Function)(const Settings& settings): SymInstrFunction(settings)
{
}


vector<uint32_t> X86_SYMINSTR_NAME(Function)::GetCallerSavedRegisters()
{
	vector<uint32_t> result;
	// TODO: Handle alternate special registers
#ifdef OUTPUT32
	result.push_back(SYMREG_NATIVE_REG(REG_EAX));
	result.push_back(SYMREG_NATIVE_REG(REG_ECX));
	result.push_back(SYMREG_NATIVE_REG(REG_EDX));
#else
	result.push_back(SYMREG_NATIVE_REG(REG_EAX));
	result.push_back(SYMREG_NATIVE_REG(REG_ECX));
	result.push_back(SYMREG_NATIVE_REG(REG_EDX));
	result.push_back(SYMREG_NATIVE_REG(REG_R8D));
	result.push_back(SYMREG_NATIVE_REG(REG_R9D));
	result.push_back(SYMREG_NATIVE_REG(REG_R10D));
	result.push_back(SYMREG_NATIVE_REG(REG_R11D));
#endif
	return result;
}


vector<uint32_t> X86_SYMINSTR_NAME(Function)::GetCalleeSavedRegisters()
{
	vector<uint32_t> result;
	// TODO: Handle alternate special registers
#ifdef OUTPUT32
	result.push_back(SYMREG_NATIVE_REG(REG_EBX));
	result.push_back(SYMREG_NATIVE_REG(REG_ESI));
	result.push_back(SYMREG_NATIVE_REG(REG_EDI));
#else
	result.push_back(SYMREG_NATIVE_REG(REG_EBX));
	result.push_back(SYMREG_NATIVE_REG(REG_ESI));
	result.push_back(SYMREG_NATIVE_REG(REG_EDI));
	result.push_back(SYMREG_NATIVE_REG(REG_R12D));
	result.push_back(SYMREG_NATIVE_REG(REG_R13D));
	result.push_back(SYMREG_NATIVE_REG(REG_R14D));
	result.push_back(SYMREG_NATIVE_REG(REG_R15D));
#endif
	return result;
}


set<uint32_t> X86_SYMINSTR_NAME(Function)::GetRegisterClassInterferences(uint32_t cls)
{
	set<uint32_t> result;
	// TODO: Floating point
	// TODO: Handle alternate special registers

#define ADD_REG_TO_LIST(reg, exclude) \
	if ((SYMREG_NATIVE_REG(reg) != SYMREG_NATIVE_REG(exclude)) || (SYMREG_NATIVE_REG(reg) == m_settings.framePointer) || \
		(SYMREG_NATIVE_REG(reg) == m_settings.stackPointer)) \
		result.insert(SYMREG_NATIVE_REG(reg));
#ifdef OUTPUT32
#define ALL_EXCEPT(exclude) \
	ADD_REG_TO_LIST(REG_EAX, exclude) \
	ADD_REG_TO_LIST(REG_ECX, exclude) \
	ADD_REG_TO_LIST(REG_EDX, exclude) \
	ADD_REG_TO_LIST(REG_EBX, exclude) \
	ADD_REG_TO_LIST(REG_ESP, exclude) \
	ADD_REG_TO_LIST(REG_EBP, exclude) \
	ADD_REG_TO_LIST(REG_ESI, exclude) \
	ADD_REG_TO_LIST(REG_EDI, exclude)
#else
#define ALL_EXCEPT(exclude) \
	ADD_REG_TO_LIST(REG_EAX, exclude) \
	ADD_REG_TO_LIST(REG_ECX, exclude) \
	ADD_REG_TO_LIST(REG_EDX, exclude) \
	ADD_REG_TO_LIST(REG_EBX, exclude) \
	ADD_REG_TO_LIST(REG_ESP, exclude) \
	ADD_REG_TO_LIST(REG_EBP, exclude) \
	ADD_REG_TO_LIST(REG_ESI, exclude) \
	ADD_REG_TO_LIST(REG_EDI, exclude) \
	ADD_REG_TO_LIST(REG_R8D, exclude) \
	ADD_REG_TO_LIST(REG_R9D, exclude) \
	ADD_REG_TO_LIST(REG_R10D, exclude) \
	ADD_REG_TO_LIST(REG_R11D, exclude) \
	ADD_REG_TO_LIST(REG_R12D, exclude) \
	ADD_REG_TO_LIST(REG_R13D, exclude) \
	ADD_REG_TO_LIST(REG_R14D, exclude) \
	ADD_REG_TO_LIST(REG_R15D, exclude)
#endif

	switch (cls)
	{
#ifdef OUTPUT32
	case X86REGCLASS_INTEGER_8BIT:
		result.insert(SYMREG_NATIVE_REG(REG_ESP));
		result.insert(SYMREG_NATIVE_REG(REG_EBP));
		result.insert(SYMREG_NATIVE_REG(REG_ESI));
		result.insert(SYMREG_NATIVE_REG(REG_EDI));
		break;
#endif
#ifdef OUTPUT64
	case X86REGCLASS_INTEGER_INDEX:
		result.insert(SYMREG_NATIVE_REG(REG_ESP));
		result.insert(SYMREG_NATIVE_REG(REG_R12D));
		break;
#endif
	case X86REGCLASS_INTEGER_RETURN_VALUE:
		ALL_EXCEPT(REG_EAX);
		break;
	case X86REGCLASS_INTEGER_RETURN_VALUE_HIGH:
		ALL_EXCEPT(REG_EDX);
		break;
	case X86REGCLASS_EAX:
		ALL_EXCEPT(REG_EAX);
		break;
	case X86REGCLASS_ECX:
		ALL_EXCEPT(REG_ECX);
		break;
	case X86REGCLASS_EDX:
		ALL_EXCEPT(REG_EDX);
		break;
	case X86REGCLASS_ESI:
		ALL_EXCEPT(REG_ESI);
		break;
	case X86REGCLASS_EDI:
		ALL_EXCEPT(REG_EDI);
		break;
	case X86REGCLASS_ESP:
		ALL_EXCEPT(REG_ESP);
		break;
	case X86REGCLASS_SYSCALL_PARAM_0:
		ALL_EXCEPT(REG_EAX);
		break;
	case X86REGCLASS_SYSCALL_PARAM_1:
#ifdef OUTPUT32
		ALL_EXCEPT(REG_EBX);
#else
		ALL_EXCEPT(REG_EDI);
#endif
		break;
	case X86REGCLASS_SYSCALL_PARAM_2:
#ifdef OUTPUT32
		ALL_EXCEPT(REG_ECX);
#else
		ALL_EXCEPT(REG_ESI);
#endif
		break;
	case X86REGCLASS_SYSCALL_PARAM_3:
		ALL_EXCEPT(REG_EDX);
		break;
	case X86REGCLASS_SYSCALL_PARAM_4:
#ifdef OUTPUT32
		ALL_EXCEPT(REG_ESI);
#else
		if (m_settings.os == OS_FREEBSD) { ALL_EXCEPT(REG_ECX); } else { ALL_EXCEPT(REG_R10D); }
#endif
		break;
	case X86REGCLASS_SYSCALL_PARAM_5:
#ifdef OUTPUT32
		ALL_EXCEPT(REG_EDI);
#else
		ALL_EXCEPT(REG_R8D);
#endif
		break;
	case X86REGCLASS_SYSCALL_PARAM_6:
#ifdef OUTPUT32
		ALL_EXCEPT(REG_EBP);
#else
		ALL_EXCEPT(REG_R9D);
#endif
		break;
	case X86REGCLASS_SYSCALL_RESULT_1:
		ALL_EXCEPT(REG_EAX);
		break;
	case X86REGCLASS_SYSCALL_RESULT_2:
		ALL_EXCEPT(REG_EDX);
		break;
	default:
		break;
	}

	return result;
}


uint32_t X86_SYMINSTR_NAME(Function)::GetSpecialRegisterAssignment(uint32_t reg)
{
	switch (reg)
	{
	case SYMREG_SP:
		return m_settings.stackPointer;
	case SYMREG_BP:
		return m_settings.framePointer;
#ifdef OUTPUT64
	case SYMREG_IP:
		return SYMREG_NATIVE_REG(REG_RIP);
#endif
	default:
		return SYMREG_NONE;
	}
}


void X86_SYMINSTR_NAME(Function)::AdjustStackFrame()
{
}


void X86_SYMINSTR_NAME(Function)::PrintRegisterClass(uint32_t cls)
{
	switch (cls)
	{
	case X86REGCLASS_INTEGER:
		fprintf(stderr, "int");
		break;
	case X86REGCLASS_INTEGER_8BIT:
		fprintf(stderr, "int8");
		break;
	case X86REGCLASS_INTEGER_INDEX:
		fprintf(stderr, "index");
		break;
	case X86REGCLASS_FLOAT:
		fprintf(stderr, "float");
		break;
	case X86REGCLASS_INTEGER_RETURN_VALUE:
		fprintf(stderr, "retval");
		break;
	case X86REGCLASS_INTEGER_RETURN_VALUE_HIGH:
		fprintf(stderr, "retvalhigh");
		break;
	case X86REGCLASS_FLOAT_RETURN_VALUE:
		fprintf(stderr, "floatretval");
		break;
	case X86REGCLASS_INTEGER_PARAM_0:
	case X86REGCLASS_INTEGER_PARAM_1:
	case X86REGCLASS_INTEGER_PARAM_2:
	case X86REGCLASS_INTEGER_PARAM_3:
		fprintf(stderr, "intparam%d", cls - X86REGCLASS_INTEGER_PARAM_0);
		break;
	case X86REGCLASS_FLOAT_PARAM_0:
	case X86REGCLASS_FLOAT_PARAM_1:
	case X86REGCLASS_FLOAT_PARAM_2:
	case X86REGCLASS_FLOAT_PARAM_3:
		fprintf(stderr, "floatparam%d", cls - X86REGCLASS_FLOAT_PARAM_0);
		break;
	case X86REGCLASS_EAX:
		fprintf(stderr, "eax");
		break;
	case X86REGCLASS_ECX:
		fprintf(stderr, "ecx");
		break;
	case X86REGCLASS_EDX:
		fprintf(stderr, "edx");
		break;
	case X86REGCLASS_ESI:
		fprintf(stderr, "esi");
		break;
	case X86REGCLASS_EDI:
		fprintf(stderr, "edi");
		break;
	case X86REGCLASS_ESP:
		fprintf(stderr, "esp");
		break;
	case X86REGCLASS_SYSCALL_PARAM_0:
	case X86REGCLASS_SYSCALL_PARAM_1:
	case X86REGCLASS_SYSCALL_PARAM_2:
	case X86REGCLASS_SYSCALL_PARAM_3:
	case X86REGCLASS_SYSCALL_PARAM_4:
	case X86REGCLASS_SYSCALL_PARAM_5:
	case X86REGCLASS_SYSCALL_PARAM_6:
		fprintf(stderr, "sysparam%d", cls - X86REGCLASS_SYSCALL_PARAM_0);
		break;
	case X86REGCLASS_SYSCALL_RESULT_1:
		fprintf(stderr, "sysresult1");
		break;
	case X86REGCLASS_SYSCALL_RESULT_2:
		fprintf(stderr, "sysresult2");
		break;
	default:
		fprintf(stderr, "invalid");
		break;
	}
}


void X86_SYMINSTR_NAME(Function)::PrintRegister(uint32_t reg)
{
	if (!SYMREG_IS_SPECIAL_REG(reg))
		fprintf(stderr, "reg%d", reg);
	else if (reg < SYMREG_MIN_SPECIAL_REG)
		X86_SYMINSTR_NAME(PrintNativeOperand)((OperandType)(reg & 0x7fffffff));
	else
		SymInstrFunction::PrintRegister(reg);
}


SymInstr* X86_SYMINSTR_NAME_OP(lea, RM)(uint32_t a, X86_MEM_OP_PARAM) { return new X86_SYMINSTR_CLASS_OP(lea, RM)(a, X86_MEM_OP_PASS); }
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(rdtsc, 32, RR)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS_SIZE_OP(rdtsc, 32, RR)(a, b); }
SymInstr* X86_SYMINSTR_NAME(cwd)(uint32_t eax, uint32_t edx) { return new X86_SYMINSTR_CLASS(cwd)(eax, edx); }
SymInstr* X86_SYMINSTR_NAME(cdq)(uint32_t eax, uint32_t edx) { return new X86_SYMINSTR_CLASS(cdq)(eax, edx); }
#ifdef OUTPUT64
SymInstr* X86_SYMINSTR_NAME(cqo)(uint32_t eax, uint32_t edx) { return new X86_SYMINSTR_CLASS(cqo)(eax, edx); }
#endif
SymInstr* X86_SYMINSTR_NAME(rep_movsb)(uint32_t edi, uint32_t esi, uint32_t ecx) { return new X86_SYMINSTR_CLASS(rep_movsb)(edi, esi, ecx); }
SymInstr* X86_SYMINSTR_NAME(rep_stosb)(uint32_t edi, uint32_t eax, uint32_t ecx) { return new X86_SYMINSTR_CLASS(rep_stosb)(edi, eax, ecx); }
SymInstr* X86_SYMINSTR_NAME(repne_scasb)(uint32_t edi, uint32_t eax, uint32_t ecx) { return new X86_SYMINSTR_CLASS(repne_scasb)(edi, eax, ecx); }

SymInstr* X86_SYMINSTR_NAME(CallPopDataAddr)(uint32_t dest, int64_t offset) { return new X86_SYMINSTR_CLASS(CallPopDataAddr)(dest, offset); }
SymInstr* X86_SYMINSTR_NAME(CallPopCodeAddr)(uint32_t dest, Function* func, ILBlock* block) { return new X86_SYMINSTR_CLASS(CallPopCodeAddr)(dest, func, block); }
SymInstr* X86_SYMINSTR_NAME(FstenvDataAddr)(uint32_t dest, int64_t offset) { return new X86_SYMINSTR_CLASS(FstenvDataAddr)(dest, offset); }
SymInstr* X86_SYMINSTR_NAME(FstenvCodeAddr)(uint32_t dest, Function* func, ILBlock* block) { return new X86_SYMINSTR_CLASS(FstenvCodeAddr)(dest, func, block); }
SymInstr* X86_SYMINSTR_NAME(MovDataPtrBaseRelative)(uint32_t dest, uint32_t base, int64_t offset) { return new X86_SYMINSTR_CLASS(MovDataPtrBaseRelative)(dest, base, offset); }
SymInstr* X86_SYMINSTR_NAME(MovCodePtrBaseRelative)(uint32_t dest, uint32_t base, Function* func, ILBlock* block) { return new X86_SYMINSTR_CLASS(MovCodePtrBaseRelative)(dest, base, func, block); }
SymInstr* X86_SYMINSTR_NAME(MovDataPtrAbsolute)(uint32_t dest, int64_t offset) { return new X86_SYMINSTR_CLASS(MovDataPtrAbsolute)(dest, offset); }
SymInstr* X86_SYMINSTR_NAME(MovCodePtrAbsolute)(uint32_t dest, Function* func, ILBlock* block) { return new X86_SYMINSTR_CLASS(MovCodePtrAbsolute)(dest, func, block); }
SymInstr* X86_SYMINSTR_NAME(CondJump)(uint8_t type, Function* func, ILBlock* block) { return new X86_SYMINSTR_CLASS(CondJump)(type, func, block); }
SymInstr* X86_SYMINSTR_NAME(JumpRelative)(Function* func, ILBlock* block) { return new X86_SYMINSTR_CLASS(JumpRelative)(func, block); }
SymInstr* X86_SYMINSTR_NAME(CallDirect)(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch) { return new X86_SYMINSTR_CLASS(CallDirect)(func, block, retVal, retValHigh, key, scratch); }
SymInstr* X86_SYMINSTR_NAME(CallIndirectReg)(uint32_t func, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch) { return new X86_SYMINSTR_CLASS(CallIndirectReg)(func, retVal, retValHigh, key, scratch); }
SymInstr* X86_SYMINSTR_NAME(CallIndirectMem)(X86_MEM_OP_PARAM, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch) { return new X86_SYMINSTR_CLASS(CallIndirectMem)(X86_MEM_OP_PASS, retVal, retValHigh, key, scratch); }
SymInstr* X86_SYMINSTR_NAME(Syscall)(uint32_t eax, uint32_t edx, uint32_t ecx, const std::vector<uint32_t> readRegs) { return new X86_SYMINSTR_CLASS(Syscall)(eax, edx, ecx, readRegs); }
SymInstr* X86_SYMINSTR_NAME(SyscallInt80)(uint32_t eax, uint32_t edx, uint32_t ecx, const std::vector<uint32_t> readRegs) { return new X86_SYMINSTR_CLASS(SyscallInt80)(eax, edx, ecx, readRegs); }
SymInstr* X86_SYMINSTR_NAME(SyscallCorrectErrorCode)(uint32_t eax) { return new X86_SYMINSTR_CLASS(SyscallCorrectErrorCode)(eax); }
SymInstr* X86_SYMINSTR_NAME(SymReturn)(uint32_t a, uint32_t b) { return new X86_SYMINSTR_CLASS(SymReturn)(a, b); }


#endif

