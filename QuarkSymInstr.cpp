#include "QuarkSymInstr.h"
#include "Struct.h"
#include "asmquark.h"


QuarkSymInstr::QuarkSymInstr()
{
}


Quark1OpInstr::Quark1OpInstr(uint32_t op, uint32_t a, uint32_t shift)
{
	SetOperation(op);
	AddReadRegisterOperand(a);
	AddImmediateOperand(shift);
}


Quark1OpInstr::Quark1OpInstr(uint32_t op, int32_t immed)
{
	SetOperation(op);
	AddImmediateOperand(immed);
}


Quark1OpReadRegInstr::Quark1OpReadRegInstr(uint32_t op, uint32_t a)
{
	SetOperation(op);
	AddReadRegisterOperand(a);
}


Quark1OpWriteRegInstr::Quark1OpWriteRegInstr(uint32_t op, uint32_t a)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
}


Quark2OpInstr::Quark2OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t shift)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddImmediateOperand(shift);
}


Quark2OpInstr::Quark2OpInstr(uint32_t op, uint32_t a, int32_t immed)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddImmediateOperand(immed);
}


Quark2OpRegInstr::Quark2OpRegInstr(uint32_t op, uint32_t a, uint32_t b)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
}


Quark3OpInstr::Quark3OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


Quark3OpInstr::Quark3OpInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddImmediateOperand(immed);
}


Quark3OpExInstr::Quark3OpExInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


Quark3OpExInstr::Quark3OpExInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddImmediateOperand(immed);
}


QuarkFloat3OpInstr::QuarkFloat3OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddReadRegisterOperand(c);
}


Quark4OpInstr::Quark4OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	SetOperation(op);
	AddWriteRegisterOperand(a);
	AddWriteRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddReadRegisterOperand(d);
}


QuarkLoadInstr::QuarkLoadInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


QuarkLoadInstr::QuarkLoadInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddImmediateOperand(immed);
}


QuarkStackLoadInstr::QuarkStackLoadInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddStackVarOperand(var, offset);
}


QuarkGlobalLoadInstr::QuarkGlobalLoadInstr(uint32_t op, uint32_t a, uint32_t b, int64_t offset)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddGlobalVarOperand(offset);
}


QuarkLoadUpdateInstr::QuarkLoadUpdateInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadWriteRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


QuarkLoadUpdateInstr::QuarkLoadUpdateInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadWriteRegisterOperand(b);
	AddImmediateOperand(immed);
}


QuarkLoadImmInstr::QuarkLoadImmInstr(uint32_t a, int32_t immed)
{
	SetOperation(0x14);
	AddWriteRegisterOperand(a);
	AddImmediateOperand(immed);
}


QuarkLoadImmHighInstr::QuarkLoadImmHighInstr(uint32_t a, int32_t immed)
{
	SetOperation(0x15);
	AddReadWriteRegisterOperand(a);
	AddImmediateOperand(immed);
}


QuarkStoreInstr::QuarkStoreInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


QuarkStoreInstr::QuarkStoreInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddImmediateOperand(immed);
}


QuarkStackStoreInstr::QuarkStackStoreInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddStackVarOperand(var, offset);
}


QuarkGlobalStoreInstr::QuarkGlobalStoreInstr(uint32_t op, uint32_t a, uint32_t b, int64_t offset)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddGlobalVarOperand(offset);
}


QuarkStoreUpdateInstr::QuarkStoreUpdateInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadWriteRegisterOperand(b);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


QuarkStoreUpdateInstr::QuarkStoreUpdateInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadWriteRegisterOperand(b);
	AddImmediateOperand(immed);
}


QuarkBranchInstr::QuarkBranchInstr(uint32_t op, Function* func, ILBlock* block)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	AddBlockOperand(func, block);
}


QuarkCondJumpInstr::QuarkCondJumpInstr(uint32_t b, uint32_t val, Function* func, ILBlock* block)
{
	SetOperation(0x16);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	AddImmediateOperand(b);
	AddImmediateOperand(val);
	AddBlockOperand(func, block);
}


QuarkCallInstr::QuarkCallInstr(uint32_t a)
{
	SetOperation(0x1f06);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	AddReadRegisterOperand(a);
}


QuarkSyscallImmInstr::QuarkSyscallImmInstr(int32_t immed)
{
	SetOperation(0x2c);
	AddImmediateOperand(immed);
}


