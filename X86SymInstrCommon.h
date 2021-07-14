#ifdef X86_SYMINSTR_PREFIX

#include "SymInstr.h"
#include "Output.h"
#include "asmx86.h"

enum X86RegisterClass
{
	X86REGCLASS_INTEGER,
	X86REGCLASS_INTEGER_8BIT,
	X86REGCLASS_INTEGER_INDEX,
	X86REGCLASS_INTEGER_NO_REX,
	X86REGCLASS_FLOAT,
	X86REGCLASS_INTEGER_RETURN_VALUE,
	X86REGCLASS_INTEGER_RETURN_VALUE_HIGH,
	X86REGCLASS_INTEGER_PARAM_0,
	X86REGCLASS_INTEGER_PARAM_1,
	X86REGCLASS_INTEGER_PARAM_2,
	X86REGCLASS_INTEGER_PARAM_3,
	X86REGCLASS_INTEGER_PARAM_4,
	X86REGCLASS_INTEGER_PARAM_5,
	X86REGCLASS_EAX,
	X86REGCLASS_ECX,
	X86REGCLASS_EDX,
	X86REGCLASS_ESI,
	X86REGCLASS_EDI,
	X86REGCLASS_ESP,
	X86REGCLASS_R8,
	X86REGCLASS_R9,
	X86REGCLASS_FLOAT_RETURN_VALUE,
	X86REGCLASS_FLOAT_PARAM_0,
	X86REGCLASS_FLOAT_PARAM_1,
	X86REGCLASS_FLOAT_PARAM_2,
	X86REGCLASS_FLOAT_PARAM_3,
	X86REGCLASS_FLOAT_PARAM_4,
	X86REGCLASS_FLOAT_PARAM_5,
	X86REGCLASS_SYSCALL_PARAM_0,
	X86REGCLASS_SYSCALL_PARAM_1,
	X86REGCLASS_SYSCALL_PARAM_2,
	X86REGCLASS_SYSCALL_PARAM_3,
	X86REGCLASS_SYSCALL_PARAM_4,
	X86REGCLASS_SYSCALL_PARAM_5,
	X86REGCLASS_SYSCALL_PARAM_6,
	X86REGCLASS_SYSCALL_RESULT_1,
	X86REGCLASS_SYSCALL_RESULT_2
};

// Don't try to combine these using X86_SYMINSTR_PREFIX, there be dragons here
#ifdef OUTPUT32
#define X86_SYMINSTR_NAME(name) X86Sym_ ## name
#define X86_SYMINSTR_NAME_SIZE(name, size) X86Sym_ ## name ## _ ## size
#define X86_SYMINSTR_NAME_OP(name, op) X86Sym_ ## name ## _ ## op
#define X86_SYMINSTR_NAME_SIZE_OP(name, size, op) X86Sym_ ## name ## _ ## size ## _ ## op
#define X86_SYMINSTR_NAME_2SIZE_OP(name, destSize, srcSize, op) X86Sym_ ## name ## _ ## destSize ## _ ## srcSize ## _ ## op
#define X86_SYMINSTR_CLASS(name) X86Sym_ ## name ## _Instr
#define X86_SYMINSTR_CLASS_SIZE(name, size) X86Sym_ ## name ## _ ## size ## _Instr
#define X86_SYMINSTR_CLASS_OP(name, op) X86Sym_ ## name ## _ ## op ## _Instr
#define X86_SYMINSTR_CLASS_SIZE_OP(name, size, op) X86Sym_ ## name ## _ ## size ## _ ## op ## _Instr
#define X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, op) X86Sym_ ## name ## _ ## destSize ## _ ## srcSize ## _ ## op ## _Instr
#else
#define X86_SYMINSTR_NAME(name) X64Sym_ ## name
#define X86_SYMINSTR_NAME_SIZE(name, size) X64Sym_ ## name ## _ ## size
#define X86_SYMINSTR_NAME_OP(name, op) X64Sym_ ## name ## _ ## op
#define X86_SYMINSTR_NAME_SIZE_OP(name, size, op) X64Sym_ ## name ## _ ## size ## _ ## op
#define X86_SYMINSTR_NAME_2SIZE_OP(name, destSize, srcSize, op) X64Sym_ ## name ## _ ## destSize ## _ ## srcSize ## _ ## op
#define X86_SYMINSTR_CLASS(name) X64Sym_ ## name ## _Instr
#define X86_SYMINSTR_CLASS_SIZE(name, size) X64Sym_ ## name ## _ ## size ## _Instr
#define X86_SYMINSTR_CLASS_OP(name, op) X64Sym_ ## name ## _ ## op ## _Instr
#define X86_SYMINSTR_CLASS_SIZE_OP(name, size, op) X64Sym_ ## name ## _ ## size ## _ ## op ## _Instr
#define X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, op) X64Sym_ ## name ## _ ## destSize ## _ ## srcSize ## _ ## op ## _Instr
#endif

