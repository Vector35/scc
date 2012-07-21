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

#include "SymInstr.h"
#include "Output.h"
#include "asmx86.h"

// Don't try to combine these using X86_SYMINSTR_PREFIX, there be dragons here
#ifdef OUTPUT32
#define X86_SYMINSTR_NAME(name) X86Sym_ ## name
#define X86_SYMINSTR_NAME_SIZE(name, size) X86Sym_ ## name ## size
#define X86_SYMINSTR_NAME_OP(name, op) X86Sym_ ## name ## _ ## op
#define X86_SYMINSTR_NAME_SIZE_OP(name, size, op) X86Sym_ ## name ## size ## _ ## op
#define X86_SYMINSTR_CLASS(name) X86Sym_ ## name ## _Instr
#define X86_SYMINSTR_CLASS_SIZE(name, size) X86Sym_ ## name ## size ## _Instr
#define X86_SYMINSTR_CLASS_OP(name, op) X86Sym_ ## name ## _ ## op ## _Instr
#define X86_SYMINSTR_CLASS_SIZE_OP(name, size, op) X86Sym_ ## name ## size ## _ ## op ## _Instr
#else
#define X86_SYMINSTR_NAME(name) X64Sym_ ## name
#define X86_SYMINSTR_NAME_SIZE(name, size) X64Sym_ ## name ## size
#define X86_SYMINSTR_NAME_OP(name, op) X64Sym_ ## name ## _ ## op
#define X86_SYMINSTR_NAME_SIZE_OP(name, size, op) X64Sym_ ## name ## size ## _ ## op
#define X86_SYMINSTR_CLASS(name) X64Sym_ ## name ## _Instr
#define X86_SYMINSTR_CLASS_SIZE(name, size) X64Sym_ ## name ## size ## _Instr
#define X86_SYMINSTR_CLASS_OP(name, op) X64Sym_ ## name ## _ ## op ## _Instr
#define X86_SYMINSTR_CLASS_SIZE_OP(name, size, op) X64Sym_ ## name ## size ## _ ## op ## _Instr
#endif

#define X86_MEM_OP_PARAM uint32_t base, uint32_t index, uint32_t scale, int64_t offset
#define X86_MEM_OP_PASS base, index, scale, offset


class X86_SYMINSTR_NAME(Instr): public SymInstr
{
protected:
	asmx86::OperandType GetOperandOfSize(uint32_t reg, uint32_t size);
	void PrintNativeOperand(asmx86::OperandType reg);
};


#define X86_DECLARE_NO_OPERANDS(name) \
class X86_SYMINSTR_CLASS(name): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS(name)(); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME(name)();


#define X86_DECLARE_1OP_MODRM_NATIVE(name) \
class X86_SYMINSTR_CLASS_OP(name, R): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, R)(uint32_t a); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_OP(name, M): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, M)(X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_OP(name, R)(uint32_t a); \
SymInstr* X86_SYMINSTR_NAME_OP(name, M)(X86_MEM_OP_PARAM);


#define X86_DECLARE_1OP_MODRM_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, R): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(uint32_t a); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, M): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, R)(uint32_t a); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM);


#define X86_DECLARE_2OP_RM_NATIVE(name) \
class X86_SYMINSTR_CLASS_OP(name, RM): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, RM)(uint32_t a, X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_OP(name, RM)(uint32_t a, X86_MEM_OP_PARAM);


#define X86_DECLARE_2OP_RR_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b);


#define X86_DECLARE_2OP_RM_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RM)(uint32_t a, X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RM)(uint32_t a, X86_MEM_OP_PARAM);


#define X86_DECLARE_2OP_MR_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b);


#define X86_DECLARE_2OP_MODRM_SIZE(name, size) \
X86_DECLARE_2OP_RR_SIZE(name, size) \
X86_DECLARE_2OP_RM_SIZE(name, size) \
X86_DECLARE_2OP_MR_SIZE(name, size)


#define X86_DECLARE_2OP_MODRM_IMM_SIZE(name, size) \
X86_DECLARE_2OP_MODRM_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(uint32_t a, int64_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
};\
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RI)(uint32_t a, int64_t b); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b);


#ifdef OUTPUT32
#define X86_DECLARE_1OP_MODRM(name) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 8) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 16) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 32)
#define X86_DECLARE_2OP_MODRM(name) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 8) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 16) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 32)
#define X86_DECLARE_2OP_MODRM_IMM(name) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 8) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 16) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 32)
#else
#define X86_DECLARE_1OP_MODRM(name) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 8) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 16) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 32) \
	X86_DECLARE_1OP_MODRM_SIZE(name, 64)
#define X86_DECLARE_2OP_MODRM(name) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 8) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 16) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 32) \
	X86_DECLARE_2OP_MODRM_SIZE(name, 64)
#define X86_DECLARE_2OP_MODRM_IMM(name) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 8) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 16) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 32) \
	X86_DECLARE_2OP_MODRM_IMM_SIZE(name, 64)
#endif


X86_DECLARE_NO_OPERANDS(cld)
X86_DECLARE_NO_OPERANDS(std)
X86_DECLARE_NO_OPERANDS(nop)
X86_DECLARE_NO_OPERANDS(int3)

X86_DECLARE_1OP_MODRM(dec)
X86_DECLARE_1OP_MODRM(inc)
X86_DECLARE_1OP_MODRM(neg)
X86_DECLARE_1OP_MODRM(not)
X86_DECLARE_1OP_MODRM_NATIVE(pop)
X86_DECLARE_1OP_MODRM_NATIVE(push)

X86_DECLARE_2OP_RM_NATIVE(lea)

X86_DECLARE_2OP_MODRM_IMM(adc)
X86_DECLARE_2OP_MODRM_IMM(add)
X86_DECLARE_2OP_MODRM_IMM(and)
X86_DECLARE_2OP_MODRM_IMM(cmp)
X86_DECLARE_2OP_MODRM_IMM(sbb)
X86_DECLARE_2OP_MODRM_IMM(sub)
X86_DECLARE_2OP_MODRM_IMM(test)
X86_DECLARE_2OP_MODRM_IMM(or)
X86_DECLARE_2OP_MODRM(xchg)
X86_DECLARE_2OP_MODRM_IMM(xor)


#endif