QuarkCmpInstr::QuarkCmpInstr(uint32_t op, uint32_t cmp, uint32_t b, uint32_t a, uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(cmp);
	AddImmediateOperand(b);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


QuarkCmpInstr::QuarkCmpInstr(uint32_t op, uint32_t cmp, uint32_t b, uint32_t a, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(cmp);
	AddImmediateOperand(b);
	AddReadRegisterOperand(a);
	AddImmediateOperand(immed);
}


QuarkCondCmpInstr::QuarkCondCmpInstr(uint32_t op, uint32_t cc, uint32_t val, uint32_t cmp, uint32_t b, uint32_t a,
	uint32_t c, uint32_t shift)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(cc);
	AddImmediateOperand(val);
	AddImmediateOperand(cmp);
	AddImmediateOperand(b);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(c);
	AddImmediateOperand(shift);
}


QuarkCondCmpInstr::QuarkCondCmpInstr(uint32_t op, uint32_t cc, uint32_t val, uint32_t cmp, uint32_t b, uint32_t a, int32_t immed)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(cc);
	AddImmediateOperand(val);
	AddImmediateOperand(cmp);
	AddImmediateOperand(b);
	AddReadRegisterOperand(a);
	AddImmediateOperand(immed);
}


QuarkXchgInstr::QuarkXchgInstr(uint32_t a, uint32_t b)
{
	SetOperation(0x1f01);
	AddReadWriteRegisterOperand(a);
	AddReadWriteRegisterOperand(b);
}


QuarkAddStackInstr::QuarkAddStackInstr(uint32_t a, uint32_t b, uint32_t var, int64_t offset)
{
	SetOperation(0x18);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddStackVarOperand(var, offset);
}


QuarkAddGlobalInstr::QuarkAddGlobalInstr(uint32_t a, uint32_t b, int64_t offset)
{
	SetOperation(0x18);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddGlobalVarOperand(offset);
}


QuarkCondBit1OpInstr::QuarkCondBit1OpInstr(uint32_t op, uint32_t a)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(a);
}


QuarkCondBit2OpInstr::QuarkCondBit2OpInstr(uint32_t op, uint32_t a, uint32_t b)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(a);
	AddImmediateOperand(b);
}


QuarkCondBit3OpInstr::QuarkCondBit3OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_USES_FLAGS);
	EnableFlag(SYMFLAG_WRITES_FLAGS);
	AddImmediateOperand(a);
	AddImmediateOperand(b);
	AddImmediateOperand(c);
}


QuarkBreakpointInstr::QuarkBreakpointInstr()
{
	SetOperation(0x1f1f);
}


QuarkSymInstrFunction::QuarkSymInstrFunction()
{
}