#define X86_MEM_OP_PARAM uint32_t base, uint32_t index, uint32_t scale, uint32_t stackVar, int64_t offset
#define X86_MEM_OP_PASS base, index, scale, stackVar, offset

#define X86_SYM_MEM(base, offset) base, SYMREG_NONE, 1, SYMREG_NONE, offset
#define X86_SYM_MEM_INDEX(base, index, scale, stackVar, offset) base, index, scale, stackVar, offset


class X86_SYMINSTR_NAME(Instr): public SymInstr
{
protected:
	asmx86::OperandType GetOperandOfSize(uint32_t reg, uint32_t size);

	static void LeaOverflowHandler(OutputBlock* out, Relocation& reloc);
	static void BaseRelativeLeaOverflowHandler(OutputBlock* out, Relocation& reloc);
	static void ConditionalJumpOverflowHandler(OutputBlock* out, Relocation& reloc);
	static void UnconditionalJumpOverflowHandler(OutputBlock* out, Relocation& reloc);

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


#define X86_DECLARE_NO_OPERANDS_OR_IMM(name) \
class X86_SYMINSTR_CLASS(name): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS(name)(); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_OP(name, I): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, I)(int64_t a); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME(name)(); \
SymInstr* X86_SYMINSTR_NAME_OP(name, I)(int64_t a);


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


#define X86_DECLARE_1OP_MODRM_NATIVE_IMM(name) \
X86_DECLARE_1OP_MODRM_NATIVE(name) \
class X86_SYMINSTR_CLASS_OP(name, I): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, I)(int64_t a); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_OP(name, I)(int64_t a); \


#define X86_DECLARE_1OP_R_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, R): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(uint32_t a); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, R)(uint32_t a);


#define X86_DECLARE_1OP_M_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, M): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM);


#define X86_DECLARE_1OP_MODRM_SIZE(name, size) \
X86_DECLARE_1OP_R_SIZE(name, size) \
X86_DECLARE_1OP_M_SIZE(name, size)


#define X86_DECLARE_SETCC(name) \
class X86_SYMINSTR_CLASS_OP(name, R): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, R)(uint32_t a); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_OP(name, R)(uint32_t a);


#define X86_DECLARE_2OP_RR_NATIVE(name) \
class X86_SYMINSTR_CLASS_OP(name, RR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, RR)(uint32_t a, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_OP(name, RR)(uint32_t a, uint32_t b);


#define X86_DECLARE_2OP_II_NATIVE(name) \
class X86_SYMINSTR_CLASS_OP(name, II): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_OP(name, II)(int64_t a, int64_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_OP(name, II)(int64_t a, int64_t b);


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


#define X86_DECLARE_2OP_IMM_SIZE(name, size) \
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


#define X86_DECLARE_2OP_MODRM_IMM_SIZE(name, size) \
X86_DECLARE_2OP_MODRM_SIZE(name, size) \
X86_DECLARE_2OP_IMM_SIZE(name, size)


#define X86_DECLARE_MOV_SIZE(name, size) \
X86_DECLARE_2OP_RM_SIZE(name, size) \
X86_DECLARE_2OP_MR_SIZE(name, size) \
X86_DECLARE_2OP_IMM_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name##_fs, size, RM): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name##_fs, size, RM)(uint32_t a, X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name##_fs, size, MR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name##_fs, size, MR)(X86_MEM_OP_PARAM, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name##_gs, size, RM): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name##_gs, size, RM)(uint32_t a, X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name##_gs, size, MR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name##_gs, size, MR)(X86_MEM_OP_PARAM, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name##_fs, size, RM)(uint32_t a, X86_MEM_OP_PARAM); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name##_fs, size, MR)(X86_MEM_OP_PARAM, uint32_t b); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name##_gs, size, RM)(uint32_t a, X86_MEM_OP_PARAM); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name##_gs, size, MR)(X86_MEM_OP_PARAM, uint32_t b);


#define X86_DECLARE_3OP_RRI_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c);


#define X86_DECLARE_3OP_RMI_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RMI)(uint32_t a, X86_MEM_OP_PARAM, int64_t c); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RMI)(uint32_t a, X86_MEM_OP_PARAM, int64_t c);


#define X86_DECLARE_SHIFT_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RI)(uint32_t a, int64_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b); \
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
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RR)(uint32_t a, uint32_t b); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RI)(uint32_t a, int64_t b); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MR)(X86_MEM_OP_PARAM, uint32_t b); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MI)(X86_MEM_OP_PARAM, int64_t b);