const char* QuarkSymInstr::GetOperationName() const
{
	switch (GetOperation())
	{
	case 0x00:   return "ldb";
	case 0x01:   return "ldh";
	case 0x02:   return "ldw";
	case 0x03:   return "ldmw";
	case 0x04:   return "stb";
	case 0x05:   return "sth";
	case 0x06:   return "stw";
	case 0x07:   return "stmw";
	case 0x08:   return "ldbu";
	case 0x09:   return "ldhu";
	case 0x0a:   return "ldwu";
	case 0x0b:   return "ldmwu";
	case 0x0c:   return "stbu";
	case 0x0d:   return "sthu";
	case 0x0e:   return "stwu";
	case 0x0f:   return "stmwu";
	case 0x10:   return "ldsxb";
	case 0x11:   return "ldsxh";
	case 0x12:   return "ldsxbu";
	case 0x13:   return "ldsxhu";
	case 0x14:   return "ldi";
	case 0x15:   return "ldih";
	case 0x16:   return "jmp";
	case 0x17:   return "call";
	case 0x18:   return "add";
	case 0x19:   return "sub";
	case 0x1a:   return "addx";
	case 0x1b:   return "subx";
	case 0x1c:   return "mulx";
	case 0x1d:   return "imulx";
	case 0x1e:   return "mul";
	case 0x20:   return "div";
	case 0x21:   return "idiv";
	case 0x22:   return "mod";
	case 0x23:   return "imod";
	case 0x24:   return "and";
	case 0x25:   return "or";
	case 0x26:   return "xor";
	case 0x27:   return "sar";
	case 0x28:   return "shl";
	case 0x29:   return "shr";
	case 0x2a:   return "rol";
	case 0x2b:   return "ror";
	case 0x2c:   return "syscall";
	case 0x2d:   return "cmp";
	case 0x2e:   return "icmp";
	case 0x2f:   return "fcmp";
	case 0x30:   return "ldfs";
	case 0x31:   return "ldfd";
	case 0x32:   return "stfs";
	case 0x33:   return "stfd";
	case 0x34:   return "ldfsu";
	case 0x35:   return "ldfdu";
	case 0x36:   return "stfsu";
	case 0x37:   return "stfdu";
	case 0x38:   return "fadd";
	case 0x39:   return "fsub";
	case 0x3a:   return "fmul";
	case 0x3b:   return "fdiv";
	case 0x3c:   return "fmod";
	case 0x3d:   return "fpow";
	case 0x3e:   return "flog";
	case 0x1f00: return "mov";
	case 0x1f01: return "xchg";
	case 0x1f02: return "sxb";
	case 0x1f03: return "sxh";
	case 0x1f04: return "swaph";
	case 0x1f05: return "swapw";
	case 0x1f06: return "call";
	case 0x1f08: return "neg";
	case 0x1f09: return "not";
	case 0x1f0c: return "ldsr";
	case 0x1f0d: return "stsr";
	case 0x1f0e: return "ldcr";
	case 0x1f0f: return "stcr";
	case 0x1f10: return "syscall";
	case 0x1f11: return "iret";
	case 0x1f12: return "invpg";
	case 0x1f13: return "invall";
	case 0x1f14: return "eoi";
	case 0x1f16: return "ldvec";
	case 0x1f17: return "stvec";
	case 0x1f18: return "setcc";
	case 0x1f19: return "clrcc";
	case 0x1f1a: return "notcc";
	case 0x1f1b: return "movcc";
	case 0x1f1c: return "andcc";
	case 0x1f1d: return "orcc";
	case 0x1f1e: return "xorcc";
	case 0x1f1f: return "bp";
	case 0x3f00: return "ldfi";
	case 0x3f01: return "stfi";
	case 0x3f02: return "fldpi";
	case 0x3f03: return "flde";
	case 0x3f04: return "fex";
	case 0x3f05: return "f2x";
	case 0x3f06: return "f10x";
	case 0x3f08: return "fsqrt";
	case 0x3f09: return "frcp";
	case 0x3f0a: return "frsqrt";
	case 0x3f0b: return "fneg";
	case 0x3f0c: return "fsin";
	case 0x3f0d: return "fcos";
	case 0x3f0e: return "ftan";
	case 0x3f0f: return "ffloor";
	case 0x3f10: return "fasin";
	case 0x3f11: return "facos";
	case 0x3f12: return "fatan";
	case 0x3f13: return "fceil";
	case 0x3f14: return "fsinh";
	case 0x3f15: return "fcosh";
	case 0x3f16: return "ftanh";
	case 0x3f17: return "fround";
	case 0x3f18: return "fasinh";
	case 0x3f19: return "facosh";
	case 0x3f1a: return "fatanh";
	case 0x3f1b: return "fabs";
	case 0x3f1c: return "fln";
	case 0x3f1d: return "flog2";
	case 0x3f1e: return "flog10";
	case 0x3f1f: return "fmov";
	default:
		return "invalid";
	}
}


void Quark1OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	if ((m_operands.size() > 1) && (m_operands[1].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[1].Print();
	}
}


void Quark1OpReadRegInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
}


void Quark1OpWriteRegInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
}


void Quark2OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	if ((m_operands.size() > 2) && (m_operands[2].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[2].Print();
	}
}


void Quark2OpRegInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
}


void Quark3OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print();
	}
}


void Quark3OpExInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print();
	}
}


void QuarkFloat3OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void Quark4OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	fprintf(stderr, ", ");
	m_operands[3].Print();
}


void QuarkLoadInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print();
	}
}


void QuarkStackLoadInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkGlobalLoadInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkLoadUpdateInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print();
	}
}


void QuarkLoadImmInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
}


void QuarkLoadImmHighInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
}


void QuarkStoreInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print();
	}
}


void QuarkStackStoreInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkGlobalStoreInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkStoreUpdateInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print();
	}
}


void QuarkBranchInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
}


void QuarkCondJumpInstr::Print()
{
	fprintf(stderr, "jmp.%d%s ", (int)m_operands[0].immed, m_operands[1].immed ? "t" : "f");
	m_operands[2].Print();
}


void QuarkCallInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
}


void QuarkSyscallImmInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
}


void QuarkCmpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());

	if (m_operands[0].immed < 6)
	{
		switch (m_operands[0].immed)
		{
		case 0: fprintf(stderr, "lt%d, ", (int)m_operands[1].immed); break;
		case 1: fprintf(stderr, "le%d, ", (int)m_operands[1].immed); break;
		case 2: fprintf(stderr, "ge%d, ", (int)m_operands[1].immed); break;
		case 3: fprintf(stderr, "gt%d, ", (int)m_operands[1].immed); break;
		case 4: fprintf(stderr, "eq%d, ", (int)m_operands[1].immed); break;
		case 5: fprintf(stderr, "ne%d, ", (int)m_operands[1].immed); break;
		default: break;
		}
	}
	else if (GetOperation() == 0x2f)
	{
		if (m_operands[0].immed == 6)
			fprintf(stderr, "nan%d, ", (int)m_operands[1].immed);
		else
			fprintf(stderr, "inf%d, ", (int)m_operands[1].immed);
	}
	else
	{
		if (m_operands[0].immed == 6)
			fprintf(stderr, "btnz%d, ", (int)m_operands[1].immed);
		else
			fprintf(stderr, "btz%d, ", (int)m_operands[1].immed);
	}

	m_operands[2].Print();
	fprintf(stderr, ", ");
	m_operands[3].Print();
	if ((m_operands.size() > 4) && (m_operands[4].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[4].Print();
	}
}


void QuarkCondCmpInstr::Print()
{
	fprintf(stderr, "%s.%d%s ", GetOperationName(), (int)m_operands[0].immed, m_operands[1].immed ? "t" : "f");

	if (m_operands[2].immed < 6)
	{
		switch (m_operands[2].immed)
		{
		case 0: fprintf(stderr, "lt%d, ", (int)m_operands[3].immed); break;
		case 1: fprintf(stderr, "le%d, ", (int)m_operands[3].immed); break;
		case 2: fprintf(stderr, "ge%d, ", (int)m_operands[3].immed); break;
		case 3: fprintf(stderr, "gt%d, ", (int)m_operands[3].immed); break;
		case 4: fprintf(stderr, "eq%d, ", (int)m_operands[3].immed); break;
		case 5: fprintf(stderr, "ne%d, ", (int)m_operands[3].immed); break;
		default: break;
		}
	}
	else if (GetOperation() == 0x2f)
	{
		if (m_operands[2].immed == 6)
			fprintf(stderr, "nan%d, ", (int)m_operands[3].immed);
		else
			fprintf(stderr, "inf%d, ", (int)m_operands[3].immed);
	}
	else
	{
		if (m_operands[2].immed == 6)
			fprintf(stderr, "btnz%d, ", (int)m_operands[3].immed);
		else
			fprintf(stderr, "btz%d, ", (int)m_operands[3].immed);
	}

	m_operands[4].Print();
	fprintf(stderr, ", ");
	m_operands[5].Print();
	if ((m_operands.size() > 6) && (m_operands[6].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[6].Print();
	}
}


void QuarkXchgInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
}


void QuarkAddStackInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkAddGlobalInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkCondBit1OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
}


void QuarkCondBit2OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
}


void QuarkCondBit3OpInstr::Print()
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print();
	fprintf(stderr, ", ");
	m_operands[1].Print();
	fprintf(stderr, ", ");
	m_operands[2].Print();
}


void QuarkBreakpointInstr::Print()
{
	fprintf(stderr, "bp");
}


void QuarkSymInstrFunction::PrintRegisterClass(uint32_t cls)
{
	switch (cls)
	{
	case QUARKREGCLASS_INTEGER:
		fprintf(stderr, "int");
		break;
	case QUARKREGCLASS_FLOAT:
		fprintf(stderr, "float");
		break;
	case QUARKREGCLASS_INTEGER_RETURN_VALUE:
		fprintf(stderr, "retval");
		break;
	case QUARKREGCLASS_INTEGER_RETURN_VALUE_HIGH:
		fprintf(stderr, "retvalhigh");
		break;
	case QUARKREGCLASS_INTEGER_PARAM_0:
	case QUARKREGCLASS_INTEGER_PARAM_1:
	case QUARKREGCLASS_INTEGER_PARAM_2:
	case QUARKREGCLASS_INTEGER_PARAM_3:
	case QUARKREGCLASS_INTEGER_PARAM_4:
	case QUARKREGCLASS_INTEGER_PARAM_5:
	case QUARKREGCLASS_INTEGER_PARAM_6:
	case QUARKREGCLASS_INTEGER_PARAM_7:
		fprintf(stderr, "intparam%d", cls - QUARKREGCLASS_INTEGER_PARAM_0);
		break;
	case QUARKREGCLASS_FLOAT_PARAM_0:
	case QUARKREGCLASS_FLOAT_PARAM_1:
	case QUARKREGCLASS_FLOAT_PARAM_2:
	case QUARKREGCLASS_FLOAT_PARAM_3:
	case QUARKREGCLASS_FLOAT_PARAM_4:
	case QUARKREGCLASS_FLOAT_PARAM_5:
	case QUARKREGCLASS_FLOAT_PARAM_6:
	case QUARKREGCLASS_FLOAT_PARAM_7:
		fprintf(stderr, "floatparam%d", cls - QUARKREGCLASS_FLOAT_PARAM_0);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_0:
	case QUARKREGCLASS_SYSCALL_PARAM_1:
	case QUARKREGCLASS_SYSCALL_PARAM_2:
	case QUARKREGCLASS_SYSCALL_PARAM_3:
	case QUARKREGCLASS_SYSCALL_PARAM_4:
	case QUARKREGCLASS_SYSCALL_PARAM_5:
	case QUARKREGCLASS_SYSCALL_PARAM_6:
	case QUARKREGCLASS_SYSCALL_PARAM_7:
		fprintf(stderr, "sysparam%d", cls - QUARKREGCLASS_SYSCALL_PARAM_0);
		break;
	case QUARKREGCLASS_SYSCALL_RESULT:
		fprintf(stderr, "sysresult");
		break;
	default:
		fprintf(stderr, "invalid");
		break;
	}
}