#define X86_DECLARE_MOVEXT_SIZE(name, destSize, srcSize) \
class X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RR)(uint32_t a, uint32_t b); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_2SIZE_OP(name, destSize, srcSize, RM)(uint32_t a, X86_MEM_OP_PARAM); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_2SIZE_OP(name, destSize, srcSize, RR)(uint32_t a, uint32_t b); \
SymInstr* X86_SYMINSTR_NAME_2SIZE_OP(name, destSize, srcSize, RM)(uint32_t a, X86_MEM_OP_PARAM);


#define X86_DECLARE_MULDIV8(name) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, 8, R)(uint32_t a, uint32_t eax); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, 8, M)(X86_MEM_OP_PARAM, uint32_t eax); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, 8, R)(uint32_t a, uint32_t eax); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, 8, M)(X86_MEM_OP_PARAM, uint32_t eax);


#define X86_DECLARE_MULDIV_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, R): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, R)(uint32_t a, uint32_t eax, uint32_t edx); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, M): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM, uint32_t eax, uint32_t edx); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, R)(uint32_t a, uint32_t eax, uint32_t edx); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, M)(X86_MEM_OP_PARAM, uint32_t eax, uint32_t edx);


#define X86_DECLARE_SHIFT_DOUBLE_SIZE(name, size) \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRR)(uint32_t a, uint32_t b, uint32_t c); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRR)(X86_MEM_OP_PARAM, uint32_t b, uint32_t c); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
class X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI): public X86_SYMINSTR_NAME(Instr) \
{ \
public: \
	X86_SYMINSTR_CLASS_SIZE_OP(name, size, MRI)(X86_MEM_OP_PARAM, uint32_t b, int64_t c); \
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out); \
	virtual void Print(SymInstrFunction* func); \
}; \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RRR)(uint32_t a, uint32_t b, uint32_t c); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MRR)(X86_MEM_OP_PARAM, uint32_t b, uint32_t c); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, RRI)(uint32_t a, uint32_t b, int64_t c); \
SymInstr* X86_SYMINSTR_NAME_SIZE_OP(name, size, MRI)(X86_MEM_OP_PARAM, uint32_t b, int64_t c);


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
#define X86_DECLARE_MOV(name) \
	X86_DECLARE_MOV_SIZE(name, 8) \
	X86_DECLARE_MOV_SIZE(name, 16) \
	X86_DECLARE_MOV_SIZE(name, 32)
#define X86_DECLARE_CMOV(name) \
	X86_DECLARE_2OP_RR_SIZE(name, 16) \
	X86_DECLARE_2OP_RM_SIZE(name, 16) \
	X86_DECLARE_2OP_RR_SIZE(name, 32) \
	X86_DECLARE_2OP_RM_SIZE(name, 32)
#define X86_DECLARE_SHIFT(name) \
	X86_DECLARE_SHIFT_SIZE(name, 8) \
	X86_DECLARE_SHIFT_SIZE(name, 16) \
	X86_DECLARE_SHIFT_SIZE(name, 32)
#define X86_DECLARE_MOVEXT(name) \
	X86_DECLARE_MOVEXT_SIZE(name, 16, 8) \
	X86_DECLARE_MOVEXT_SIZE(name, 32, 8) \
	X86_DECLARE_MOVEXT_SIZE(name, 32, 16)
#define X86_DECLARE_MULDIV(name) \
	X86_DECLARE_MULDIV8(name) \
	X86_DECLARE_MULDIV_SIZE(name, 16) \
	X86_DECLARE_MULDIV_SIZE(name, 32)
#define X86_DECLARE_SHIFT_DOUBLE(name) \
	X86_DECLARE_SHIFT_DOUBLE_SIZE(name, 16) \
	X86_DECLARE_SHIFT_DOUBLE_SIZE(name, 32)
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
#define X86_DECLARE_MOV(name) \
	X86_DECLARE_MOV_SIZE(name, 8) \
	X86_DECLARE_MOV_SIZE(name, 16) \
	X86_DECLARE_MOV_SIZE(name, 32) \
	X86_DECLARE_MOV_SIZE(name, 64)
#define X86_DECLARE_CMOV(name) \
	X86_DECLARE_2OP_RR_SIZE(name, 16) \
	X86_DECLARE_2OP_RM_SIZE(name, 16) \
	X86_DECLARE_2OP_RR_SIZE(name, 32) \
	X86_DECLARE_2OP_RM_SIZE(name, 32) \
	X86_DECLARE_2OP_RR_SIZE(name, 64) \
	X86_DECLARE_2OP_RM_SIZE(name, 64)
#define X86_DECLARE_SHIFT(name) \
	X86_DECLARE_SHIFT_SIZE(name, 8) \
	X86_DECLARE_SHIFT_SIZE(name, 16) \
	X86_DECLARE_SHIFT_SIZE(name, 32) \
	X86_DECLARE_SHIFT_SIZE(name, 64)
#define X86_DECLARE_MOVEXT(name) \
	X86_DECLARE_MOVEXT_SIZE(name, 16, 8) \
	X86_DECLARE_MOVEXT_SIZE(name, 32, 8) \
	X86_DECLARE_MOVEXT_SIZE(name, 32, 16) \
	X86_DECLARE_MOVEXT_SIZE(name, 64, 8) \
	X86_DECLARE_MOVEXT_SIZE(name, 64, 16)
#define X86_DECLARE_MULDIV(name) \
	X86_DECLARE_MULDIV8(name) \
	X86_DECLARE_MULDIV_SIZE(name, 16) \
	X86_DECLARE_MULDIV_SIZE(name, 32) \
	X86_DECLARE_MULDIV_SIZE(name, 64)
#define X86_DECLARE_SHIFT_DOUBLE(name) \
	X86_DECLARE_SHIFT_DOUBLE_SIZE(name, 16) \
	X86_DECLARE_SHIFT_DOUBLE_SIZE(name, 32) \
	X86_DECLARE_SHIFT_DOUBLE_SIZE(name, 64)
#endif


X86_DECLARE_NO_OPERANDS(cld)
X86_DECLARE_NO_OPERANDS(std)
X86_DECLARE_NO_OPERANDS(nop)
X86_DECLARE_NO_OPERANDS(int3)
X86_DECLARE_NO_OPERANDS_OR_IMM(retn)
X86_DECLARE_NO_OPERANDS(leave)

X86_DECLARE_1OP_MODRM(dec)
X86_DECLARE_1OP_MODRM(inc)
X86_DECLARE_1OP_MODRM(neg)
X86_DECLARE_1OP_MODRM(not)
X86_DECLARE_1OP_MODRM_NATIVE(pop)
X86_DECLARE_1OP_MODRM_NATIVE_IMM(push)
X86_DECLARE_1OP_MODRM_NATIVE(jmpn)

X86_DECLARE_1OP_R_SIZE(bswap, 32)
#ifdef OUTPUT64
X86_DECLARE_1OP_R_SIZE(bswap, 64)
#endif