SymInstr* QuarkLoad8(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x00, a, b, c, s); }
SymInstr* QuarkLoad8(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x00, a, b, immed); }
SymInstr* QuarkLoad16(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x01, a, b, c, s); }
SymInstr* QuarkLoad16(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x01, a, b, immed); }
SymInstr* QuarkLoad32(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x02, a, b, c, s); }
SymInstr* QuarkLoad32(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x02, a, b, immed); }
SymInstr* QuarkLoadFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x30, a, b, c, s); }
SymInstr* QuarkLoadFS(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x30, a, b, immed); }
SymInstr* QuarkLoadFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x31, a, b, c, s); }
SymInstr* QuarkLoadFD(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x31, a, b, immed); }
SymInstr* QuarkLoadSX8(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x10, a, b, c, s); }
SymInstr* QuarkLoadSX8(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x10, a, b, immed); }
SymInstr* QuarkLoadSX16(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadInstr(0x11, a, b, c, s); }
SymInstr* QuarkLoadSX16(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadInstr(0x11, a, b, immed); }
SymInstr* QuarkLoadStack8(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x00, a, b, var, o); }
SymInstr* QuarkLoadStack16(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x01, a, b, var, o); }
SymInstr* QuarkLoadStack32(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x02, a, b, var, o); }
SymInstr* QuarkLoadStackFS(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x30, a, b, var, o); }
SymInstr* QuarkLoadStackFD(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x31, a, b, var, o); }
SymInstr* QuarkLoadStackSX8(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x10, a, b, var, o); }
SymInstr* QuarkLoadStackSX16(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackLoadInstr(0x11, a, b, var, o); }
SymInstr* QuarkLoadGlobal8(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x00, a, b, offset); }
SymInstr* QuarkLoadGlobal16(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x01, a, b, offset); }
SymInstr* QuarkLoadGlobal32(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x02, a, b, offset); }
SymInstr* QuarkLoadGlobalFS(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x30, a, b, offset); }
SymInstr* QuarkLoadGlobalFD(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x31, a, b, offset); }
SymInstr* QuarkLoadGlobalSX8(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x10, a, b, offset); }
SymInstr* QuarkLoadGlobalSX16(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalLoadInstr(0x11, a, b, offset); }
SymInstr* QuarkLoadUpdate8(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x08, a, b, c, s); }
SymInstr* QuarkLoadUpdate8(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x08, a, b, immed); }
SymInstr* QuarkLoadUpdate16(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x09, a, b, c, s); }
SymInstr* QuarkLoadUpdate16(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x09, a, b, immed); }
SymInstr* QuarkLoadUpdate32(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x0a, a, b, c, s); }
SymInstr* QuarkLoadUpdate32(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x0a, a, b, immed); }
SymInstr* QuarkLoadUpdateFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x34, a, b, c, s); }
SymInstr* QuarkLoadUpdateFS(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x34, a, b, immed); }
SymInstr* QuarkLoadUpdateFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x35, a, b, c, s); }
SymInstr* QuarkLoadUpdateFD(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x35, a, b, immed); }
SymInstr* QuarkLoadSXUpdate8(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x12, a, b, c, s); }
SymInstr* QuarkLoadSXUpdate8(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x12, a, b, immed); }
SymInstr* QuarkLoadSXUpdate16(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x13, a, b, c, s); }
SymInstr* QuarkLoadSXUpdate16(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x13, a, b, immed); }

SymInstr* QuarkStore8(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreInstr(0x04, a, b, c, s); }
SymInstr* QuarkStore8(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreInstr(0x04, a, b, immed); }
SymInstr* QuarkStore16(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreInstr(0x05, a, b, c, s); }
SymInstr* QuarkStore16(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreInstr(0x05, a, b, immed); }
SymInstr* QuarkStore32(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreInstr(0x06, a, b, c, s); }
SymInstr* QuarkStore32(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreInstr(0x06, a, b, immed); }
SymInstr* QuarkStoreFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreInstr(0x32, a, b, c, s); }
SymInstr* QuarkStoreFS(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreInstr(0x32, a, b, immed); }
SymInstr* QuarkStoreFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreInstr(0x33, a, b, c, s); }
SymInstr* QuarkStoreFD(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreInstr(0x33, a, b, immed); }
SymInstr* QuarkStoreStack8(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackStoreInstr(0x04, a, b, var, o); }
SymInstr* QuarkStoreStack16(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackStoreInstr(0x05, a, b, var, o); }
SymInstr* QuarkStoreStack32(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackStoreInstr(0x06, a, b, var, o); }
SymInstr* QuarkStoreStackFS(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackStoreInstr(0x32, a, b, var, o); }
SymInstr* QuarkStoreStackFD(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkStackStoreInstr(0x33, a, b, var, o); }
SymInstr* QuarkStoreGlobal8(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalStoreInstr(0x04, a, b, offset); }
SymInstr* QuarkStoreGlobal16(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalStoreInstr(0x05, a, b, offset); }
SymInstr* QuarkStoreGlobal32(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalStoreInstr(0x06, a, b, offset); }
SymInstr* QuarkStoreGlobalFS(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalStoreInstr(0x32, a, b, offset); }
SymInstr* QuarkStoreGlobalFD(uint32_t a, uint32_t b, int64_t offset) { return new QuarkGlobalStoreInstr(0x33, a, b, offset); }
SymInstr* QuarkStoreUpdate8(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreUpdateInstr(0x0c, a, b, c, s); }
SymInstr* QuarkStoreUpdate8(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreUpdateInstr(0x0c, a, b, immed); }
SymInstr* QuarkStoreUpdate16(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreUpdateInstr(0x0d, a, b, c, s); }
SymInstr* QuarkStoreUpdate16(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreUpdateInstr(0x0d, a, b, immed); }
SymInstr* QuarkStoreUpdate32(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreUpdateInstr(0x0e, a, b, c, s); }
SymInstr* QuarkStoreUpdate32(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreUpdateInstr(0x0e, a, b, immed); }
SymInstr* QuarkStoreUpdateFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreUpdateInstr(0x36, a, b, c, s); }
SymInstr* QuarkStoreUpdateFS(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreUpdateInstr(0x36, a, b, immed); }
SymInstr* QuarkStoreUpdateFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreUpdateInstr(0x37, a, b, c, s); }
SymInstr* QuarkStoreUpdateFD(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreUpdateInstr(0x37, a, b, immed); }

SymInstr* QuarkLoadImm(uint32_t a, int32_t immed) { return new QuarkLoadImmInstr(a, immed); }
SymInstr* QuarkLoadImmHigh(uint32_t a, int32_t immed) { return new QuarkLoadImmHighInstr(a, immed); }

SymInstr* QuarkJump(Function* func, ILBlock* block) { return new QuarkBranchInstr(0x16, func, block); }
SymInstr* QuarkCondJump(uint32_t b, uint32_t value, Function* func, ILBlock* block) { return new QuarkCondJumpInstr(b, value, func, block); }
SymInstr* QuarkCall(Function* func, ILBlock* block) { return new QuarkBranchInstr(0x17, func, block); }
SymInstr* QuarkCall(uint32_t reg) { return new QuarkCallInstr(reg); }
SymInstr* QuarkSyscallReg(uint32_t a) { return new Quark1OpReadRegInstr(0x1f10, a); }
SymInstr* QuarkSyscallImmed(int32_t immed) { return new QuarkSyscallImmInstr(immed); }