X86_DECLARE_SETCC(seto)
X86_DECLARE_SETCC(setno)
X86_DECLARE_SETCC(setb)
X86_DECLARE_SETCC(setae)
X86_DECLARE_SETCC(setz)
X86_DECLARE_SETCC(setnz)
X86_DECLARE_SETCC(setbe)
X86_DECLARE_SETCC(seta)
X86_DECLARE_SETCC(sets)
X86_DECLARE_SETCC(setns)
X86_DECLARE_SETCC(setp)
X86_DECLARE_SETCC(setnp)
X86_DECLARE_SETCC(setl)
X86_DECLARE_SETCC(setge)
X86_DECLARE_SETCC(setle)
X86_DECLARE_SETCC(setg)

X86_DECLARE_CMOV(cmovo)
X86_DECLARE_CMOV(cmovno)
X86_DECLARE_CMOV(cmovb)
X86_DECLARE_CMOV(cmovae)
X86_DECLARE_CMOV(cmovz)
X86_DECLARE_CMOV(cmovnz)
X86_DECLARE_CMOV(cmovbe)
X86_DECLARE_CMOV(cmova)
X86_DECLARE_CMOV(cmovs)
X86_DECLARE_CMOV(cmovns)
X86_DECLARE_CMOV(cmovp)
X86_DECLARE_CMOV(cmovnp)
X86_DECLARE_CMOV(cmovl)
X86_DECLARE_CMOV(cmovge)
X86_DECLARE_CMOV(cmovle)
X86_DECLARE_CMOV(cmovg)

X86_DECLARE_2OP_II_NATIVE(enter)

X86_DECLARE_2OP_MODRM_IMM(adc)
X86_DECLARE_2OP_MODRM_IMM(add)
X86_DECLARE_2OP_MODRM_IMM(and)
X86_DECLARE_2OP_MODRM_IMM(cmp)
X86_DECLARE_MOV(mov)
X86_DECLARE_2OP_MODRM_IMM(sbb)
X86_DECLARE_2OP_MODRM_IMM(sub)
X86_DECLARE_2OP_MODRM_IMM(test)
X86_DECLARE_2OP_MODRM_IMM(or)
X86_DECLARE_2OP_MODRM(xchg)
X86_DECLARE_2OP_MODRM_IMM(xor)

X86_DECLARE_2OP_RR_SIZE(rdtsc, 32)

X86_DECLARE_MULDIV(div)
X86_DECLARE_MULDIV(mul)
X86_DECLARE_MULDIV(idiv)
X86_DECLARE_MULDIV(imul)
X86_DECLARE_2OP_RR_SIZE(imul, 16)
X86_DECLARE_2OP_RM_SIZE(imul, 16)
X86_DECLARE_3OP_RRI_SIZE(imul, 16)
X86_DECLARE_3OP_RMI_SIZE(imul, 16)
X86_DECLARE_2OP_RR_SIZE(imul, 32)
X86_DECLARE_2OP_RM_SIZE(imul, 32)
X86_DECLARE_3OP_RRI_SIZE(imul, 32)
X86_DECLARE_3OP_RMI_SIZE(imul, 32)
#ifdef OUTPUT64
X86_DECLARE_2OP_RR_SIZE(imul, 64)
X86_DECLARE_2OP_RM_SIZE(imul, 64)
X86_DECLARE_3OP_RRI_SIZE(imul, 64)
X86_DECLARE_3OP_RMI_SIZE(imul, 64)
#endif

X86_DECLARE_SHIFT(shl)
X86_DECLARE_SHIFT(shr)
X86_DECLARE_SHIFT(sar)
X86_DECLARE_SHIFT(rol)
X86_DECLARE_SHIFT(ror)

X86_DECLARE_SHIFT_DOUBLE(shld)
X86_DECLARE_SHIFT_DOUBLE(shrd)

X86_DECLARE_MOVEXT(movsx)
X86_DECLARE_MOVEXT(movzx)
#ifdef OUTPUT64
X86_DECLARE_MOVEXT_SIZE(movsxd, 64, 32)
#endif


class X86_SYMINSTR_CLASS_OP(lea, RM): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS_OP(lea, RM)(uint32_t a, X86_MEM_OP_PARAM);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
};


class X86_SYMINSTR_CLASS(cwd): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(cwd)(uint32_t eax, uint32_t edx);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(cdq): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(cdq)(uint32_t eax, uint32_t edx);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


#ifdef OUTPUT64
class X86_SYMINSTR_CLASS(cqo): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(cqo)(uint32_t eax, uint32_t edx);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};
#endif