SymInstr* QuarkAdd(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x18, a, b, c, s); }
SymInstr* QuarkAdd(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x18, a, b, immed); }
SymInstr* QuarkAddStack(uint32_t a, uint32_t b, uint32_t var, int64_t o) { return new QuarkAddStackInstr(a, b, var, o); }
SymInstr* QuarkAddGlobal(uint32_t a, uint32_t b, int64_t offset) { return new QuarkAddGlobalInstr(a, b, offset); }
SymInstr* QuarkSub(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x19, a, b, c, s); }
SymInstr* QuarkSub(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x19, a, b, immed); }
SymInstr* QuarkAddx(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpExInstr(0x1a, a, b, c, s); }
SymInstr* QuarkAddx(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpExInstr(0x1a, a, b, immed); }
SymInstr* QuarkSubx(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpExInstr(0x1b, a, b, c, s); }
SymInstr* QuarkSubx(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpExInstr(0x1b, a, b, immed); }
SymInstr* QuarkMulx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) { return new Quark4OpInstr(0x1c, a, b, c, d); }
SymInstr* QuarkImulx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) { return new Quark4OpInstr(0x1d, a, b, c, d); }
SymInstr* QuarkMul(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x1e, a, b, c, s); }
SymInstr* QuarkMul(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x1e, a, b, immed); }
SymInstr* QuarkDiv(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x20, a, b, c, s); }
SymInstr* QuarkDiv(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x20, a, b, immed); }
SymInstr* QuarkIdiv(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x21, a, b, c, s); }
SymInstr* QuarkIdiv(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x21, a, b, immed); }
SymInstr* QuarkMod(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x22, a, b, c, s); }
SymInstr* QuarkMod(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x22, a, b, immed); }
SymInstr* QuarkImod(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x23, a, b, c, s); }
SymInstr* QuarkImod(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x23, a, b, immed); }
SymInstr* QuarkAnd(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x24, a, b, c, s); }
SymInstr* QuarkAnd(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x24, a, b, immed); }
SymInstr* QuarkOr(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x25, a, b, c, s); }
SymInstr* QuarkOr(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x25, a, b, immed); }
SymInstr* QuarkXor(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x26, a, b, c, s); }
SymInstr* QuarkXor(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x26, a, b, immed); }
SymInstr* QuarkSar(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x27, a, b, c, s); }
SymInstr* QuarkSar(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x27, a, b, immed); }
SymInstr* QuarkShl(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x28, a, b, c, s); }
SymInstr* QuarkShl(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x28, a, b, immed); }
SymInstr* QuarkShr(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x29, a, b, c, s); }
SymInstr* QuarkShr(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x29, a, b, immed); }
SymInstr* QuarkRol(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x2a, a, b, c, s); }
SymInstr* QuarkRol(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x2a, a, b, immed); }
SymInstr* QuarkRor(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x2b, a, b, c, s); }
SymInstr* QuarkRor(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x2b, a, b, immed); }
SymInstr* QuarkFadd(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x38, a, b, c); }
SymInstr* QuarkFadd(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x38, a, b, immed); }
SymInstr* QuarkFsub(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x39, a, b, c); }
SymInstr* QuarkFsub(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x39, a, b, immed); }
SymInstr* QuarkFmul(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x3a, a, b, c); }
SymInstr* QuarkFmul(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x3a, a, b, immed); }
SymInstr* QuarkFdiv(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x3b, a, b, c); }
SymInstr* QuarkFdiv(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x3b, a, b, immed); }
SymInstr* QuarkFmod(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x3c, a, b, c); }
SymInstr* QuarkFmod(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x3c, a, b, immed); }
SymInstr* QuarkFpow(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x3d, a, b, c); }
SymInstr* QuarkFpow(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x3d, a, b, immed); }
SymInstr* QuarkFlog(uint32_t a, uint32_t b, uint32_t c) { return new QuarkFloat3OpInstr(0x3e, a, b, c); }
SymInstr* QuarkFlog(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x3e, a, b, immed); }

SymInstr* QuarkCmp(uint32_t op, uint32_t b, uint32_t a, uint32_t c, uint32_t s) { return new QuarkCmpInstr(0x2d, op, b, a, c, s); }
SymInstr* QuarkCmp(uint32_t op, uint32_t b, uint32_t a, int32_t immed) { return new QuarkCmpInstr(0x2d, op, b, a, immed); }
SymInstr* QuarkIcmp(uint32_t op, uint32_t b, uint32_t a, uint32_t c, uint32_t s) { return new QuarkCmpInstr(0x2e, op, b, a, c, s); }
SymInstr* QuarkIcmp(uint32_t op, uint32_t b, uint32_t a, int32_t immed) { return new QuarkCmpInstr(0x2e, op, b, a, immed); }
SymInstr* QuarkFcmp(uint32_t op, uint32_t b, uint32_t a, uint32_t c, uint32_t s) { return new QuarkCmpInstr(0x2f, op, b, a, c, s); }
SymInstr* QuarkFcmp(uint32_t op, uint32_t b, uint32_t a, int32_t immed) { return new QuarkCmpInstr(0x2f, op, b, a, immed); }
SymInstr* QuarkCondCmp(uint32_t cc, uint32_t val, uint32_t op, uint32_t b, uint32_t a, uint32_t c, uint32_t s) { return new QuarkCondCmpInstr(0x2d, cc, val, op, b, a, c, s); }
SymInstr* QuarkCondCmp(uint32_t cc, uint32_t val, uint32_t op, uint32_t b, uint32_t a, int32_t immed) { return new QuarkCondCmpInstr(0x2d, cc, val, op, b, a, immed); }
SymInstr* QuarkCondIcmp(uint32_t cc, uint32_t val, uint32_t op, uint32_t b, uint32_t a, uint32_t c, uint32_t s) { return new QuarkCondCmpInstr(0x2e, cc, val, op, b, a, c, s); }
SymInstr* QuarkCondIcmp(uint32_t cc, uint32_t val, uint32_t op, uint32_t b, uint32_t a, int32_t immed) { return new QuarkCondCmpInstr(0x2e, cc, val, op, b, a, immed); }