class X86_SYMINSTR_CLASS(rep_movsb): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(rep_movsb)(uint32_t edi, uint32_t esi, uint32_t ecx);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(rep_stosb): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(rep_stosb)(uint32_t edi, uint32_t eax, uint32_t ecx);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(repne_scasb): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(repne_scasb)(uint32_t edi, uint32_t eax, uint32_t ecx);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(CallPopDataAddr): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(CallPopDataAddr)(uint32_t dest, int64_t offset);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(CallPopCodeAddr): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(CallPopCodeAddr)(uint32_t dest, Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(FstenvDataAddr): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(FstenvDataAddr)(uint32_t dest, int64_t offset);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(FstenvCodeAddr): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(FstenvCodeAddr)(uint32_t dest, Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(MovDataPtrBaseRelative): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(MovDataPtrBaseRelative)(uint32_t dest, uint32_t base, int64_t offset);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(MovCodePtrBaseRelative): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(MovCodePtrBaseRelative)(uint32_t dest, uint32_t base, Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(MovDataPtrAbsolute): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(MovDataPtrAbsolute)(uint32_t dest, int64_t offset);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(MovCodePtrAbsolute): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(MovCodePtrAbsolute)(uint32_t dest, Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(CondJump): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(CondJump)(uint8_t type, Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(JumpRelative): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(JumpRelative)(Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(CallDirect): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(CallDirect)(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh, uint32_t key,
		uint32_t scratch, const std::vector<uint32_t>& reads);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(CallIndirectReg): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(CallIndirectReg)(uint32_t func, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch,
		const std::vector<uint32_t>& reads);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(CallIndirectMem): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(CallIndirectMem)(X86_MEM_OP_PARAM, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch,
		const std::vector<uint32_t>& reads);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(Syscall): public X86_SYMINSTR_NAME(Instr)
{
	std::vector<uint32_t> m_spilledRegs;

public:
	X86_SYMINSTR_CLASS(Syscall)(uint32_t eax, uint32_t edx, uint32_t ecx, const std::vector<uint32_t> readRegs,
		const std::vector<uint32_t> spilledRegs);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(SyscallInt80): public X86_SYMINSTR_NAME(Instr)
{
	std::vector<uint32_t> m_spilledRegs;

public:
	X86_SYMINSTR_CLASS(SyscallInt80)(uint32_t eax, uint32_t edx, uint32_t ecx, const std::vector<uint32_t> readRegs,
		const std::vector<uint32_t> spilledRegs);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(SyscallCorrectErrorCode): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(SyscallCorrectErrorCode)(uint32_t eax);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_CLASS(RegParam): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(RegParam)(const std::vector<uint32_t>& regs);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
};


class X86_SYMINSTR_CLASS(SymReturn): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(SymReturn)(uint32_t a, uint32_t b);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
};


class X86_SYMINSTR_CLASS(SaveCalleeSavedRegs): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(SaveCalleeSavedRegs)();
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
};


class X86_SYMINSTR_CLASS(RestoreCalleeSavedRegs): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(RestoreCalleeSavedRegs)();
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
};


class X86_SYMINSTR_CLASS(AntiDisassembly): public X86_SYMINSTR_NAME(Instr)
{
public:
	X86_SYMINSTR_CLASS(AntiDisassembly)(uint32_t reg);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class X86_SYMINSTR_NAME(Function): public SymInstrFunction
{
public:
	X86_SYMINSTR_NAME(Function)(const Settings& settings, Function* func);
	virtual std::vector<uint32_t> GetCallerSavedRegisters();
	virtual std::vector<uint32_t> GetCalleeSavedRegisters();
	virtual std::set<uint32_t> GetRegisterClassInterferences(uint32_t cls);
	virtual bool IsRegisterClassFixed(uint32_t cls);
	virtual uint32_t GetFixedRegisterForClass(uint32_t cls);
	virtual uint32_t GetSpecialRegisterAssignment(uint32_t reg);
	virtual bool DoesRegisterClassConflictWithSpecialRegisters(uint32_t cls);
	virtual size_t GetNativeSize();
	virtual void LayoutStackFrame();
	virtual bool GenerateSpillLoad(uint32_t reg, uint32_t var, int64_t offset, ILParameterType type, std::vector<SymInstr*>& code);
	virtual bool GenerateSpillStore(uint32_t reg, uint32_t var, int64_t offset, ILParameterType type, std::vector<SymInstr*>& code);
	virtual void PrintRegisterClass(uint32_t cls);
	virtual void PrintRegister(uint32_t reg);
};


SymInstr* X86_SYMINSTR_NAME_OP(lea, RM)(uint32_t a, X86_MEM_OP_PARAM);
SymInstr* X86_SYMINSTR_NAME(cwd)(uint32_t eax, uint32_t edx);
SymInstr* X86_SYMINSTR_NAME(cdq)(uint32_t eax, uint32_t edx);
#ifdef OUTPUT64
SymInstr* X86_SYMINSTR_NAME(cqo)(uint32_t eax, uint32_t edx);
#endif
SymInstr* X86_SYMINSTR_NAME(rep_movsb)(uint32_t edi, uint32_t esi, uint32_t ecx);
SymInstr* X86_SYMINSTR_NAME(rep_stosb)(uint32_t edi, uint32_t eax, uint32_t ecx);
SymInstr* X86_SYMINSTR_NAME(repne_scasb)(uint32_t edi, uint32_t eax, uint32_t ecx);

SymInstr* X86_SYMINSTR_NAME(CallPopDataAddr)(uint32_t dest, int64_t offset);
SymInstr* X86_SYMINSTR_NAME(CallPopCodeAddr)(uint32_t dest, Function* func, ILBlock* block);
SymInstr* X86_SYMINSTR_NAME(FstenvDataAddr)(uint32_t dest, int64_t offset);
SymInstr* X86_SYMINSTR_NAME(FstenvCodeAddr)(uint32_t dest, Function* func, ILBlock* block);
SymInstr* X86_SYMINSTR_NAME(MovDataPtrBaseRelative)(uint32_t dest, uint32_t base, int64_t offset);
SymInstr* X86_SYMINSTR_NAME(MovCodePtrBaseRelative)(uint32_t dest, uint32_t base, Function* func, ILBlock* block);
SymInstr* X86_SYMINSTR_NAME(MovDataPtrAbsolute)(uint32_t dest, int64_t offset);
SymInstr* X86_SYMINSTR_NAME(MovCodePtrAbsolute)(uint32_t dest, Function* func, ILBlock* block);
SymInstr* X86_SYMINSTR_NAME(CondJump)(uint8_t type, Function* func, ILBlock* block);
SymInstr* X86_SYMINSTR_NAME(JumpRelative)(Function* func, ILBlock* block);
SymInstr* X86_SYMINSTR_NAME(CallDirect)(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch, const std::vector<uint32_t>& reads);
SymInstr* X86_SYMINSTR_NAME(CallIndirectReg)(uint32_t func, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch, const std::vector<uint32_t>& reads);
SymInstr* X86_SYMINSTR_NAME(CallIndirectMem)(X86_MEM_OP_PARAM, uint32_t retVal, uint32_t retValHigh, uint32_t key, uint32_t scratch, const std::vector<uint32_t>& reads);
SymInstr* X86_SYMINSTR_NAME(Syscall)(uint32_t eax, uint32_t edx, uint32_t ecx, const std::vector<uint32_t>& readRegs,
	const std::vector<uint32_t>& spilledRegs);
SymInstr* X86_SYMINSTR_NAME(SyscallInt80)(uint32_t eax, uint32_t edx, uint32_t ecx, const std::vector<uint32_t>& readRegs,
	const std::vector<uint32_t>& spilledRegs);
SymInstr* X86_SYMINSTR_NAME(SyscallCorrectErrorCode)(uint32_t eax);
SymInstr* X86_SYMINSTR_NAME(RegParam)(const std::vector<uint32_t>& regs);
SymInstr* X86_SYMINSTR_NAME(SymReturn)(uint32_t a, uint32_t b);
SymInstr* X86_SYMINSTR_NAME(SaveCalleeSavedRegs)();
SymInstr* X86_SYMINSTR_NAME(RestoreCalleeSavedRegs)();
SymInstr* X86_SYMINSTR_NAME(AntiDisassembly)(uint32_t reg);


#endif