SymInstr* QuarkMov(uint32_t a, uint32_t b, uint32_t s) { return new Quark2OpInstr(0x1f00, a, b, s); }
SymInstr* QuarkMov(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x1f00, a, immed); }
SymInstr* QuarkXchg(uint32_t a, uint32_t b) { return new QuarkXchgInstr(a, b); }
SymInstr* QuarkSxb(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f02, a, b); }
SymInstr* QuarkSxh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f03, a, b); }
SymInstr* QuarkSwaph(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f04, a, b); }
SymInstr* QuarkSwapw(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f05, a, b); }
SymInstr* QuarkNeg(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f08, a, b); }
SymInstr* QuarkNot(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f09, a, b); }

SymInstr* QuarkLoadCR(uint32_t a) { return new Quark1OpWriteRegInstr(0x1f0e, a); }
SymInstr* QuarkStoreCR(uint32_t a) { return new Quark1OpReadRegInstr(0x1f0f, a); }

SymInstr* QuarkSetCC(uint32_t a) { return new QuarkCondBit1OpInstr(0x1f18, a); }
SymInstr* QuarkClrCC(uint32_t a) { return new QuarkCondBit1OpInstr(0x1f19, a); }
SymInstr* QuarkNotCC(uint32_t a, uint32_t b) { return new QuarkCondBit2OpInstr(0x1f1a, a, b); }
SymInstr* QuarkAndCC(uint32_t a, uint32_t b, uint32_t c) { return new QuarkCondBit3OpInstr(0x1f1c, a, b, c); }
SymInstr* QuarkOrCC(uint32_t a, uint32_t b, uint32_t c) { return new QuarkCondBit3OpInstr(0x1f1d, a, b, c); }
SymInstr* QuarkXorCC(uint32_t a, uint32_t b, uint32_t c) { return new QuarkCondBit3OpInstr(0x1f1e, a, b, c); }
SymInstr* QuarkMovCC(uint32_t a, uint32_t b) { return new QuarkCondBit2OpInstr(0x1f1b, a, b); }

SymInstr* QuarkBreakpoint() { return new QuarkBreakpointInstr(); }

SymInstr* QuarkLoadFI(uint32_t a, uint32_t b, uint32_t s) { return new Quark2OpInstr(0x3f00, a, b, s); }
SymInstr* QuarkLoadFI(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f00, a, immed); }
SymInstr* QuarkStoreFI(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f01, a, b); }
SymInstr* QuarkLoadPi(uint32_t a) { return new Quark1OpWriteRegInstr(0x3f02, a); }
SymInstr* QuarkLoadE(uint32_t a) { return new Quark1OpWriteRegInstr(0x3f03, a); }
SymInstr* QuarkEtoXReg(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f04, a, b); }
SymInstr* QuarkEtoXImmed(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f04, a, immed); }
SymInstr* Quark2toXReg(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f05, a, b); }
SymInstr* Quark2toXImmed(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f05, a, immed); }
SymInstr* Quark10toXReg(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f06, a, b); }
SymInstr* Quark10toXImmed(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f06, a, immed); }
SymInstr* QuarkSqrt(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f08, a, b); }
SymInstr* QuarkSqrt(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f08, a, immed); }
SymInstr* QuarkRecip(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f09, a, b); }
SymInstr* QuarkRecip(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f09, a, immed); }
SymInstr* QuarkRecipSqrt(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f0a, a, b); }
SymInstr* QuarkRecipSqrt(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f0a, a, immed); }
SymInstr* QuarkFneg(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f0b, a, b); }
SymInstr* QuarkFsin(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f0c, a, b); }
SymInstr* QuarkFcos(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f0d, a, b); }
SymInstr* QuarkFtan(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f0e, a, b); }
SymInstr* QuarkFloor(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f0f, a, b); }
SymInstr* QuarkFasin(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f10, a, b); }
SymInstr* QuarkFacos(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f11, a, b); }
SymInstr* QuarkFatan(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f12, a, b); }
SymInstr* QuarkCeil(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f13, a, b); }
SymInstr* QuarkFsinh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f14, a, b); }
SymInstr* QuarkFcosh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f15, a, b); }
SymInstr* QuarkFtanh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f16, a, b); }
SymInstr* QuarkRound(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f17, a, b); }
SymInstr* QuarkFasinh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f18, a, b); }
SymInstr* QuarkFacosh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f19, a, b); }
SymInstr* QuarkFatanh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f1a, a, b); }
SymInstr* QuarkFabs(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f1b, a, b); }
SymInstr* QuarkFln(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f1c, a, b); }
SymInstr* QuarkFln(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f1c, a, immed); }
SymInstr* QuarkFlog2(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f1d, a, b); }
SymInstr* QuarkFlog2(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f1d, a, immed); }
SymInstr* QuarkFlog10(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f1e, a, b); }
SymInstr* QuarkFlog10(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f1e, a, immed); }
SymInstr* QuarkFmov(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x3f1f, a, b); }
SymInstr* QuarkFmov(uint32_t a, int32_t immed) { return new Quark2OpInstr(0x3f1f, a, immed); }

