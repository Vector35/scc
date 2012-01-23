#ifdef OUTPUT_CLASS_NAME

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "OutputX86Common.h"
#include "Struct.h"
#include "Function.h"
#include "asmx86.h"

#define X86_MEM_REF(ref) X86_MEM_INDEX((ref).base, (ref).index, (ref).scale, (ref).offset)
#define X86_MEM_REF_OFFSET(ref, ofs) X86_MEM_INDEX((ref).base, (ref).index, (ref).scale, (ref).offset + (ofs))

using namespace std;
using namespace asmx86;


bool OUTPUT_CLASS_NAME::OperandReference::operator==(const OperandReference& ref) const
{
	if (type != ref.type)
		return false;
	if (width != ref.width)
		return false;

	switch (type)
	{
	case OPERANDREF_REG:
		if (reg != ref.reg)
			return false;
#ifdef OUTPUT32
		if ((width == 8) && (highReg != ref.highReg))
			return false;
#endif
		return true;
	case OPERANDREF_MEM:
		if (mem.base != ref.mem.base)
			return false;
		if (mem.index != ref.mem.index)
			return false;
		if (mem.scale != ref.mem.scale)
			return false;
		if (mem.offset != ref.mem.offset)
			return false;
		return true;
	case OPERANDREF_IMMED:
		if (immed != ref.immed)
			return false;
		return true;
	default:
		return false;
	}
}


bool OUTPUT_CLASS_NAME::OperandReference::operator!=(const OperandReference& ref) const
{
	return !((*this) == ref);
}


OUTPUT_CLASS_NAME::OUTPUT_CLASS_NAME(const Settings& settings): Output(settings)
{
}


OperandType OUTPUT_CLASS_NAME::GetRegisterOfSize(OperandType base, size_t size)
{
	switch (size)
	{
	case 1:
		switch (base)
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
	case 2:
		switch (base)
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
	case 4:
		switch (base)
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
	case 8:
		switch (base)
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


bool OUTPUT_CLASS_NAME::IsRegisterValid(OperandType reg)
{
#ifdef OUTPUT32
	if ((reg == REG_SPL) || (reg == REG_BPL) || (reg == REG_SIL) || (reg == REG_DIL))
		return false;
	return true;
#else
	return true;
#endif
}


bool OUTPUT_CLASS_NAME::IsValidIndexRegister(asmx86::OperandType reg)
{
	if ((reg == REG_ESP) || (reg == REG_R12D))
		return false;
	if ((reg == REG_RSP) || (reg == REG_R12))
		return false;
	return true;
}


OperandType OUTPUT_CLASS_NAME::AllocateTemporaryRegister(OutputBlock* out, size_t size, RegisterUsageType usage)
{
	size_t possibleRegs[16];
	size_t possibleRegCount = 0;

	for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
	{
		if ((!m_alloc[i]) && (!m_reserved[i]))
		{
			OperandType reg = GetRegisterOfSize(m_temporaryRegisters[i], size);
			if (!IsRegisterValid(reg))
				continue;
			if ((usage == USAGE_INDEX) && (!IsValidIndexRegister(reg)))
				continue;
			possibleRegs[possibleRegCount++] = i;
		}
	}

	if (possibleRegCount == 0)
		return NONE;

	if (m_settings.polymorph)
	{
		size_t choice = rand() % possibleRegCount;
		m_alloc[possibleRegs[choice]] = true;
		return GetRegisterOfSize(m_temporaryRegisters[possibleRegs[choice]], size);
	}
	else
	{
		m_alloc[possibleRegs[0]] = true;
		return GetRegisterOfSize(m_temporaryRegisters[possibleRegs[0]], size);
	}
}


void OUTPUT_CLASS_NAME::ReserveRegisters(OutputBlock* out, ...)
{
	OperandType regs[16];
	size_t regCount;
	va_list va;
	va_start(va, out);

	// Process register list and mark them as reserved
	for (regCount = 0; regCount < 16; regCount++)
	{
		OperandType reg = (OperandType)va_arg(va, int);
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
#ifdef OUTPUT32
			if (GetRegisterOfSize(regs[i], 4) == m_stackPointer)
			{
				// Caller needs register that currently holds the stack pointer, relocate it to another register
				OperandType temp = AllocateTemporaryRegister(out, 4);
				EMIT_RR(xchg_32, m_stackPointer, temp);
				m_stackPointer = temp;
			}
			else if (GetRegisterOfSize(regs[i], 4) == m_framePointer)
			{
				// Caller needs register that currently holds the frame pointer, relocate it to another register
				OperandType temp = AllocateTemporaryRegister(out, 4);
				EMIT_RR(xchg_32, m_framePointer, temp);
				m_framePointer = temp;
			}
			else if (GetRegisterOfSize(regs[i], 4) == m_basePointer)
			{
				// Caller needs register that currently holds the base pointer, relocate it to another register
				OperandType temp = AllocateTemporaryRegister(out, 4);
				EMIT_RR(xchg_32, m_basePointer, temp);
				m_basePointer = temp;
			}
#else
			if (GetRegisterOfSize(regs[i], 8) == m_stackPointer)
			{
				// Caller needs register that currently holds the stack pointer, relocate it to another register
				OperandType temp = AllocateTemporaryRegister(out, 8);
				EMIT_RR(xchg_64, m_stackPointer, temp);
				m_stackPointer = temp;
			}
			else if (GetRegisterOfSize(regs[i], 8) == m_framePointer)
			{
				// Caller needs register that currently holds the frame pointer, relocate it to another register
				OperandType temp = AllocateTemporaryRegister(out, 8);
				EMIT_RR(xchg_64, m_framePointer, temp);
				m_framePointer = temp;
			}
			else if (GetRegisterOfSize(regs[i], 8) == m_basePointer)
			{
				// Caller needs register that currently holds the base pointer, relocate it to another register
				OperandType temp = AllocateTemporaryRegister(out, 8);
				EMIT_RR(xchg_64, m_basePointer, temp);
				m_basePointer = temp;
			}
#endif
		}
	}
}


void OUTPUT_CLASS_NAME::ClearReservedRegisters(OutputBlock* out)
{
	if (m_stackPointer != m_origStackPointer)
	{
		// Stack pointer was relocated, move it back
#ifdef OUTPUT32
		EMIT_RR(xchg_32, m_stackPointer, m_origStackPointer);
#else
		EMIT_RR(xchg_64, m_stackPointer, m_origStackPointer);
#endif
	}

	if (m_framePointer != m_origFramePointer)
	{
		// Frame pointer was relocated, move it back
#ifdef OUTPUT32
		EMIT_RR(xchg_32, m_framePointer, m_origFramePointer);
#else
		EMIT_RR(xchg_64, m_framePointer, m_origFramePointer);
#endif
	}

	if (m_basePointer != m_origBasePointer)
	{
		// Base pointer was relocated, move it back
#ifdef OUTPUT32
		EMIT_RR(xchg_32, m_basePointer, m_origBasePointer);
#else
		EMIT_RR(xchg_64, m_basePointer, m_origBasePointer);
#endif
	}

	m_stackPointer = m_origStackPointer;
	m_framePointer = m_origFramePointer;
	m_basePointer = m_origBasePointer;

	for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
		m_reserved[i] = false;
}


OperandType OUTPUT_CLASS_NAME::GetRegisterByName(const string& name)
{
	if (name == "al")
		return REG_AL;
	if (name == "ah")
		return REG_AH;
	if (name == "ax")
		return REG_AX;
	if (name == "eax")
		return REG_EAX;
	if (name == "rax")
		return REG_RAX;
	if (name == "cl")
		return REG_CL;
	if (name == "ch")
		return REG_CH;
	if (name == "cx")
		return REG_CX;
	if (name == "ecx")
		return REG_ECX;
	if (name == "rcx")
		return REG_RCX;
	if (name == "dl")
		return REG_DL;
	if (name == "dh")
		return REG_DH;
	if (name == "dx")
		return REG_DX;
	if (name == "edx")
		return REG_EDX;
	if (name == "rdx")
		return REG_RDX;
	if (name == "bl")
		return REG_BL;
	if (name == "bh")
		return REG_BH;
	if (name == "bx")
		return REG_BX;
	if (name == "ebx")
		return REG_EBX;
	if (name == "rbx")
		return REG_RBX;
	if (name == "spl")
		return REG_SPL;
	if (name == "sp")
		return REG_SP;
	if (name == "esp")
		return REG_ESP;
	if (name == "rsp")
		return REG_RSP;
	if (name == "bpl")
		return REG_BPL;
	if (name == "bp")
		return REG_BP;
	if (name == "ebp")
		return REG_EBP;
	if (name == "rbp")
		return REG_RBP;
	if (name == "sil")
		return REG_SIL;
	if (name == "si")
		return REG_SI;
	if (name == "esi")
		return REG_ESI;
	if (name == "rsi")
		return REG_RSI;
	if (name == "dil")
		return REG_DIL;
	if (name == "di")
		return REG_DI;
	if (name == "edi")
		return REG_EDI;
	if (name == "rdi")
		return REG_RDI;
	if (name == "r8b")
		return REG_R8B;
	if (name == "r8w")
		return REG_R8W;
	if (name == "r8d")
		return REG_R8D;
	if (name == "r8")
		return REG_R8;
	if (name == "r9b")
		return REG_R9B;
	if (name == "r9w")
		return REG_R9W;
	if (name == "r9d")
		return REG_R9D;
	if (name == "r9")
		return REG_R9;
	if (name == "r10b")
		return REG_R10B;
	if (name == "r10w")
		return REG_R10W;
	if (name == "r10d")
		return REG_R10D;
	if (name == "r10")
		return REG_R10;
	if (name == "r11b")
		return REG_R11B;
	if (name == "r11w")
		return REG_R11W;
	if (name == "r11d")
		return REG_R11D;
	if (name == "r11")
		return REG_R11;
	if (name == "r12b")
		return REG_R12B;
	if (name == "r12w")
		return REG_R12W;
	if (name == "r12d")
		return REG_R12D;
	if (name == "r12")
		return REG_R12;
	if (name == "r13b")
		return REG_R13B;
	if (name == "r13w")
		return REG_R13W;
	if (name == "r13d")
		return REG_R13D;
	if (name == "r13")
		return REG_R13;
	if (name == "r14b")
		return REG_R14B;
	if (name == "r14w")
		return REG_R14W;
	if (name == "r14d")
		return REG_R14D;
	if (name == "r14")
		return REG_R14;
	if (name == "r15b")
		return REG_R15B;
	if (name == "r15w")
		return REG_R15W;
	if (name == "r15d")
		return REG_R15D;
	if (name == "r15")
		return REG_R15;
	return NONE;
}


void OUTPUT_CLASS_NAME::LeaOverflowHandler(OutputBlock* out, size_t start, size_t offset)
{
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
}


void OUTPUT_CLASS_NAME::BaseRelativeLeaOverflowHandler(OutputBlock* out, size_t start, size_t offset)
{
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
}


void OUTPUT_CLASS_NAME::ConditionalJumpOverflowHandler(OutputBlock* out, size_t start, size_t offset)
{
	uint8_t* instr = (uint8_t*)((size_t)out->code + start);
	uint8_t newInstr[6];
	newInstr[0] = 0x0f;
	newInstr[1] = 0x80 + (instr[0] & 0x0f);
	*(uint32_t*)&newInstr[2] = instr[1];
	out->ReplaceInstruction(start, 2, newInstr, 6, 2);
}


void OUTPUT_CLASS_NAME::UnconditionalJumpOverflowHandler(OutputBlock* out, size_t start, size_t offset)
{
	uint8_t* instr = (uint8_t*)((size_t)out->code + start);
	uint8_t newInstr[5];
	newInstr[0] = 0xe9;
	*(uint32_t*)&newInstr[1] = instr[1];
	out->ReplaceInstruction(start, 2, newInstr, 5, 1);
}


size_t OUTPUT_CLASS_NAME::GetInstructionPointer(OutputBlock* out, OperandType reg)
{
	size_t capturedOffset, leaOffset;

	if (m_normalStack)
	{
		// Normal stack, use call/pop method
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
		buffer[0] = 0xe8;
		*(uint32_t*)(&buffer[1]) = 0;
		out->FinishWrite(5);
		capturedOffset = out->len;
		EMIT_R(pop, reg);
		leaOffset = out->len;
		EMIT_RM(lea_32, reg, X86_MEM(reg, 1));
	}
	else
	{
		// Not a normal stack, must use fstenv method
		capturedOffset = out->len;
		EMIT(fnop);
#ifdef OUTPUT32
		EMIT_M(fstenv, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 4 : -28));
		EMIT_RM(mov_32, reg, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 16 : -16));
		leaOffset = out->len;
		EMIT_RM(lea_32, reg, X86_MEM(reg, 1));
#else
		EMIT_M(fstenv, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 8 : -28));
		EMIT_RM(mov_64, reg, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 20 : -16));
		leaOffset = out->len;
		EMIT_RM(lea_64, reg, X86_MEM(reg, 1));
#endif
	}

	*(int8_t*)((size_t)out->code + out->len - 1) = (int8_t)(out->len - capturedOffset);
	return leaOffset;
}


bool OUTPUT_CLASS_NAME::AccessVariableStorage(OutputBlock* out, const ILParameter& param, X86MemoryReference& ref)
{
	if (param.cls == ILPARAM_MEMBER)
	{
		OperandReference parent;
		if (!PrepareLoad(out, *param.parent, parent))
			return false;
		if (parent.type != OPERANDREF_MEM)
			return false;
		if (!param.structure)
			return false;

		const StructMember* member = param.structure->GetMember(param.stringValue);
		if (!member)
			return false;

		parent.mem.offset += member->offset;
		ref = parent.mem;
		return true;
	}

	if (param.cls != ILPARAM_VAR)
		return false;

	if (param.variable->IsGlobal())
	{
#ifdef OUTPUT32
		OperandType ptr = AllocateTemporaryRegister(out, 4);
#else
		OperandType ptr = AllocateTemporaryRegister(out, 8);
#endif
		ref.base = ptr;
		ref.index = NONE;
		ref.scale = 1;
		ref.offset = 0;

#ifdef OUTPUT32
		if (!m_settings.positionIndependent)
		{
			EMIT_RI(mov_32, ptr, 0);
			Relocation reloc;
			reloc.type = DATA_RELOC_ABSOLUTE_32;
			reloc.offset = out->len - 4;
			reloc.dataOffset = param.variable->GetDataSectionOffset();
			out->relocs.push_back(reloc);
			return true;
		}

		if (m_basePointer != NONE)
		{
			size_t leaOffset = out->len;
			EMIT_RM(lea_32, ptr, X86_MEM(m_basePointer, 1));
			*(int8_t*)((size_t)out->code + out->len - 1) = 0;
			Relocation reloc;
			reloc.type = DATA_RELOC_BASE_RELATIVE_8;
			reloc.overflow = BaseRelativeLeaOverflowHandler;
			reloc.instruction = leaOffset;
			reloc.offset = out->len - 1;
			reloc.dataOffset = param.variable->GetDataSectionOffset();
			out->relocs.push_back(reloc);
			return true;
		}

		size_t leaOffset = GetInstructionPointer(out, ptr);

		Relocation reloc;
		reloc.type = DATA_RELOC_RELATIVE_8;
		reloc.overflow = LeaOverflowHandler;
		reloc.instruction = leaOffset;
		reloc.offset = out->len - 1;
		reloc.dataOffset = param.variable->GetDataSectionOffset();
		out->relocs.push_back(reloc);
#else
		EMIT_RM(lea_64, ptr, X86_MEM(REG_RIP, 0));
		Relocation reloc;
		reloc.type = DATA_RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.dataOffset = param.variable->GetDataSectionOffset();
		out->relocs.push_back(reloc);
#endif

		return true;
	}

	map<Variable*, int32_t>::iterator i = m_stackFrame.find(param.variable);
	if (i == m_stackFrame.end())
		return false;
	if (m_framePointerEnabled)
	{
		ref.base = m_framePointer;
		ref.index = NONE;
		ref.scale = 1;
		ref.offset = i->second;
	}
	else
	{
		ref.base = m_stackPointer;
		ref.index = NONE;
		ref.scale = 1;
		ref.offset = i->second;
	}
	return true;
}


bool OUTPUT_CLASS_NAME::LoadCodePointer(OutputBlock* out, ILBlock* block, OperandReference& ref)
{
	ref.type = OPERANDREF_REG;
#ifdef OUTPUT32
	ref.width = 4;
#else
	ref.width = 8;
#endif
	ref.reg = AllocateTemporaryRegister(out, ref.width);

#ifdef OUTPUT32
	if (m_basePointer != NONE)
	{
		size_t leaOffset = out->len;
		EMIT_RM(lea_32, ref.reg, X86_MEM(m_basePointer, 1));
		*(int8_t*)((size_t)out->code + out->len - 1) = 0;
		Relocation reloc;
		reloc.type = CODE_RELOC_BASE_RELATIVE_8;
		reloc.overflow = BaseRelativeLeaOverflowHandler;
		reloc.instruction = leaOffset;
		reloc.offset = out->len - 1;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}
	else if (m_settings.positionIndependent)
	{
		size_t leaOffset = GetInstructionPointer(out, ref.reg);

		Relocation reloc;
		reloc.type = CODE_RELOC_RELATIVE_8;
		reloc.overflow = LeaOverflowHandler;
		reloc.instruction = leaOffset;
		reloc.offset = out->len - 1;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}
	else
	{
		EMIT_RI(mov_32, ref.reg, 0);
		Relocation reloc;
		reloc.type = CODE_RELOC_ABSOLUTE_32;
		reloc.offset = out->len - 4;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}
#else
	EMIT_RM(lea_64, ref.reg, X86_MEM(REG_RIP, 0));

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32;
	reloc.offset = out->len - 4;
	reloc.target = block;
	out->relocs.push_back(reloc);
#endif

	if (m_settings.encodePointers)
	{
		ILParameter keyParam(m_settings.encodePointerKey);
		OperandReference key;
		if (!PrepareLoad(out, keyParam, key))
			return false;
		if (!Xor(out, ref, key))
			return false;
	}

	return true;
}


bool OUTPUT_CLASS_NAME::PrepareLoad(OutputBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		ref.type = OPERANDREF_MEM;
		if (!AccessVariableStorage(out, param, ref.mem))
			return false;
		return true;
	case ILPARAM_INT:
		ref.type = OPERANDREF_IMMED;
		ref.immed = param.integerValue;
		return true;
	case ILPARAM_BOOL:
		ref.type = OPERANDREF_IMMED;
		ref.immed = param.boolValue ? 1 : 0;
		return true;
	case ILPARAM_FUNC:
		return LoadCodePointer(out, param.function->GetIL()[0], ref);
	case ILPARAM_UNDEFINED:
		ref.type = OPERANDREF_REG;
		ref.width = param.GetWidth();
		ref.reg = GetRegisterOfSize(REG_EAX, ref.width);
		return true;
	default:
		return false;
	}
}


bool OUTPUT_CLASS_NAME::PrepareStore(OutputBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		ref.type = OPERANDREF_MEM;
		if (!AccessVariableStorage(out, param, ref.mem))
			return false;
		return true;
	default:
		return false;
	}
}


#ifdef OUTPUT32
#define IMPLEMENT_BINARY_OP_64(operation, highOperation) \
	if (dest.type == OPERANDREF_REG) \
	{ \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_RR(operation ## _32, dest.reg, src.reg); \
			EMIT_RR(highOperation ## _32, dest.highReg, src.highReg); \
			return true; \
		case OPERANDREF_MEM: \
			EMIT_RM(operation ## _32, dest.reg, X86_MEM_REF(src.mem)); \
			EMIT_RM(highOperation ## _32, dest.highReg, X86_MEM_REF_OFFSET(src.mem, 4)); \
			return true; \
		case OPERANDREF_IMMED: \
			EMIT_RI(operation ## _32, dest.reg, (uint32_t)src.immed); \
			EMIT_RI(highOperation ## _32, dest.highReg, (uint32_t)(src.immed >> 32)); \
			return true; \
		default: \
			return false; \
		} \
	} \
	else if (dest.type == OPERANDREF_MEM) \
	{ \
		OperandType temp; \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), src.reg); \
			EMIT_MR(highOperation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), src.highReg); \
			return true; \
		case OPERANDREF_MEM: \
			temp = AllocateTemporaryRegister(out, 4); \
			EMIT_RM(mov_32, temp, X86_MEM_REF(src.mem)); \
			EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), temp); \
			EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(src.mem, 4)); \
			EMIT_MR(highOperation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), temp); \
			return true; \
		case OPERANDREF_IMMED: \
			EMIT_MI(operation ## _32, X86_MEM_REF(dest.mem), (uint32_t)src.immed); \
			EMIT_MI(highOperation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), (uint32_t)(src.immed >> 32)); \
			return true; \
		default: \
			return false; \
		} \
	}
#else
#define IMPLEMENT_BINARY_OP_64(operation, highOperation) \
	if (dest.type == OPERANDREF_REG) \
	{ \
		switch (src.type) \
		{ \
		case OPERANDREF_REG:  EMIT_RR(operation ## _64, dest.reg, src.reg); return true; \
		case OPERANDREF_MEM:  EMIT_RM(operation ## _64, dest.reg, X86_MEM_REF(src.mem)); return true; \
		case OPERANDREF_IMMED:  EMIT_RI(operation ## _64, dest.reg, src.immed); return true; \
		default:  return false; \
		} \
	} \
	else if (dest.type == OPERANDREF_MEM) \
	{ \
		OperandType temp; \
		switch (src.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(operation ## _64, X86_MEM_REF(dest.mem), src.reg); return true; \
		case OPERANDREF_MEM: \
			temp = AllocateTemporaryRegister(out, dest.width); \
			EMIT_RM(mov_64, temp, X86_MEM_REF(src.mem)); \
			EMIT_MR(operation ## _64, X86_MEM_REF(dest.mem), temp); \
			return true; \
		case OPERANDREF_IMMED: \
			if ((src.immed < -0x80000000LL) || (src.immed >= 0x80000000LL)) \
			{ \
				temp = AllocateTemporaryRegister(out, dest.width); \
				EMIT_RI(mov_64, temp, src.immed); \
				EMIT_MR(operation ## _64, X86_MEM_REF(dest.mem), temp); \
			} \
			else \
			{ \
				EMIT_MI(operation ## _64, X86_MEM_REF(dest.mem), src.immed); \
			} \
			return true; \
		default:  return false; \
		} \
	}
#endif

#define IMPLEMENT_BINARY_OP(operation, highOperation) \
	if (dest.width == 1) \
	{ \
		if (dest.type == OPERANDREF_REG) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(operation ## _8, dest.reg, src.reg); return true; \
			case OPERANDREF_MEM:  EMIT_RM(operation ## _8, dest.reg, X86_MEM_REF(src.mem)); return true; \
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _8, dest.reg, (uint8_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
		else if (dest.type == OPERANDREF_MEM) \
		{ \
			OperandType temp; \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _8, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_MEM: \
				temp = AllocateTemporaryRegister(out, dest.width); \
				EMIT_RM(mov_8, temp, X86_MEM_REF(src.mem)); \
				EMIT_MR(operation ## _8, X86_MEM_REF(dest.mem), temp); \
				return true; \
			case OPERANDREF_IMMED:  EMIT_MI(operation ## _8, X86_MEM_REF(dest.mem), (uint8_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
	} \
	else if (dest.width == 2) \
	{ \
		if (dest.type == OPERANDREF_REG) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(operation ## _16, dest.reg, src.reg); return true; \
			case OPERANDREF_MEM:  EMIT_RM(operation ## _16, dest.reg, X86_MEM_REF(src.mem)); return true; \
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _16, dest.reg, (uint16_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
		else if (dest.type == OPERANDREF_MEM) \
		{ \
			OperandType temp; \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _16, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_MEM: \
				temp = AllocateTemporaryRegister(out, dest.width); \
				EMIT_RM(mov_16, temp, X86_MEM_REF(src.mem)); \
				EMIT_MR(operation ## _16, X86_MEM_REF(dest.mem), temp); \
				return true; \
			case OPERANDREF_IMMED:  EMIT_MI(operation ## _16, X86_MEM_REF(dest.mem), (uint16_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
	} \
	else if (dest.width == 4) \
	{ \
		if (dest.type == OPERANDREF_REG) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(operation ## _32, dest.reg, src.reg); return true; \
			case OPERANDREF_MEM:  EMIT_RM(operation ## _32, dest.reg, X86_MEM_REF(src.mem)); return true; \
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _32, dest.reg, (uint32_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
		else if (dest.type == OPERANDREF_MEM) \
		{ \
			OperandType temp; \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_MEM: \
				temp = AllocateTemporaryRegister(out, dest.width); \
				EMIT_RM(mov_32, temp, X86_MEM_REF(src.mem)); \
				EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), temp); \
				return true; \
			case OPERANDREF_IMMED:  EMIT_MI(operation ## _32, X86_MEM_REF(dest.mem), (uint32_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
	} \
	else if (dest.width == 8) \
	{ \
		IMPLEMENT_BINARY_OP_64(operation, highOperation) \
	} \
	return false;


bool OUTPUT_CLASS_NAME::Move(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	if (dest == src)
		return true;
	IMPLEMENT_BINARY_OP(mov, mov);
}


bool OUTPUT_CLASS_NAME::Add(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(add, adc);
}


bool OUTPUT_CLASS_NAME::Sub(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(sub, sbb);
}


bool OUTPUT_CLASS_NAME::And(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(and, and);
}


bool OUTPUT_CLASS_NAME::Or(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(or, or);
}


bool OUTPUT_CLASS_NAME::Xor(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(xor, xor);
}


#ifdef OUTPUT32
#define IMPLEMENT_SHIFT_OP_64(operation, right, sign) \
	if (dest.type == OPERANDREF_REG) \
	{ \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			if (right) \
			{ \
				OperandType temp = AllocateTemporaryRegister(out, 1); \
				OperandType temp32 = GetRegisterOfSize(temp, 4); \
				EMIT_RR(xor_32, temp32, temp32); \
				if (sign) \
				{ \
					EMIT_RR(test_32, dest.highReg, dest.highReg); \
					EMIT_R(sets, temp); \
					EMIT_R(neg_32, temp32); \
				} \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, dest.reg, dest.highReg); \
				EMIT_RR(cmovnz_32, dest.highReg, temp32); \
				EMIT_RRR(shrd_32, dest.reg, dest.highReg, src.reg); \
				EMIT_RR(operation ## _32, dest.highReg, src.reg); \
			} \
			else \
			{ \
				OperandType temp = AllocateTemporaryRegister(out, 4); \
				EMIT_RR(xor_32, temp, temp); \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, dest.highReg, dest.reg); \
				EMIT_RR(cmovnz_32, dest.reg, temp); \
				EMIT_RRR(shld_32, dest.highReg, dest.reg, src.reg); \
				EMIT_RR(operation ## _32, dest.reg, src.reg); \
			} \
			return true; \
		case OPERANDREF_IMMED: \
			if (right) \
			{ \
				if ((src.immed & 63) >= 32) \
				{ \
					EMIT_RR(mov_32, dest.reg, dest.highReg); \
					if (src.immed & 31) \
						EMIT_RI(operation ## _32, dest.reg, (src.immed & 31)); \
					if (sign) \
					{ \
						if ((dest.reg == REG_EAX) && (dest.highReg == REG_EDX)) \
							EMIT(cdq); \
						else \
							EMIT_RI(operation ## _32, dest.highReg, 31); \
					} \
					else \
					{ \
						EMIT_RR(xor_32, dest.highReg, dest.highReg); \
					} \
				} \
				else \
				{ \
					EMIT_RRI(shrd_32, dest.reg, dest.highReg, src.immed); \
					EMIT_RI(operation ## _32, dest.highReg, src.immed); \
				} \
			} \
			else \
			{ \
				if ((src.immed & 63) >= 32) \
				{ \
					EMIT_RR(mov_32, dest.highReg, dest.reg); \
					EMIT_RR(xor_32, dest.reg, dest.reg); \
					if (src.immed & 31) \
						EMIT_RI(operation ## _32, dest.highReg, (src.immed & 31)); \
				} \
				else \
				{ \
					EMIT_RRI(shld_32, dest.highReg, dest.reg, src.immed); \
					EMIT_RI(operation ## _32, dest.reg, src.immed); \
				} \
			} \
			return true; \
		default: \
			return false; \
		} \
	} \
	else if (dest.type == OPERANDREF_MEM) \
	{ \
		OperandType temp = AllocateTemporaryRegister(out, 4); \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			if (right) \
			{ \
				OperandType high = AllocateTemporaryRegister(out, 1); \
				OperandType high32 = GetRegisterOfSize(high, 4); \
				EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_RR(xor_32, high32, high32); \
				if (sign) \
				{ \
					EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(dest.mem, 4), high32); \
					EMIT_R(sets, high); \
					EMIT_R(neg_32, high32); \
				} \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, temp, high32); \
				OperandType low = high32; \
				EMIT_RM(mov_32, low, X86_MEM_REF(dest.mem)); \
				EMIT_RM(cmovnz_32, low, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_RRR(shrd_32, low, temp, src.reg); \
				EMIT_RR(operation ## _32, temp, src.reg); \
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), low); \
				EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), temp); \
			} \
			else \
			{ \
				OperandType low = AllocateTemporaryRegister(out, 4); \
				EMIT_RM(mov_32, temp, X86_MEM_REF(dest.mem)); \
				EMIT_RR(xor_32, low, low); \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, temp, low); \
				OperandType high = low; \
				EMIT_RM(mov_32, high, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_RM(cmovnz_32, high, X86_MEM_REF(dest.mem)); \
				EMIT_RRR(shld_32, high, temp, src.reg); \
				EMIT_RR(operation ## _32, temp, src.reg); \
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp); \
				EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), high); \
			} \
			return true; \
		case OPERANDREF_IMMED: \
			if (right) \
			{ \
				if ((src.immed & 63) >= 32) \
				{ \
					EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(dest.mem, 4)); \
					if (src.immed & 31) \
						EMIT_RI(operation ## _32, temp, (src.immed & 31)); \
					EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp); \
					if (sign) \
						EMIT_MI(operation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), 31); \
					else \
						EMIT_MI(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0); \
				} \
				else \
				{ \
					EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(dest.mem, 4)); \
					EMIT_MRI(shrd_32, X86_MEM_REF(dest.mem), temp, src.immed); \
					EMIT_MI(operation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), src.immed); \
				} \
			} \
			else \
			{ \
				if ((src.immed & 63) >= 32) \
				{ \
					EMIT_RM(mov_32, temp, X86_MEM_REF(dest.mem)); \
					if (src.immed & 31) \
						EMIT_RI(operation ## _32, temp, (src.immed & 31)); \
					EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), temp); \
					EMIT_MI(mov_32, X86_MEM_REF(dest.mem), 0); \
				} \
				else \
				{ \
					EMIT_RM(mov_32, temp, X86_MEM_REF(dest.mem)); \
					EMIT_MRI(shld_32, X86_MEM_REF_OFFSET(dest.mem, 4), temp, src.immed); \
					EMIT_MI(operation ## _32, X86_MEM_REF(dest.mem), src.immed); \
				} \
			} \
			return true; \
		default: \
			return false; \
		} \
	}
#else
#define IMPLEMENT_SHIFT_OP_64(operation, right, sign) \
	if (dest.type == OPERANDREF_REG) \
	{ \
		switch (src.type) \
		{ \
		case OPERANDREF_REG:  EMIT_RR(operation ## _64, dest.reg, src.reg); return true; \
		case OPERANDREF_IMMED:  EMIT_RI(operation ## _64, dest.reg, src.immed); return true; \
		default:  return false; \
		} \
	} \
	else if (dest.type == OPERANDREF_MEM) \
	{ \
		switch (src.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(operation ## _64, X86_MEM_REF(dest.mem), src.reg); return true; \
		case OPERANDREF_IMMED: EMIT_MI(operation ## _64, X86_MEM_REF(dest.mem), src.immed); return true; \
		default:  return false; \
		} \
	}
#endif

#define IMPLEMENT_SHIFT_OP(operation, right, sign) \
	if (dest.width == 1) \
	{ \
		if (dest.type == OPERANDREF_REG) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(operation ## _8, dest.reg, src.reg); return true; \
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _8, dest.reg, (uint8_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
		else if (dest.type == OPERANDREF_MEM) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _8, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_IMMED:  EMIT_MI(operation ## _8, X86_MEM_REF(dest.mem), (uint8_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
	} \
	else if (dest.width == 2) \
	{ \
		if (dest.type == OPERANDREF_REG) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(operation ## _16, dest.reg, src.reg); return true; \
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _16, dest.reg, (uint8_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
		else if (dest.type == OPERANDREF_MEM) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _16, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_IMMED:  EMIT_MI(operation ## _16, X86_MEM_REF(dest.mem), (uint16_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
	} \
	else if (dest.width == 4) \
	{ \
		if (dest.type == OPERANDREF_REG) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(operation ## _32, dest.reg, src.reg); return true; \
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _32, dest.reg, (uint8_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
		else if (dest.type == OPERANDREF_MEM) \
		{ \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_IMMED:  EMIT_MI(operation ## _32, X86_MEM_REF(dest.mem), (uint32_t)src.immed); return true; \
			default:  return false; \
			} \
		} \
	} \
	else if (dest.width == 8) \
	{ \
		IMPLEMENT_SHIFT_OP_64(operation, right, sign) \
	} \
	return false;


bool OUTPUT_CLASS_NAME::ShiftLeft(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(shl, false, false);
}


bool OUTPUT_CLASS_NAME::ShiftRightUnsigned(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(shr, true, false);
}


bool OUTPUT_CLASS_NAME::ShiftRightSigned(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(sar, true, true);
}


#define IMPLEMENT_UNARY_OP_COMMON(op) \
	if (dest.width == 1) \
	{ \
		if (dest.type == OPERANDREF_REG) \
			EMIT_R(op ## _8, dest.reg); \
		else \
			EMIT_M(op ## _8, X86_MEM_REF(dest.mem)); \
		return true; \
	} \
	else if (dest.width == 2) \
	{ \
		if (dest.type == OPERANDREF_REG) \
			EMIT_R(op ## _16, dest.reg); \
		else \
			EMIT_M(op ## _16, X86_MEM_REF(dest.mem)); \
		return true; \
	} \
	else if (dest.width == 4) \
	{ \
		if (dest.type == OPERANDREF_REG) \
			EMIT_R(op ## _32, dest.reg); \
		else \
			EMIT_M(op ## _32, X86_MEM_REF(dest.mem)); \
		return true; \
	}


bool OUTPUT_CLASS_NAME::Neg(OutputBlock* out, const OperandReference& dest)
{
	IMPLEMENT_UNARY_OP_COMMON(neg)
	else if (dest.width == 8)
	{
#ifdef OUTPUT32
		if (dest.type == OPERANDREF_REG)
		{
			EMIT_R(not_32, dest.reg);
			EMIT_R(not_32, dest.highReg);
			EMIT_RI(add_32, dest.reg, 1);
			EMIT_RI(adc_32, dest.highReg, 0);
		}
		else
		{
			EMIT_M(not_32, X86_MEM_REF(dest.mem));
			EMIT_M(not_32, X86_MEM_REF_OFFSET(dest.mem, 4));
			EMIT_MI(add_32, X86_MEM_REF(dest.mem), 1);
			EMIT_MI(adc_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
		}
		return true;
#else
		if (dest.type == OPERANDREF_REG)
			EMIT_R(neg_64, dest.reg);
		else
			EMIT_M(neg_64, X86_MEM_REF(dest.mem));
		return true;
#endif
	}

	return false;
}


bool OUTPUT_CLASS_NAME::Not(OutputBlock* out, const OperandReference& dest)
{
	IMPLEMENT_UNARY_OP_COMMON(not)
	else if (dest.width == 8)
	{
#ifdef OUTPUT32
		if (dest.type == OPERANDREF_REG)
		{
			EMIT_R(not_32, dest.reg);
			EMIT_R(not_32, dest.highReg);
		}
		else
		{
			EMIT_M(not_32, X86_MEM_REF(dest.mem));
			EMIT_M(not_32, X86_MEM_REF_OFFSET(dest.mem, 4));
		}
		return true;
#else
		if (dest.type == OPERANDREF_REG)
			EMIT_R(not_64, dest.reg);
		else
			EMIT_M(not_64, X86_MEM_REF(dest.mem));
		return true;
#endif
	}

	return false;
}


bool OUTPUT_CLASS_NAME::Increment(OutputBlock* out, const OperandReference& dest)
{
	IMPLEMENT_UNARY_OP_COMMON(inc)
	else if (dest.width == 8)
	{
#ifdef OUTPUT32
		if (dest.type == OPERANDREF_REG)
		{
			EMIT_RI(add_32, dest.reg, 1);
			EMIT_RI(adc_32, dest.highReg, 0);
		}
		else
		{
			EMIT_MI(add_32, X86_MEM_REF(dest.mem), 1);
			EMIT_MI(adc_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
		}
		return true;
#else
		if (dest.type == OPERANDREF_REG)
			EMIT_R(inc_64, dest.reg);
		else
			EMIT_M(inc_64, X86_MEM_REF(dest.mem));
		return true;
#endif
	}

	return false;
}


bool OUTPUT_CLASS_NAME::Decrement(OutputBlock* out, const OperandReference& dest)
{
	IMPLEMENT_UNARY_OP_COMMON(dec)
	else if (dest.width == 8)
	{
#ifdef OUTPUT32
		if (dest.type == OPERANDREF_REG)
		{
			EMIT_RI(sub_32, dest.reg, 1);
			EMIT_RI(sbb_32, dest.highReg, 0);
		}
		else
		{
			EMIT_MI(sub_32, X86_MEM_REF(dest.mem), 1);
			EMIT_MI(sbb_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
		}
		return true;
#else
		if (dest.type == OPERANDREF_REG)
			EMIT_R(dec_64, dest.reg);
		else
			EMIT_M(dec_64, X86_MEM_REF(dest.mem));
		return true;
#endif
	}

	return false;
}


void OUTPUT_CLASS_NAME::ConditionalJump(OutputBlock* out, ConditionalJumpType type, ILBlock* trueBlock, ILBlock* falseBlock)
{
	uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
	buffer[0] = 0x70 + (uint8_t)type;
	buffer[1] = 0;
	out->FinishWrite(2);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_8;
	reloc.overflow = ConditionalJumpOverflowHandler;
	reloc.instruction = out->len - 2;
	reloc.offset = out->len - 1;
	reloc.target = trueBlock;
	out->relocs.push_back(reloc);

	if (falseBlock)
		UnconditionalJump(out, falseBlock);
}


void OUTPUT_CLASS_NAME::UnconditionalJump(OutputBlock* out, ILBlock* block, bool canOmit)
{
	if (canOmit && (block->GetGlobalIndex() == (m_currentBlock->GetGlobalIndex() + 1)))
	{
		// The destination block is the one just after the current one, just fall through
		return;
	}

	uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
	buffer[0] = 0xeb;
	buffer[1] = 0;
	out->FinishWrite(2);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_8;
	reloc.overflow = UnconditionalJumpOverflowHandler;
	reloc.instruction = out->len - 2;
	reloc.offset = out->len - 1;
	reloc.target = block;
	out->relocs.push_back(reloc);
}


bool OUTPUT_CLASS_NAME::GenerateAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GenerateAddressOf(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src, temp;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (src.type != OPERANDREF_MEM)
		return false;

	if (dest.type == OPERANDREF_REG)
	{
#ifdef OUTPUT32
		EMIT_RM(lea_32, dest.reg, X86_MEM_REF(src.mem));
#else
		EMIT_RM(lea_64, dest.reg, X86_MEM_REF(src.mem));
#endif
		return true;
	}

	temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
	temp.width = 4;
#else
	temp.width = 8;
#endif
	temp.reg = AllocateTemporaryRegister(out, temp.width);

#ifdef OUTPUT32
	EMIT_RM(lea_32, temp.reg, X86_MEM_REF(src.mem));
#else
	EMIT_RM(lea_64, temp.reg, X86_MEM_REF(src.mem));
#endif

	return Move(out, dest, temp);
}


bool OUTPUT_CLASS_NAME::GenerateAddressOfMember(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (src.type != OPERANDREF_MEM)
		return false;

	// Load pointer into a register
	OperandReference temp;
	temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
	temp.width = 4;
#else
	temp.width = 8;
#endif
	temp.reg = AllocateTemporaryRegister(out, temp.width);
	if (!Move(out, temp, src))
		return false;
	src = temp;

	if (!instr.params[2].structure)
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = dest.width;
	deref.mem.base = src.reg;
	deref.mem.scale = 1;
	deref.mem.index = NONE;
	deref.mem.offset = member->offset;

	temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
	temp.width = 4;
#else
	temp.width = 8;
#endif
	temp.reg = AllocateTemporaryRegister(out, temp.width);

#ifdef OUTPUT32
	EMIT_RM(lea_32, temp.reg, X86_MEM_REF(deref.mem));
#else
	EMIT_RM(lea_64, temp.reg, X86_MEM_REF(deref.mem));
#endif

	return Move(out, dest, temp);
}


bool OUTPUT_CLASS_NAME::GenerateDeref(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	if (src.type != OPERANDREF_REG)
	{
		// Load pointer into a register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
		temp.width = 4;
#else
		temp.width = 8;
#endif
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, src))
			return false;
		src = temp;
	}

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = dest.width;
	deref.mem.base = src.reg;
	deref.mem.scale = 1;
	deref.mem.index = NONE;
	deref.mem.offset = 0;
	return Move(out, dest, deref);
}


bool OUTPUT_CLASS_NAME::GenerateDerefMember(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	if (src.type != OPERANDREF_REG)
	{
		// Load pointer into a register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
		temp.width = 4;
#else
		temp.width = 8;
#endif
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, src))
			return false;
		src = temp;
	}

	if (!instr.params[2].structure)
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = dest.width;
	deref.mem.base = src.reg;
	deref.mem.scale = 1;
	deref.mem.index = NONE;
	deref.mem.offset = member->offset;
	return Move(out, dest, deref);
}


bool OUTPUT_CLASS_NAME::GenerateDerefAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareLoad(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	if (dest.type != OPERANDREF_REG)
	{
		// Load pointer into a register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
		temp.width = 4;
#else
		temp.width = 8;
#endif
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, dest))
			return false;
		dest = temp;
	}

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = src.width;
	deref.mem.base = dest.reg;
	deref.mem.scale = 1;
	deref.mem.index = NONE;
	deref.mem.offset = 0;
	return Move(out, deref, src);
}


bool OUTPUT_CLASS_NAME::GenerateDerefMemberAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareLoad(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[2], src))
		return false;

	if (dest.type != OPERANDREF_REG)
	{
		// Load pointer into a register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
		temp.width = 4;
#else
		temp.width = 8;
#endif
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, dest))
			return false;
		dest = temp;
	}

	if (!instr.params[1].structure)
		return false;

	const StructMember* member = instr.params[1].structure->GetMember(instr.params[1].stringValue);
	if (!member)
		return false;

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = src.width;
	deref.mem.base = dest.reg;
	deref.mem.scale = 1;
	deref.mem.index = NONE;
	deref.mem.offset = member->offset;
	return Move(out, deref, src);
}


bool OUTPUT_CLASS_NAME::GenerateArrayIndex(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src, index;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (!PrepareLoad(out, instr.params[2], index))
		return false;
	if (src.type != OPERANDREF_MEM)
		return false;
	if (src.mem.index != NONE)
		return false;

	src.width = (size_t)instr.params[3].integerValue;

	if (index.type == OPERANDREF_IMMED)
		src.mem.offset += src.width * index.immed;
	else if ((src.width == 1) || (src.width == 2) || (src.width == 4) || (src.width == 8))
	{
		if ((index.type != OPERANDREF_REG) || (!IsValidIndexRegister(index.reg)))
		{
			// Load index into a register
			OperandReference temp;
			temp.type = OPERANDREF_REG;
			temp.width = index.width;
			temp.reg = AllocateTemporaryRegister(out, temp.width, USAGE_INDEX);
			if (!Move(out, temp, index))
				return false;
			index = temp;
		}

		src.mem.scale = src.width;
		src.mem.index = index.reg;
	}
	else
	{
		return false;
	}

	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GenerateArrayIndexAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src, index;
	if (!PrepareLoad(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], index))
		return false;
	if (!PrepareLoad(out, instr.params[3], src))
		return false;
	if (dest.type != OPERANDREF_MEM)
		return false;
	if (dest.mem.index != NONE)
		return false;

	dest.width = (size_t)instr.params[2].integerValue;

	if (index.type == OPERANDREF_IMMED)
		dest.mem.offset += src.width * index.immed;
	else if ((dest.width == 1) || (dest.width == 2) || (dest.width == 4) || (dest.width == 8))
	{
		if ((index.type != OPERANDREF_REG) || (!IsValidIndexRegister(index.reg)))
		{
			// Load index into a register
			OperandReference temp;
			temp.type = OPERANDREF_REG;
			temp.width = index.width;
			temp.reg = AllocateTemporaryRegister(out, temp.width, USAGE_INDEX);
			if (!Move(out, temp, index))
				return false;
			index = temp;
		}

		dest.mem.scale = dest.width;
		dest.mem.index = index.reg;
	}
	else
	{
		return false;
	}

	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GeneratePtrAdd(OutputBlock* out, const ILInstruction& instr)
{
	size_t width = (size_t)instr.params[3].integerValue;
	if (width == 1)
		return GenerateAdd(out, instr);
	if ((width != 2) && (width != 4) && (width != 8))
		return false;

	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	if (b.type == OPERANDREF_MEM)
	{
		// Load count into register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = b.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}
	else if (b.type == OPERANDREF_IMMED)
	{
		// Immediate count
		b.immed *= width;
		if (!Move(out, dest, a))
			return false;
		return Add(out, dest, b);
	}

	if ((a.type != OPERANDREF_REG) || (!IsValidIndexRegister(a.reg)))
	{
		// Load pointer into register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width, USAGE_INDEX);
		if (!Move(out, temp, a))
			return false;
		a = temp;
	}

	if (dest.type == OPERANDREF_REG)
	{
#ifdef OUTPUT32
		EMIT_RM(lea_32, dest.reg, X86_MEM_INDEX(a.reg, b.reg, width, 0));
#else
		EMIT_RM(lea_64, dest.reg, X86_MEM_INDEX(a.reg, b.reg, width, 0));
#endif
	}
	else
	{
		OperandType temp = AllocateTemporaryRegister(out, dest.width);
#ifdef OUTPUT32
		EMIT_RM(lea_32, temp, X86_MEM_INDEX(a.reg, b.reg, width, 0));
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp);
#else
		EMIT_RM(lea_64, temp, X86_MEM_INDEX(a.reg, b.reg, width, 0));
		EMIT_MR(mov_64, X86_MEM_REF(dest.mem), temp);
#endif
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GeneratePtrSub(OutputBlock* out, const ILInstruction& instr)
{
	size_t width = (size_t)instr.params[3].integerValue;
	if (width == 1)
		return GenerateSub(out, instr);
	if ((width != 2) && (width != 4) && (width != 8))
		return false;

	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		// Immediate count
		b.immed = -(b.immed * width);
		if (!Move(out, dest, a))
			return false;
		return Add(out, dest, b);
	}

	// Load count into temporary register and negate
	OperandReference countTemp;
	countTemp.type = OPERANDREF_REG;
	countTemp.width = b.width;
	countTemp.reg = AllocateTemporaryRegister(out, countTemp.width, USAGE_INDEX);
	if (!Move(out, countTemp, b))
		return false;
	b = countTemp;

#ifdef OUTPUT32
	EMIT_R(neg_32, b.reg);
#else
	EMIT_R(neg_64, b.reg);
#endif

	if (a.type != OPERANDREF_REG)
	{
		// Load pointer into register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, a))
			return false;
		a = temp;
	}

	if (dest.type == OPERANDREF_REG)
	{
#ifdef OUTPUT32
		EMIT_RM(lea_32, dest.reg, X86_MEM_INDEX(a.reg, b.reg, width, 0));
#else
		EMIT_RM(lea_64, dest.reg, X86_MEM_INDEX(a.reg, b.reg, width, 0));
#endif
	}
	else
	{
		OperandType temp = AllocateTemporaryRegister(out, dest.width);
#ifdef OUTPUT32
		EMIT_RM(lea_32, temp, X86_MEM_INDEX(a.reg, b.reg, width, 0));
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp);
#else
		EMIT_RM(lea_64, temp, X86_MEM_INDEX(a.reg, b.reg, width, 0));
		EMIT_MR(mov_64, X86_MEM_REF(dest.mem), temp);
#endif
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GeneratePtrDiff(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;
	if (!Sub(out, dest, b))
		return false;

	size_t width = (size_t)instr.params[3].integerValue;
	if (width & (width - 1))
	{
		// Elements are not a power of two in size, must divide
		return false;
	}

	size_t count = 0;
	for (; width > 1; count++)
		width >>= 1;

	OperandReference countRef;
	countRef.type = OPERANDREF_IMMED;
	countRef.width = 1;
	countRef.immed = count;
	return ShiftRightSigned(out, dest, countRef);
}


bool OUTPUT_CLASS_NAME::GenerateAdd(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;
	return Add(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GenerateSub(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;
	return Sub(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GenerateSignedMult(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

#ifdef OUTPUT32
	if (a.width == 8)
		return false;
#endif

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = GetRegisterOfSize(REG_EAX, eax.width);
	if (!Move(out, eax, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			EMIT_R(imul_8, b.reg);
		else
			EMIT_M(imul_8, X86_MEM_REF(b.mem));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			EMIT_R(imul_16, b.reg);
		else
			EMIT_M(imul_16, X86_MEM_REF(b.mem));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			EMIT_R(imul_32, b.reg);
		else
			EMIT_M(imul_32, X86_MEM_REF(b.mem));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			EMIT_R(imul_64, b.reg);
		else
			EMIT_M(imul_64, X86_MEM_REF(b.mem));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedMult(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

#ifdef OUTPUT32
	if (a.width == 8)
		return false;
#endif

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = GetRegisterOfSize(REG_EAX, eax.width);
	if (!Move(out, eax, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			EMIT_R(mul_8, b.reg);
		else
			EMIT_M(mul_8, X86_MEM_REF(b.mem));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			EMIT_R(mul_16, b.reg);
		else
			EMIT_M(mul_16, X86_MEM_REF(b.mem));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			EMIT_R(mul_32, b.reg);
		else
			EMIT_M(mul_32, X86_MEM_REF(b.mem));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			EMIT_R(mul_64, b.reg);
		else
			EMIT_M(mul_64, X86_MEM_REF(b.mem));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateSignedDiv(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

#ifdef OUTPUT32
	if (a.width == 8)
		return false;
#endif

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = GetRegisterOfSize(REG_EAX, eax.width);
	if (!Move(out, eax, a))
		return false;

	switch (a.width)
	{
	case 1:
		EMIT_RR(movsx_16_8, REG_AX, REG_AL);
		break;
	case 2:
		EMIT(cwd);
		break;
	case 4:
		EMIT(cdq);
		break;
#ifdef OUTPUT64
	case 8:
		EMIT(cqo);
		break;
#endif
	default:
		return false;
	}

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_8, b.reg);
		else
			EMIT_M(idiv_8, X86_MEM_REF(b.mem));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_16, b.reg);
		else
			EMIT_M(idiv_16, X86_MEM_REF(b.mem));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_32, b.reg);
		else
			EMIT_M(idiv_32, X86_MEM_REF(b.mem));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_64, b.reg);
		else
			EMIT_M(idiv_64, X86_MEM_REF(b.mem));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedDiv(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	switch (a.width)
	{
	case 1:
		EMIT_RR(xor_8, REG_AH, REG_AH);
		break;
	case 2:
		EMIT_RR(xor_16, REG_DX, REG_DX);
		break;
	case 4:
		EMIT_RR(xor_32, REG_EDX, REG_EDX);
		break;
#ifdef OUTPUT64
	case 8:
		EMIT_RR(xor_64, REG_RDX, REG_RDX);
		break;
#endif
	default:
		return false;
	}

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = GetRegisterOfSize(REG_EAX, eax.width);
	if (!Move(out, eax, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_8, b.reg);
		else
			EMIT_M(div_8, X86_MEM_REF(b.mem));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_16, b.reg);
		else
			EMIT_M(div_16, X86_MEM_REF(b.mem));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_32, b.reg);
		else
			EMIT_M(div_32, X86_MEM_REF(b.mem));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_64, b.reg);
		else
			EMIT_M(div_64, X86_MEM_REF(b.mem));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateSignedMod(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

#ifdef OUTPUT32
	if (a.width == 8)
		return false;
#endif

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = GetRegisterOfSize(REG_EAX, eax.width);
	if (!Move(out, eax, a))
		return false;

	OperandReference edx;
	edx.type = OPERANDREF_REG;
	edx.width = a.width;

	switch (a.width)
	{
	case 1:
		edx.reg = REG_AH;
		EMIT_RR(movsx_16_8, REG_AX, REG_AL);
		break;
	case 2:
		edx.reg = REG_DX;
		EMIT(cwd);
		break;
	case 4:
		edx.reg = REG_EDX;
		EMIT(cdq);
		break;
#ifdef OUTPUT64
	case 8:
		edx.reg = REG_RDX;
		EMIT(cqo);
		break;
#endif
	default:
		return false;
	}

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_8, b.reg);
		else
			EMIT_M(idiv_8, X86_MEM_REF(b.mem));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_16, b.reg);
		else
			EMIT_M(idiv_16, X86_MEM_REF(b.mem));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_32, b.reg);
		else
			EMIT_M(idiv_32, X86_MEM_REF(b.mem));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			EMIT_R(idiv_64, b.reg);
		else
			EMIT_M(idiv_64, X86_MEM_REF(b.mem));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, edx);
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedMod(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	OperandReference edx;
	edx.type = OPERANDREF_REG;
	edx.width = a.width;

	switch (a.width)
	{
	case 1:
		edx.reg = REG_AH;
		EMIT_RR(xor_8, REG_AH, REG_AH);
		break;
	case 2:
		edx.reg = REG_DX;
		EMIT_RR(xor_16, REG_DX, REG_DX);
		break;
	case 4:
		edx.reg = REG_EDX;
		EMIT_RR(xor_32, REG_EDX, REG_EDX);
		break;
#ifdef OUTPUT64
	case 8:
		edx.reg = REG_RDX;
		EMIT_RR(xor_64, REG_RDX, REG_RDX);
		break;
#endif
	default:
		return false;
	}

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = GetRegisterOfSize(REG_EAX, eax.width);
	if (!Move(out, eax, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_8, b.reg);
		else
			EMIT_M(div_8, X86_MEM_REF(b.mem));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_16, b.reg);
		else
			EMIT_M(div_16, X86_MEM_REF(b.mem));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_32, b.reg);
		else
			EMIT_M(div_32, X86_MEM_REF(b.mem));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			EMIT_R(div_64, b.reg);
		else
			EMIT_M(div_64, X86_MEM_REF(b.mem));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, edx);
}


bool OUTPUT_CLASS_NAME::GenerateAnd(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;
	return And(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GenerateOr(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;
	return Or(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GenerateXor(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;
	return Xor(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GenerateShl(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b, count;

	ReserveRegisters(out, REG_ECX, NONE);
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.reg = REG_CL;

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
		return ShiftLeft(out, dest, b);

	if (!Move(out, count, b))
		return false;
	return ShiftLeft(out, dest, count);
}


bool OUTPUT_CLASS_NAME::GenerateShr(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b, count;

	ReserveRegisters(out, REG_ECX, NONE);
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.reg = REG_CL;

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
		return ShiftRightUnsigned(out, dest, b);

	if (!Move(out, count, b))
		return false;
	return ShiftRightUnsigned(out, dest, count);
}


bool OUTPUT_CLASS_NAME::GenerateSar(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b, count;

	ReserveRegisters(out, REG_ECX, NONE);
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.reg = REG_CL;

	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
		return ShiftRightSigned(out, dest, b);

	if (!Move(out, count, b))
		return false;
	return ShiftRightSigned(out, dest, count);
}


bool OUTPUT_CLASS_NAME::GenerateNeg(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (!Move(out, dest, src))
		return false;
	return Neg(out, dest);
}


bool OUTPUT_CLASS_NAME::GenerateNot(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (!Move(out, dest, src))
		return false;
	return Not(out, dest);
}


bool OUTPUT_CLASS_NAME::GenerateIfTrue(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference value;
	if (!PrepareLoad(out, instr.params[0], value))
		return false;
	if (value.width != 1)
		return false;

	if (value.type == OPERANDREF_REG)
		EMIT_RR(test_8, value.reg, value.reg);
	else if (value.type == OPERANDREF_MEM)
		EMIT_MI(cmp_8, X86_MEM_REF(value.mem), 0);
	else if (value.immed)
	{
		UnconditionalJump(out, instr.params[1].block);
		return true;
	}
	else
	{
		UnconditionalJump(out, instr.params[2].block);
		return true;
	}

	ConditionalJump(out, CONDJUMP_NOT_EQUAL, instr.params[1].block, instr.params[2].block);
	return true;
}


#ifdef OUTPUT32
#define IMPLEMENT_ORDERED_COMPARE_64(pred, lowPred, highPred, highInverse) \
	if (left.type == OPERANDREF_REG) \
	{ \
		switch (right.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_RR(cmp_32, left.highReg, right.highReg); \
			ConditionalJump(out, highPred, instr.params[2].block, NULL); \
			ConditionalJump(out, highInverse, instr.params[3].block, NULL); \
			EMIT_RR(cmp_32, left.reg, right.reg); \
			ConditionalJump(out, lowPred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_MEM: \
			EMIT_RM(cmp_32, left.highReg, X86_MEM_REF_OFFSET(right.mem, 4)); \
			ConditionalJump(out, highPred, instr.params[2].block, NULL); \
			ConditionalJump(out, highInverse, instr.params[3].block, NULL); \
			EMIT_RM(cmp_32, left.reg, X86_MEM_REF(right.mem)); \
			ConditionalJump(out, lowPred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_IMMED: \
			EMIT_RI(cmp_32, left.highReg, (uint32_t)(right.immed >> 32)); \
			ConditionalJump(out, highPred, instr.params[2].block, NULL); \
			ConditionalJump(out, highInverse, instr.params[3].block, NULL); \
			EMIT_RI(cmp_32, left.reg, (uint32_t)right.immed); \
			ConditionalJump(out, lowPred, instr.params[2].block, instr.params[3].block); \
			break; \
		default:  return false; \
		} \
	} \
	else \
	{ \
		OperandType temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), right.highReg); \
			ConditionalJump(out, highPred, instr.params[2].block, NULL); \
			ConditionalJump(out, highInverse, instr.params[3].block, NULL); \
			EMIT_MR(cmp_32, X86_MEM_REF(left.mem), right.reg); \
			ConditionalJump(out, lowPred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_MEM: \
			temp = AllocateTemporaryRegister(out, 4); \
			EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(right.mem, 4)); \
			EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), temp); \
			ConditionalJump(out, highPred, instr.params[2].block, NULL); \
			ConditionalJump(out, highInverse, instr.params[3].block, NULL); \
			EMIT_RM(mov_32, temp, X86_MEM_REF(right.mem)); \
			EMIT_MR(cmp_32, X86_MEM_REF(left.mem), temp); \
			ConditionalJump(out, lowPred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_IMMED: \
			EMIT_MI(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), (uint32_t)(right.immed >> 32)); \
			ConditionalJump(out, highPred, instr.params[2].block, NULL); \
			ConditionalJump(out, highInverse, instr.params[3].block, NULL); \
			EMIT_MI(cmp_32, X86_MEM_REF(left.mem), (uint32_t)right.immed); \
			ConditionalJump(out, lowPred, instr.params[2].block, instr.params[3].block); \
			break; \
		default:  return false; \
		} \
	}
#else
#define IMPLEMENT_ORDERED_COMPARE_64(pred, lowPred, highPred, highInverse) \
	if (left.type == OPERANDREF_REG) \
	{ \
		switch (right.type) \
		{ \
		case OPERANDREF_REG:  EMIT_RR(cmp_64, left.reg, right.reg); break; \
		case OPERANDREF_MEM:  EMIT_RM(cmp_64, left.reg, X86_MEM_REF(right.mem)); break; \
		case OPERANDREF_IMMED:  EMIT_RI(cmp_64, left.reg, right.immed); break; \
		default:  return false; \
		} \
		ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
	} \
	else \
	{ \
		OperandType temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(cmp_64, X86_MEM_REF(left.mem), right.reg); break; \
		case OPERANDREF_MEM: \
			temp = AllocateTemporaryRegister(out, 8); \
			EMIT_RM(mov_64, temp, X86_MEM_REF(right.mem)); \
			EMIT_MR(cmp_64, X86_MEM_REF(left.mem), temp); \
			break; \
		case OPERANDREF_IMMED:  EMIT_MI(cmp_64, X86_MEM_REF(left.mem), right.immed); break; \
		default:  return false; \
		} \
		ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
	}
#endif

#ifdef OUTPUT32
#define IMPLEMENT_UNORDERED_COMPARE_64(pred, inverse) \
	if (left.type == OPERANDREF_REG) \
	{ \
		switch (right.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_RR(cmp_32, left.highReg, right.highReg); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_RR(cmp_32, left.reg, right.reg); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_MEM: \
			EMIT_RM(cmp_32, left.highReg, X86_MEM_REF_OFFSET(right.mem, 4)); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_RM(cmp_32, left.reg, X86_MEM_REF(right.mem)); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_IMMED: \
			EMIT_RI(cmp_32, left.highReg, (uint32_t)(right.immed >> 32)); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_RI(cmp_32, left.reg, (uint32_t)right.immed); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		default:  return false; \
		} \
	} \
	else \
	{ \
		OperandType temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), right.highReg); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_MR(cmp_32, X86_MEM_REF(left.mem), right.reg); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_MEM: \
			temp = AllocateTemporaryRegister(out, 4); \
			EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(right.mem, 4)); \
			EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), temp); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_RM(mov_32, temp, X86_MEM_REF(right.mem)); \
			EMIT_MR(cmp_32, X86_MEM_REF(left.mem), temp); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_IMMED: \
			EMIT_MI(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), (uint32_t)(right.immed >> 32)); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_MI(cmp_32, X86_MEM_REF(left.mem), (uint32_t)right.immed); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		default:  return false; \
		} \
	}
#else
#define IMPLEMENT_UNORDERED_COMPARE_64(pred, inverse) \
	if (left.type == OPERANDREF_REG) \
	{ \
		switch (right.type) \
		{ \
		case OPERANDREF_REG:  EMIT_RR(cmp_64, left.reg, right.reg); break; \
		case OPERANDREF_MEM:  EMIT_RM(cmp_64, left.reg, X86_MEM_REF(right.mem)); break; \
		case OPERANDREF_IMMED:  EMIT_RI(cmp_64, left.reg, right.immed); break; \
		default:  return false; \
		} \
		ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
	} \
	else \
	{ \
		OperandType temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(cmp_64, X86_MEM_REF(left.mem), right.reg); break; \
		case OPERANDREF_MEM: \
			temp = AllocateTemporaryRegister(out, 8); \
			EMIT_RM(mov_64, temp, X86_MEM_REF(right.mem)); \
			EMIT_MR(cmp_64, X86_MEM_REF(left.mem), temp); \
			break; \
		case OPERANDREF_IMMED:  EMIT_MI(cmp_64, X86_MEM_REF(left.mem), right.immed); break; \
		default:  return false; \
		} \
		ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
	}
#endif

#define IMPLEMENT_COMPARE(pred) \
	if (left.width == 1) \
	{ \
		if (left.type == OPERANDREF_REG) \
		{ \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(cmp_8, left.reg, right.reg); break; \
			case OPERANDREF_MEM:  EMIT_RM(cmp_8, left.reg, X86_MEM_REF(right.mem)); break; \
			case OPERANDREF_IMMED:  EMIT_RI(cmp_8, left.reg, right.immed); break; \
			default:  return false; \
			} \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
		} \
		else \
		{ \
			OperandType temp; \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(cmp_8, X86_MEM_REF(left.mem), right.reg); break; \
			case OPERANDREF_MEM: \
				temp = AllocateTemporaryRegister(out, 1); \
				EMIT_RM(mov_8, temp, X86_MEM_REF(right.mem)); \
				EMIT_MR(cmp_8, X86_MEM_REF(left.mem), temp); \
				break; \
			case OPERANDREF_IMMED:  EMIT_MI(cmp_8, X86_MEM_REF(left.mem), right.immed); break; \
			default:  return false; \
			} \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
		} \
	} \
	else if (left.width == 2) \
	{ \
		if (left.type == OPERANDREF_REG) \
		{ \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(cmp_16, left.reg, right.reg); break; \
			case OPERANDREF_MEM:  EMIT_RM(cmp_16, left.reg, X86_MEM_REF(right.mem)); break; \
			case OPERANDREF_IMMED:  EMIT_RI(cmp_16, left.reg, right.immed); break; \
			default:  return false; \
			} \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
		} \
		else \
		{ \
			OperandType temp; \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(cmp_16, X86_MEM_REF(left.mem), right.reg); break; \
			case OPERANDREF_MEM: \
				temp = AllocateTemporaryRegister(out, 2); \
				EMIT_RM(mov_16, temp, X86_MEM_REF(right.mem)); \
				EMIT_MR(cmp_16, X86_MEM_REF(left.mem), temp); \
				break; \
			case OPERANDREF_IMMED:  EMIT_MI(cmp_16, X86_MEM_REF(left.mem), right.immed); break; \
			default:  return false; \
			} \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
		} \
	} \
	else if (left.width == 4) \
	{ \
		if (left.type == OPERANDREF_REG) \
		{ \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_RR(cmp_32, left.reg, right.reg); break; \
			case OPERANDREF_MEM:  EMIT_RM(cmp_32, left.reg, X86_MEM_REF(right.mem)); break; \
			case OPERANDREF_IMMED:  EMIT_RI(cmp_32, left.reg, right.immed); break; \
			default:  return false; \
			} \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
		} \
		else \
		{ \
			OperandType temp; \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(cmp_32, X86_MEM_REF(left.mem), right.reg); break; \
			case OPERANDREF_MEM: \
				temp = AllocateTemporaryRegister(out, 4); \
				EMIT_RM(mov_32, temp, X86_MEM_REF(right.mem)); \
				EMIT_MR(cmp_32, X86_MEM_REF(left.mem), temp); \
				break; \
			case OPERANDREF_IMMED:  EMIT_MI(cmp_32, X86_MEM_REF(left.mem), right.immed); break; \
			default:  return false; \
			} \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
		} \
	} \
	else if (left.width == 8) \
	{

#define IMPLEMENT_COMPARE_FINISH() \
	} \
	else \
	{ \
		return false; \
	} \
	return true;

#define IMPLEMENT_ORDERED_COMPARE(pred, lowPred, highPred, highInverse) \
	IMPLEMENT_COMPARE(pred) \
	IMPLEMENT_ORDERED_COMPARE_64(pred, lowPred, highPred, highInverse) \
	IMPLEMENT_COMPARE_FINISH()

#define IMPLEMENT_UNORDERED_COMPARE(pred, inverse) \
	IMPLEMENT_COMPARE(pred) \
	IMPLEMENT_UNORDERED_COMPARE_64(pred, inverse) \
	IMPLEMENT_COMPARE_FINISH()

bool OUTPUT_CLASS_NAME::GenerateIfLessThan(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_LESS_THAN, CONDJUMP_BELOW, CONDJUMP_LESS_THAN, CONDJUMP_GREATER_THAN);
}


bool OUTPUT_CLASS_NAME::GenerateIfLessThanEqual(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_LESS_EQUAL, CONDJUMP_BELOW_EQUAL, CONDJUMP_LESS_THAN, CONDJUMP_GREATER_THAN);
}


bool OUTPUT_CLASS_NAME::GenerateIfBelow(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_BELOW, CONDJUMP_BELOW, CONDJUMP_BELOW, CONDJUMP_ABOVE);
}


bool OUTPUT_CLASS_NAME::GenerateIfBelowEqual(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_BELOW_EQUAL, CONDJUMP_BELOW_EQUAL, CONDJUMP_BELOW, CONDJUMP_ABOVE);
}


bool OUTPUT_CLASS_NAME::GenerateIfEqual(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_UNORDERED_COMPARE(CONDJUMP_EQUAL, CONDJUMP_NOT_EQUAL);
}


bool OUTPUT_CLASS_NAME::GenerateGoto(OutputBlock* out, const ILInstruction& instr)
{
	if (instr.params[0].cls != ILPARAM_BLOCK)
		return false;
	UnconditionalJump(out, instr.params[0].block);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateCall(OutputBlock* out, const ILInstruction& instr)
{
	size_t pushSize = 0;

	// Push parameters from right to left
	for (size_t i = instr.params.size() - 1; i >= 2; i--)
	{
		memset(m_alloc, 0, sizeof(m_alloc));
		ClearReservedRegisters(out);

		OperandReference param;
		if (!PrepareLoad(out, instr.params[i], param))
			return false;

		if (param.width == 0)
		{
			// Indefinite width (used for immediates, for example), use native size
#ifdef OUTPUT32
			param.width = 4;
#else
			param.width = 8;
#endif
		}

		if (!m_normalStack)
		{
#ifdef OUTPUT32
			size_t paramSize = (param.width + 3) & (~3);
			EMIT_RM(lea_32, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? paramSize : -paramSize));
#else
			size_t paramSize = (param.width + 7) & (~7);
			EMIT_RM(lea_64, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? paramSize : -paramSize));
#endif

			OperandReference dest;
			dest.type = OPERANDREF_MEM;
			dest.width = param.width;
			dest.mem.base = m_stackPointer;
			dest.mem.index = NONE;
			dest.mem.scale = 1;
			dest.mem.offset = 0;

			if (!Move(out, dest, param))
				return false;

			pushSize += paramSize;
			continue;
		}

		// Check for native size parameters
#ifdef OUTPUT32
		if (param.width == 4)
		{
			switch (param.type)
			{
			case OPERANDREF_REG:
				EMIT_R(push, param.reg);
				break;
			case OPERANDREF_MEM:
				EMIT_M(push, X86_MEM_REF(param.mem));
				break;
			case OPERANDREF_IMMED:
				EMIT_I(push, (int32_t)param.immed);
				break;
			default:
				return false;
			}

			pushSize += 4;
			continue;
		}
		else if (param.width == 8)
		{
			switch (param.type)
			{
			case OPERANDREF_REG:
				EMIT_R(push, param.highReg);
				EMIT_R(push, param.reg);
				break;
			case OPERANDREF_MEM:
				EMIT_M(push, X86_MEM_REF_OFFSET(param.mem, 4));
				EMIT_M(push, X86_MEM_REF(param.mem));
				break;
			case OPERANDREF_IMMED:
				EMIT_I(push, (int32_t)(param.immed >> 32));
				EMIT_I(push, (int32_t)param.immed);
				break;
			default:
				return false;
			}

			pushSize += 8;
			continue;
		}
#else
		if (param.width == 8)
		{
			switch (param.type)
			{
			case OPERANDREF_REG:
				EMIT_R(push, param.reg);
				break;
			case OPERANDREF_MEM:
				EMIT_M(push, X86_MEM_REF(param.mem));
				break;
			case OPERANDREF_IMMED:
				if ((param.immed < -0x80000000LL) || (param.immed >= 0x80000000LL))
				{
					// Immediate out of range for single instruction push
					OperandType reg = AllocateTemporaryRegister(out, 8);
					EMIT_RI(mov_64, reg, param.immed);
					EMIT_R(push, reg);
				}
				else
				{
					EMIT_I(push, (int32_t)param.immed);
				}
				break;
			default:
				return false;
			}

			pushSize += 8;
			continue;
		}
#endif

		// Not native size
		if (param.type == OPERANDREF_REG)
		{
			// Push native size (upper bits are ignored)
#ifdef OUTPUT32
			OperandType pushReg = GetRegisterOfSize(param.reg, 4);
#else
			OperandType pushReg = GetRegisterOfSize(param.reg, 8);
#endif
			EMIT_R(push, pushReg);
		}
		else if (param.type == OPERANDREF_IMMED)
		{
			EMIT_I(push, (int32_t)param.immed);
		}
		else
		{
			// Load into register and then push native size
			OperandReference temp;
			temp.type = OPERANDREF_REG;
			temp.width = param.width;
			temp.reg = AllocateTemporaryRegister(out, param.width);
			if (!Move(out, temp, param))
				return false;

#ifdef OUTPUT32
			OperandType pushReg = GetRegisterOfSize(temp.reg, 4);
#else
			OperandType pushReg = GetRegisterOfSize(temp.reg, 8);
#endif
			EMIT_R(push, pushReg);
		}

#ifdef OUTPUT32
		pushSize += 4;
#else
		pushSize += 8;
#endif
	}

	memset(m_alloc, 0, sizeof(m_alloc));
	ClearReservedRegisters(out);

	// Perform function call
	if (instr.params[1].cls == ILPARAM_FUNC)
	{
		// Direct function call
		if (m_settings.encodePointers || (!m_normalStack))
		{
			// Encoded pointer call or call with alternate stack pointer, push
			// return address then jump to function
			OperandReference retAddr;
			retAddr.type = OPERANDREF_REG;
#ifdef OUTPUT32
			retAddr.width = 4;
#else
			retAddr.width = 8;
#endif
			retAddr.reg = AllocateTemporaryRegister(out, retAddr.width);

			// Generate code to get return address and add a relocation for it
#ifdef OUTPUT32
			Relocation reloc;
			if (m_basePointer != NONE)
			{
				size_t leaOffset = out->len;
				EMIT_RM(lea_32, retAddr.reg, X86_MEM(m_basePointer, 1));
				*(int8_t*)((size_t)out->code + out->len - 1) = 0;
				reloc.type = CODE_RELOC_BASE_RELATIVE_8;
				reloc.overflow = BaseRelativeLeaOverflowHandler;
				reloc.instruction = leaOffset;
				reloc.offset = out->len - 1;
				reloc.target = NULL;
			}
			else if (m_settings.positionIndependent)
			{
				GetInstructionPointer(out, retAddr.reg);
				reloc.type = CODE_RELOC_RELATIVE_8;
				reloc.offset = out->len - 1;
				reloc.target = NULL;
			}
			else
			{
				EMIT_RI(mov_32, retAddr.reg, 0);
				reloc.type = CODE_RELOC_ABSOLUTE_32;
				reloc.offset = out->len - 4;
				reloc.target = NULL;
			}
#else
			EMIT_RM(lea_64, retAddr.reg, X86_MEM(REG_RIP, 0));

			Relocation reloc;
			reloc.type = CODE_RELOC_RELATIVE_32;
			reloc.offset = out->len - 4;
			reloc.target = NULL;
#endif

			size_t beforeLen = out->len;

			if (m_settings.encodePointers)
			{
				// Generate code to encode pointer
				ILParameter keyParam(m_settings.encodePointerKey);
				OperandReference key;
				if (!PrepareLoad(out, keyParam, key))
					return false;
				if (!Xor(out, retAddr, key))
					return false;
			}

			// Push return address and jump to function
			if (m_normalStack)
				EMIT_R(push, retAddr.reg);
			else
			{
#ifdef OUTPUT32
				EMIT_RM(lea_32, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 4 : -4));
				EMIT_MR(mov_32, X86_MEM(m_stackPointer, 0), retAddr.reg);
#else
				EMIT_RM(lea_64, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 8 : -8));
				EMIT_MR(mov_64, X86_MEM(m_stackPointer, 0), retAddr.reg);
#endif
			}

			UnconditionalJump(out, instr.params[1].function->GetIL()[0], false);

			// Fix up relocation to point to return address
			size_t afterLen = out->len;
			reloc.start = beforeLen;
			reloc.end = afterLen;
#ifdef OUTPUT32
			if (m_settings.positionIndependent)
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
			reloc.target = instr.params[1].function->GetIL()[0];
			out->relocs.push_back(reloc);
		}
	}
	else
	{
		// Indirect function call
		OperandReference func, key;
		if (!PrepareLoad(out, instr.params[1], func))
			return false;

		if (m_settings.encodePointers)
		{
			// Decode pointer before calling
			ILParameter keyParam(m_settings.encodePointerKey);
			if (!PrepareLoad(out, keyParam, key))
				return false;

			OperandReference temp;
			temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
			temp.width = 4;
#else
			temp.width = 8;
#endif
			temp.reg = AllocateTemporaryRegister(out, temp.width);
			if (!Move(out, temp, func))
				return false;
			if (!Xor(out, temp, key))
				return false;

			func = temp;
		}

		if (m_settings.encodePointers || (!m_normalStack))
		{
			// Encoded pointer call or call with alternate stack pointer, push
			// return address then jump to function
			OperandReference retAddr;
			retAddr.type = OPERANDREF_REG;
#ifdef OUTPUT32
			retAddr.width = 4;
#else
			retAddr.width = 8;
#endif
			retAddr.reg = AllocateTemporaryRegister(out, retAddr.width);

			// Generate code to get return address and add a relocation for it
#ifdef OUTPUT32
			Relocation reloc;
			if (m_basePointer != NONE)
			{
				size_t leaOffset = out->len;
				EMIT_RM(lea_32, retAddr.reg, X86_MEM(m_basePointer, 1));
				*(int8_t*)((size_t)out->code + out->len - 1) = 0;
				reloc.type = CODE_RELOC_BASE_RELATIVE_8;
				reloc.overflow = BaseRelativeLeaOverflowHandler;
				reloc.instruction = leaOffset;
				reloc.offset = out->len - 1;
				reloc.target = NULL;
			}
			else if (m_settings.positionIndependent)
			{
				GetInstructionPointer(out, retAddr.reg);
				reloc.type = CODE_RELOC_RELATIVE_8;
				reloc.offset = out->len - 1;
				reloc.target = NULL;
			}
			else
			{
				EMIT_RI(mov_32, retAddr.reg, 0);
				reloc.type = CODE_RELOC_ABSOLUTE_32;
				reloc.offset = out->len - 4;
				reloc.target = NULL;
			}
#else
			EMIT_RM(lea_64, retAddr.reg, X86_MEM(REG_RIP, 0));

			Relocation reloc;
			reloc.type = CODE_RELOC_RELATIVE_32;
			reloc.offset = out->len - 4;
			reloc.target = NULL;
#endif

			size_t beforeLen = out->len;

			if (m_settings.encodePointers)
			{
				// Encode return address
				if (!Xor(out, retAddr, key))
					return false;
			}

			// Push return address and jump to destination
			if (m_stackPointer == m_normalStack)
				EMIT_R(push, retAddr.reg);
			else
			{
#ifdef OUTPUT32
				EMIT_RM(lea_32, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 4 : -4));
				EMIT_MR(mov_32, X86_MEM(m_stackPointer, 0), retAddr.reg);
#else
				EMIT_RM(lea_64, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 8 : -8));
				EMIT_MR(mov_64, X86_MEM(m_stackPointer, 0), retAddr.reg);
#endif
			}

			switch (func.type)
			{
			case OPERANDREF_REG:
				EMIT_R(jmpn, func.reg);
				break;
			case OPERANDREF_MEM:
				EMIT_M(jmpn, X86_MEM_REF(func.mem));
				break;
			default:
				return false;
			}

			// Fix up relocation to point to return address
			size_t afterLen = out->len;
			reloc.start = beforeLen;
			reloc.end = afterLen;
#ifdef OUTPUT32
			if (m_settings.positionIndependent)
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
			switch (func.type)
			{
			case OPERANDREF_REG:
				EMIT_R(calln, func.reg);
				break;
			case OPERANDREF_MEM:
				EMIT_M(calln, X86_MEM_REF(func.mem));
				break;
			default:
				return false;
			}
		}
	}

	// Adjust stack pointer to pop off parameters
	if (pushSize != 0)
	{
		if (m_settings.stackGrowsUp)
		{
#ifdef OUTPUT32
			EMIT_RI(sub_32, m_stackPointer, pushSize);
#else
			EMIT_RI(sub_64, m_stackPointer, pushSize);
#endif
		}
		else
		{
#ifdef OUTPUT32
			EMIT_RI(add_32, m_stackPointer, pushSize);
#else
			EMIT_RI(add_64, m_stackPointer, pushSize);
#endif
		}
	}

	// Store return value, if there is one
	if (instr.params[0].cls != ILPARAM_VOID)
	{
#ifdef OUTPUT32
		if (instr.params[0].GetWidth() > 4)
			ReserveRegisters(out, REG_EAX, REG_EDX, NONE);
		else
			ReserveRegisters(out, REG_EAX, NONE);
#else
		ReserveRegisters(out, REG_EAX, NONE);
#endif

		OperandReference dest, retVal;
		if (!PrepareStore(out, instr.params[0], dest))
			return false;

		retVal.type = OPERANDREF_REG;
		retVal.width = dest.width;
#ifdef OUTPUT32
		if (retVal.width > 4)
		{
			retVal.reg = REG_EAX;
			retVal.highReg = REG_EDX;
		}
		else
		{
			retVal.reg = GetRegisterOfSize(REG_EAX, retVal.width);
		}
#else
		retVal.reg = GetRegisterOfSize(REG_EAX, retVal.width);
#endif
		if (!Move(out, dest, retVal))
			return false;
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateSignedConvert(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	if (dest.width == src.width)
		return Move(out, dest, src);

	if (dest.width == 1)
	{
		src.width = 1;
		if (src.type == OPERANDREF_REG)
		{
			src.reg = GetRegisterOfSize(src.reg, 1);
			if (!IsRegisterValid(src.reg))
			{
				// Source register cannot be referenced as a single byte, must go
				// through a temporary to make this work, or transfer more than
				// necessary if the destination is also a register
				if (dest.type == OPERANDREF_REG)
				{
					// Destination is a register, transfer all four bytes of the register
					src.width = 4;
					src.reg = GetRegisterOfSize(src.reg, 4);
					dest.width = 4;
					dest.reg = GetRegisterOfSize(dest.reg, 4);
				}
				else
				{
					// Destination is not a register, must go through a temporary
					OperandReference tmp;
					tmp.type = OPERANDREF_REG;
					tmp.width = 4;
					tmp.reg = GetRegisterOfSize(AllocateTemporaryRegister(out, 1), 4);
					src.width = 4;
					src.reg = GetRegisterOfSize(src.reg, 4);
					if (!Move(out, tmp, src))
						return false;
					tmp.width = 1;
					tmp.reg = GetRegisterOfSize(tmp.reg, 1);
					return Move(out, dest, tmp);
				}
			}
		}

		return Move(out, dest, src);
	}
	else if (dest.width == 2)
	{
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_16_8, dest.reg, src.reg);
				else
					EMIT_RM(movsx_16_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 2);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_16_8, tmp, src.reg);
				else
					EMIT_RM(movsx_16_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_16, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else
		{
			src.width = 2;
			if (src.type == OPERANDREF_REG)
				src.reg = GetRegisterOfSize(src.reg, 2);
			return Move(out, dest, src);
		}
	}
	else if (dest.width == 4)
	{
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_8, dest.reg, src.reg);
				else
					EMIT_RM(movsx_32_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_8, tmp, src.reg);
				else
					EMIT_RM(movsx_32_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_16, dest.reg, src.reg);
				else
					EMIT_RM(movsx_32_16, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_16, tmp, src.reg);
				else
					EMIT_RM(movsx_32_16, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else
		{
			src.width = 4;
			if (src.type == OPERANDREF_REG)
				src.reg = GetRegisterOfSize(src.reg, 4);
			return Move(out, dest, src);
		}
	}
	else if (dest.width == 8)
	{
#ifdef OUTPUT32
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_8, dest.reg, src.reg);
				else
					EMIT_RM(movsx_32_8, dest.reg, X86_MEM_REF(src.mem));
				if ((dest.reg == REG_EAX) && (dest.highReg == REG_EDX))
					EMIT(cdq);
				else
				{
					EMIT_RR(mov_32, dest.highReg, dest.reg);
					EMIT_RI(sar_32, dest.highReg, 31);
				}
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_8, tmp, src.reg);
				else
					EMIT_RM(movsx_32_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				EMIT_RI(sar_32, tmp, 31);
				EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), tmp);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_16, dest.reg, src.reg);
				else
					EMIT_RM(movsx_32_16, dest.reg, X86_MEM_REF(src.mem));
				if ((dest.reg == REG_EAX) && (dest.highReg == REG_EDX))
					EMIT(cdq);
				else
				{
					EMIT_RR(mov_32, dest.highReg, dest.reg);
					EMIT_RI(sar_32, dest.highReg, 31);
				}
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_32_16, tmp, src.reg);
				else
					EMIT_RM(movsx_32_16, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				EMIT_RI(sar_32, tmp, 31);
				EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), tmp);
				return true;
			}
		}
		else if (src.width == 4)
		{
			if (dest.type == OPERANDREF_REG)
			{
				dest.width = 4;
				if (!Move(out, dest, src))
					return false;
				if ((dest.reg == REG_EAX) && (dest.highReg == REG_EDX))
					EMIT(cdq);
				else
				{
					EMIT_RR(mov_32, dest.highReg, dest.reg);
					EMIT_RI(sar_32, dest.highReg, 31);
				}
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(mov_32, tmp, src.reg);
				else
					EMIT_RM(mov_32, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				EMIT_RI(sar_32, tmp, 31);
				EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), tmp);
				return true;
			}
		}
		else
		{
			return Move(out, dest, src);
		}
#else
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_64_8, dest.reg, src.reg);
				else
					EMIT_RM(movsx_64_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 8);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_64_8, tmp, src.reg);
				else
					EMIT_RM(movsx_64_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_64_16, dest.reg, src.reg);
				else
					EMIT_RM(movsx_64_16, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 8);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsx_64_16, tmp, src.reg);
				else
					EMIT_RM(movsx_64_16, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 4)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsxd_64_32, dest.reg, src.reg);
				else
					EMIT_RM(movsxd_64_32, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 8);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movsxd_64_32, tmp, src.reg);
				else
					EMIT_RM(movsxd_64_32, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else
		{
			return Move(out, dest, src);
		}
#endif
	}
	else
	{
		return false;
	}
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedConvert(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	if (dest.width == src.width)
		return Move(out, dest, src);

	if (dest.width == 1)
	{
		src.width = 1;
		if (src.type == OPERANDREF_REG)
		{
			src.reg = GetRegisterOfSize(src.reg, 1);
			if (!IsRegisterValid(src.reg))
			{
				// Source register cannot be referenced as a single byte, must go
				// through a temporary to make this work, or transfer more than
				// necessary if the destination is also a register
				if (dest.type == OPERANDREF_REG)
				{
					// Destination is a register, transfer all four bytes of the register
					src.width = 4;
					src.reg = GetRegisterOfSize(src.reg, 4);
					dest.width = 4;
					dest.reg = GetRegisterOfSize(dest.reg, 4);
				}
				else
				{
					// Destination is not a register, must go through a temporary
					OperandReference tmp;
					tmp.type = OPERANDREF_REG;
					tmp.width = 4;
					tmp.reg = GetRegisterOfSize(AllocateTemporaryRegister(out, 1), 4);
					src.width = 4;
					src.reg = GetRegisterOfSize(src.reg, 4);
					if (!Move(out, tmp, src))
						return false;
					tmp.width = 1;
					tmp.reg = GetRegisterOfSize(tmp.reg, 1);
					return Move(out, dest, tmp);
				}
			}
		}

		return Move(out, dest, src);
	}
	else if (dest.width == 2)
	{
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_16_8, dest.reg, src.reg);
				else
					EMIT_RM(movzx_16_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 2);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_16_8, tmp, src.reg);
				else
					EMIT_RM(movzx_16_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_16, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else
		{
			src.width = 2;
			if (src.type == OPERANDREF_REG)
				src.reg = GetRegisterOfSize(src.reg, 2);
			return Move(out, dest, src);
		}
	}
	else if (dest.width == 4)
	{
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, tmp, src.reg);
				else
					EMIT_RM(movzx_32_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_16, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, tmp, src.reg);
				else
					EMIT_RM(movzx_32_16, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else
		{
			src.width = 4;
			if (src.type == OPERANDREF_REG)
				src.reg = GetRegisterOfSize(src.reg, 4);
			return Move(out, dest, src);
		}
	}
	else if (dest.width == 8)
	{
#ifdef OUTPUT32
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_8, dest.reg, X86_MEM_REF(src.mem));
				EMIT_RR(xor_32, dest.highReg, dest.highReg);
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, tmp, src.reg);
				else
					EMIT_RM(movzx_32_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				EMIT_MI(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_16, dest.reg, X86_MEM_REF(src.mem));
				EMIT_RR(xor_32, dest.highReg, dest.highReg);
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, tmp, src.reg);
				else
					EMIT_RM(movzx_32_16, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				EMIT_MI(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
				return true;
			}
		}
		else if (src.width == 4)
		{
			if (dest.type == OPERANDREF_REG)
				EMIT_RR(xor_32, dest.highReg, dest.highReg);
			else
				EMIT_MI(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
			return true;
		}
		else
		{
			return Move(out, dest, src);
		}
#else
		if (src.width == 1)
		{
			if (dest.type == OPERANDREF_REG)
			{
				dest.width = 4;
				dest.reg = GetRegisterOfSize(dest.reg, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, tmp, src.reg);
				else
					EMIT_RM(movzx_32_8, tmp, X86_MEM_REF(src.mem));
				tmp = GetRegisterOfSize(tmp, 8);
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				dest.width = 4;
				dest.reg = GetRegisterOfSize(dest.reg, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_16, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				OperandType tmp = AllocateTemporaryRegister(out, 4);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, tmp, src.reg);
				else
					EMIT_RM(movzx_32_16, tmp, X86_MEM_REF(src.mem));
				tmp = GetRegisterOfSize(tmp, 8);
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 4)
		{
			if (dest.type == OPERANDREF_REG)
			{
				dest.width = 4;
				if (dest.type == OPERANDREF_REG)
					dest.reg = GetRegisterOfSize(dest.reg, 4);
				return Move(out, dest, src);
			}
			else
			{
				src.width = 8;
				if (src.type == OPERANDREF_REG)
					src.reg = GetRegisterOfSize(src.reg, 8);
				return Move(out, dest, src);
			}
		}
		else
		{
			return Move(out, dest, src);
		}
#endif
	}
	else
	{
		return false;
	}
}


bool OUTPUT_CLASS_NAME::GenerateReturn(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	dest.type = OPERANDREF_REG;
	dest.width = instr.params[0].GetWidth();
#ifdef OUTPUT32
	if (dest.width == 8)
	{
		dest.reg = REG_EAX;
		dest.highReg = REG_EDX;
	}
	else
	{
		dest.reg = GetRegisterOfSize(REG_EAX, dest.width);
	}
#else
	dest.reg = GetRegisterOfSize(REG_RAX, dest.width);
#endif

	if (!PrepareLoad(out, instr.params[0], src))
		return false;
	if (!Move(out, dest, src))
		return false;
	return GenerateReturnVoid(out, instr);
}


bool OUTPUT_CLASS_NAME::GenerateReturnVoid(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);
#ifdef OUTPUT32
	OperandType temp = AllocateTemporaryRegister(out, 4);
#else
	OperandType temp = AllocateTemporaryRegister(out, 8);
#endif

	if (m_settings.encodePointers)
	{
		// Using encoded pointers, load decode key into ECX
		ILParameter keyParam(m_settings.encodePointerKey);
		OperandReference tempRef, key;

		if (!PrepareLoad(out, keyParam, key))
			return false;

		tempRef.type = OPERANDREF_REG;
#ifdef OUTPUT32
		tempRef.width = 4;
#else
		tempRef.width = 8;
#endif
		tempRef.reg = GetRegisterOfSize(temp, tempRef.width);
		if (!Move(out, tempRef, key))
			return false;
	}

	if (m_framePointerEnabled)
	{
		if ((m_framePointer == DEFAULT_FRAME_POINTER) && m_normalStack)
			EMIT(leave);
		else if (m_normalStack)
		{
#ifdef OUTPUT32
			EMIT_RR(mov_32, m_stackPointer, m_framePointer);
#else
			EMIT_RR(mov_64, m_stackPointer, m_framePointer);
#endif
			EMIT_R(pop, m_framePointer);
		}
		else
		{
#ifdef OUTPUT32
			EMIT_RR(mov_32, m_stackPointer, m_framePointer);
			EMIT_RM(mov_32, m_framePointer, X86_MEM(m_stackPointer, 0));
			EMIT_RM(lea_32, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? -4 : 4));
#else
			EMIT_RR(mov_64, m_stackPointer, m_framePointer);
			EMIT_RM(mov_64, m_framePointer, X86_MEM(m_stackPointer, 0));
			EMIT_RM(lea_64, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? -8 : 8));
#endif
		}
	}
	else
	{
		if (m_stackFrameSize != 0)
		{
			if (m_settings.stackGrowsUp)
			{
#ifdef OUTPUT32
				EMIT_RI(sub_32, m_stackPointer, m_stackFrameSize);
#else
				EMIT_RI(sub_64, m_stackPointer, m_stackFrameSize);
#endif
			}
			else
			{
#ifdef OUTPUT32
				EMIT_RI(add_32, m_stackPointer, m_stackFrameSize);
#else
				EMIT_RI(add_64, m_stackPointer, m_stackFrameSize);
#endif
			}
		}
	}

	if (m_normalStack)
	{
		if (m_settings.encodePointers)
		{
			// Using encoded pointers, decode return address before returning
#ifdef OUTPUT32
			EMIT_MR(xor_32, X86_MEM(m_stackPointer, 0), temp);
#else
			EMIT_MR(xor_64, X86_MEM(m_stackPointer, 0), temp);
#endif
		}

		EMIT(retn);
	}
	else
	{
		if (m_settings.encodePointers)
		{
			// Using encoded pointers, decode return address before returning
#ifdef OUTPUT32
			EMIT_RM(xor_32, temp, X86_MEM(m_stackPointer, 0));
#else
			EMIT_RM(xor_64, temp, X86_MEM(m_stackPointer, 0));
#endif
		}
		else
		{
#ifdef OUTPUT32
			EMIT_RM(mov_32, temp, X86_MEM(m_stackPointer, 0));
#else
			EMIT_RM(mov_64, temp, X86_MEM(m_stackPointer, 0));
#endif
		}

#ifdef OUTPUT32
		EMIT_RM(lea_32, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? -4 : 4));
#else
		EMIT_RM(lea_64, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? -8 : 8));
#endif
		EMIT_R(jmpn, temp);
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateAlloca(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, size;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], size))
		return false;

	OperandReference stack;
	stack.type = OPERANDREF_REG;
#ifdef OUTPUT32
	stack.width = 4;
#else
	stack.width = 8;
#endif
	stack.reg = m_stackPointer;

	if (m_settings.stackGrowsUp)
	{
#ifdef OUTPUT32
		EMIT_RI(add_32, m_stackPointer, 4);
#else
		EMIT_RI(add_32, m_stackPointer, 8);
#endif

		if (!Move(out, dest, stack))
			return false;
		if (!Add(out, stack, size))
			return false;

		EMIT_R(dec_32, m_stackPointer);
#ifdef OUTPUT32
		EMIT_RI(and_32, m_stackPointer, ~3);
#else
		EMIT_RI(and_64, m_stackPointer, ~7);
#endif

		return true;
	}
	else
	{
		if (!Sub(out, stack, size))
			return false;

#ifdef OUTPUT32
		EMIT_RI(and_32, m_stackPointer, ~3);
#else
		EMIT_RI(and_64, m_stackPointer, ~7);
#endif

		return Move(out, dest, stack);
	}
}


bool OUTPUT_CLASS_NAME::GenerateMemcpy(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, REG_ESI, REG_EDI, REG_ECX, NONE);

	OperandReference dest, src, size;
	if (!PrepareLoad(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (!PrepareLoad(out, instr.params[2], size))
		return false;

#ifdef OUTPUT32
	switch (src.type)
	{
	case OPERANDREF_REG:
		if (src.reg != REG_ESI)
			EMIT_RR(mov_32, REG_ESI, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, REG_ESI, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, REG_ESI, src.immed);
		break;
	default:
		return false;
	}

	switch (dest.type)
	{
	case OPERANDREF_REG:
		if (dest.reg != REG_EDI)
			EMIT_RR(mov_32, REG_EDI, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, REG_EDI, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, REG_EDI, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		if (size.reg != REG_ECX)
			EMIT_RR(mov_32, REG_ECX, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, REG_ECX, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, REG_ECX, size.immed);
		break;
	default:
		return false;
	}
#else
	switch (src.type)
	{
	case OPERANDREF_REG:
		if (src.reg != REG_RDI)
			EMIT_RR(mov_64, REG_RSI, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, REG_RSI, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, REG_RSI, src.immed);
		break;
	default:
		return false;
	}

	switch (dest.type)
	{
	case OPERANDREF_REG:
		if (dest.reg != REG_RDI)
			EMIT_RR(mov_64, REG_RDI, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, REG_RDI, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, REG_RDI, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		if (size.reg != REG_RCX)
			EMIT_RR(mov_64, REG_RCX, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, REG_RCX, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, REG_RCX, size.immed);
		break;
	default:
		return false;
	}
#endif

	EMIT(rep);
	EMIT(movsb);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateMemset(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, REG_EAX, REG_EDI, REG_ECX, NONE);

	OperandReference dest, src, size;
	if (!PrepareLoad(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	if (!PrepareLoad(out, instr.params[2], size))
		return false;

	switch (src.type)
	{
	case OPERANDREF_REG:
		if (src.reg != REG_AL)
			EMIT_RR(mov_8, REG_AL, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_8, REG_AL, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_8, REG_AL, src.immed);
		break;
	default:
		return false;
	}

#ifdef OUTPUT32
	switch (dest.type)
	{
	case OPERANDREF_REG:
		if (dest.reg != REG_EDI)
			EMIT_RR(mov_32, REG_EDI, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, REG_EDI, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, REG_EDI, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		if (size.reg != REG_ECX)
			EMIT_RR(mov_32, REG_ECX, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, REG_ECX, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, REG_ECX, size.immed);
		break;
	default:
		return false;
	}
#else
	switch (dest.type)
	{
	case OPERANDREF_REG:
		if (dest.reg != REG_RDI)
			EMIT_RR(mov_64, REG_RDI, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, REG_RDI, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, REG_RDI, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		if (size.reg != REG_RCX)
			EMIT_RR(mov_64, REG_RCX, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, REG_RCX, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, REG_RCX, size.immed);
		break;
	default:
		return false;
	}
#endif

	EMIT(rep);
	EMIT(stosb);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateSyscall(OutputBlock* out, const ILInstruction& instr)
{
	if (m_settings.os == OS_LINUX)
	{
#ifdef OUTPUT32
		static const OperandType linuxRegs[] = {REG_EAX, REG_EBX, REG_ECX, REG_EDX, REG_ESI, REG_EDI, REG_EBP, NONE};
#else
		static const OperandType linuxRegs[] = {REG_RAX, REG_RDI, REG_RSI, REG_RDX, REG_R10, REG_R8, REG_R9, NONE};
#endif
		OperandType savedRegs[2];
		size_t savedRegCount = 0;
		bool savedFramePointer = false;
		bool savedBasePointer = false;

		if (m_stackPointer != DEFAULT_STACK_POINTER)
		{
#ifdef OUTPUT32
			EMIT_RR(xchg_32, m_stackPointer, REG_ESP);
#else
			EMIT_RR(xchg_64, m_stackPointer, REG_RSP);
#endif
		}

		ReserveRegisters(NULL, REG_ESP, REG_EBP, NONE);

#ifdef OUTPUT64
		if (m_framePointer == REG_RCX)
		{
			EMIT_RR(xchg_64, m_framePointer, REG_RBP);
			m_framePointer = REG_RBP;
		}
#endif

		size_t regIndex = 0;
		for (size_t i = 1; i < instr.params.size(); i++)
		{
			if (linuxRegs[regIndex] == NONE)
				return false;
			ReserveRegisters(NULL, linuxRegs[regIndex], NONE);

			if (instr.params[i].cls == ILPARAM_UNDEFINED)
				continue;

#ifdef OUTPUT32
			if (instr.params[i].GetWidth() == 8)
			{
				if (linuxRegs[regIndex] == NONE)
					return false;
				ReserveRegisters(NULL, linuxRegs[regIndex], NONE);
			}
#endif

			if ((linuxRegs[regIndex] == m_framePointer) && (!savedFramePointer))
			{
				if (m_settings.stackGrowsUp)
				{
#ifdef OUTPUT32
					EMIT_RM(lea_32, DEFAULT_STACK_POINTER, X86_MEM(DEFAULT_STACK_POINTER, 4));
					EMIT_MR(mov_32, X86_MEM(DEFAULT_STACK_POINTER, 0), m_framePointer);
#else
					EMIT_RM(lea_64, DEFAULT_STACK_POINTER, X86_MEM(DEFAULT_STACK_POINTER, 8));
					EMIT_MR(mov_64, X86_MEM(DEFAULT_STACK_POINTER, 0), m_framePointer);
#endif
				}
				else
				{
					EMIT_R(push, m_framePointer);
				}

				savedRegs[savedRegCount++] = m_framePointer;
				savedFramePointer = true;

				if (m_framePointer != DEFAULT_FRAME_POINTER)
				{
#ifdef OUTPUT32
					EMIT_RR(mov_32, DEFAULT_FRAME_POINTER, m_framePointer);
#else
					EMIT_RR(mov_64, DEFAULT_FRAME_POINTER, m_framePointer);
#endif
					m_framePointer = DEFAULT_FRAME_POINTER;
				}
			}

			if ((linuxRegs[regIndex] == m_basePointer) && (!savedBasePointer))
			{
				if (m_settings.stackGrowsUp)
				{
#ifdef OUTPUT32
					EMIT_RM(lea_32, DEFAULT_STACK_POINTER, X86_MEM(DEFAULT_STACK_POINTER, 4));
					EMIT_MR(mov_32, X86_MEM(DEFAULT_STACK_POINTER, 0), m_basePointer);
#else
					EMIT_RM(lea_64, DEFAULT_STACK_POINTER, X86_MEM(DEFAULT_STACK_POINTER, 8));
					EMIT_MR(mov_64, X86_MEM(DEFAULT_STACK_POINTER, 0), m_basePointer);
#endif
				}
				else
				{
					EMIT_R(push, m_basePointer);
				}

				savedRegs[savedRegCount++] = m_basePointer;
				savedBasePointer = true;
				m_basePointer = NONE;
			}

			OperandReference cur;
			if (!PrepareLoad(out, instr.params[i], cur))
				return false;

#ifdef OUTPUT32
			OperandType reg, highReg;
			if (instr.params[i].GetWidth() == 8)
			{
				reg = GetRegisterOfSize(linuxRegs[regIndex++], 4);
				highReg = GetRegisterOfSize(linuxRegs[regIndex++], 4);
			}
			else
			{
				reg = GetRegisterOfSize(linuxRegs[regIndex++], cur.width);
				highReg = NONE;
			}
#else
			OperandType reg = GetRegisterOfSize(linuxRegs[regIndex++], cur.width);
#endif

			OperandReference dest;
			dest.type = OPERANDREF_REG;
			dest.width = cur.width;
			dest.reg = reg;
#ifdef OUTPUT32
			dest.highReg = highReg;
#endif

			if (!Move(out, dest, cur))
				return false;
		}

#ifdef OUTPUT32
		EMIT_I(int, 0x80);
#else
		EMIT(syscall);
#endif

		m_framePointer = m_origFramePointer;
		m_basePointer = m_origBasePointer;

		for (size_t i = 0; i < savedRegCount; i++)
		{
			OperandType reg = savedRegs[(savedRegCount - 1) - i];

			if (m_settings.stackGrowsUp)
			{
#ifdef OUTPUT32
				EMIT_RM(mov_32, reg, X86_MEM(DEFAULT_STACK_POINTER, 0));
				EMIT_RM(lea_32, DEFAULT_STACK_POINTER, X86_MEM(DEFAULT_STACK_POINTER, -4));
#else
				EMIT_RM(mov_64, reg, X86_MEM(DEFAULT_STACK_POINTER, 0));
				EMIT_RM(lea_64, DEFAULT_STACK_POINTER, X86_MEM(DEFAULT_STACK_POINTER, -8));
#endif
			}
			else
			{
				EMIT_R(pop, reg);
			}
		}

		if (m_stackPointer != DEFAULT_STACK_POINTER)
		{
#ifdef OUTPUT32
			EMIT_RR(xchg_32, m_stackPointer, REG_ESP);
#else
			EMIT_RR(xchg_64, m_stackPointer, REG_RSP);
#endif
		}

#ifdef OUTPUT64
		if (m_framePointer == REG_RCX)
			EMIT_RR(xchg_64, m_framePointer, REG_RBP);
#endif

		OperandReference dest, result;
		if (!PrepareStore(out, instr.params[0], dest))
			return false;
		result.type = OPERANDREF_REG;
		result.width = dest.width;
		result.reg = GetRegisterOfSize(REG_EAX, result.width);
		return Move(out, dest, result);
	}

	return false;
}


bool OUTPUT_CLASS_NAME::GenerateRdtsc(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	EMIT(rdtsc);

	if (dest.type == OPERANDREF_REG)
	{
#ifdef OUTPUT32
		EMIT_RR(mov_32, dest.reg, REG_EAX);
		EMIT_RR(mov_32, dest.highReg, REG_EDX);
#else
		EMIT_RR(mov_64, dest.reg, REG_RDX);
		EMIT_RI(shl_64, dest.reg, 32);
		EMIT_RR(or_64, dest.reg, REG_RAX);
#endif
	}
	else
	{
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), REG_EAX);
		EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), REG_EDX);
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateRdtscLow(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	EMIT(rdtsc);
	if (dest.type == OPERANDREF_REG)
		EMIT_RR(mov_32, dest.reg, REG_EAX);
	else
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), REG_EAX);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateRdtscHigh(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, REG_EAX, REG_EDX, NONE);

	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	EMIT(rdtsc);
	if (dest.type == OPERANDREF_REG)
		EMIT_RR(mov_32, dest.reg, REG_EDX);
	else
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), REG_EDX);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateNextArg(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;

	if (m_settings.stackGrowsUp)
		return Sub(out, dest, b);
	else
		return Add(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GeneratePrevArg(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;
	if (!Move(out, dest, a))
		return false;

	if (m_settings.stackGrowsUp)
		return Add(out, dest, b);
	else
		return Sub(out, dest, b);
}


bool OUTPUT_CLASS_NAME::GenerateCodeBlock(OutputBlock* out, ILBlock* block)
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
		case ILOP_SYSCALL:
			if (!GenerateSyscall(out, *i))
				goto fail;
			break;
		case ILOP_RDTSC:
			if (!GenerateRdtsc(out, *i))
				goto fail;
			break;
		case ILOP_RDTSC_LOW:
			if (!GenerateRdtscLow(out, *i))
				goto fail;
			break;
		case ILOP_RDTSC_HIGH:
			if (!GenerateRdtscHigh(out, *i))
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


bool OUTPUT_CLASS_NAME::GenerateCode(Function* func)
{
	// Determine what the stack and frame pointers should be
	m_framePointer = DEFAULT_FRAME_POINTER;
	m_stackPointer = DEFAULT_STACK_POINTER;
#ifdef OUTPUT32
	m_basePointer = m_settings.positionIndependent ? DEFAULT_BASE_POINTER : NONE;
#else
	m_basePointer = NONE;
#endif
	if (func->IsVariableSizedStackFrame())
		m_framePointerEnabled = true;
	else
		m_framePointerEnabled = false;

#ifdef OUTPUT32
	if (m_settings.stackReg.size() != 0)
		m_stackPointer = GetRegisterOfSize(GetRegisterByName(m_settings.stackReg), 4);
	if (m_settings.frameReg.size() != 0)
		m_framePointer = GetRegisterOfSize(GetRegisterByName(m_settings.frameReg), 4);
	if ((m_settings.baseReg.size() != 0) && m_settings.positionIndependent)
		m_basePointer = GetRegisterOfSize(GetRegisterByName(m_settings.baseReg), 4);
#else
	if (m_settings.stackReg.size() != 0)
		m_stackPointer = GetRegisterOfSize(GetRegisterByName(m_settings.stackReg), 8);
	if (m_settings.frameReg.size() != 0)
		m_framePointer = GetRegisterOfSize(GetRegisterByName(m_settings.frameReg), 8);
#endif

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

#ifdef OUTPUT32
	if ((m_basePointer == NONE) && m_settings.positionIndependent)
	{
		fprintf(stderr, "error: invalid base pointer register\n");
		return false;
	}
#endif

	if (m_stackPointer == m_framePointer)
	{
		fprintf(stderr, "error: stack pointer and frame pointer cannot be the same register\n");
		return false;
	}

	if (m_stackPointer == m_basePointer)
	{
		fprintf(stderr, "error: stack pointer and base pointer cannot be the same register\n");
		return false;
	}

	if (m_framePointer == m_basePointer)
	{
		fprintf(stderr, "error: frame pointer and base pointer cannot be the same register\n");
		return false;
	}

	m_normalStack = true;
	if ((m_stackPointer != DEFAULT_STACK_POINTER) || m_settings.stackGrowsUp)
		m_normalStack = false;

	m_origStackPointer = m_stackPointer;
	m_origFramePointer = m_framePointer;
	m_origBasePointer = m_basePointer;

	// Determine which registers can be used as temporaries
	m_temporaryRegisters[0] = REG_EAX;
	m_temporaryRegisters[1] = REG_ECX;
	m_temporaryRegisters[2] = REG_EDX;
	m_temporaryRegisters[3] = REG_EBX;
	m_temporaryRegisters[4] = REG_ESP;
	m_temporaryRegisters[5] = REG_EBP;
	m_temporaryRegisters[6] = REG_ESI;
	m_temporaryRegisters[7] = REG_EDI;
#ifdef OUTPUT32
	m_maxTemporaryRegisters = 8;
#else
	m_temporaryRegisters[8] = REG_R8D;
	m_temporaryRegisters[9] = REG_R9D;
	m_temporaryRegisters[10] = REG_R10D;
	m_temporaryRegisters[11] = REG_R11D;
	m_temporaryRegisters[12] = REG_R12D;
	m_temporaryRegisters[13] = REG_R13D;
	m_temporaryRegisters[14] = REG_R14D;
	m_temporaryRegisters[15] = REG_R15D;
	m_maxTemporaryRegisters = 16;
#endif

	for (size_t i = 0; i < m_maxTemporaryRegisters; i++)
	{
		if ((m_temporaryRegisters[i] == GetRegisterOfSize(m_stackPointer, 4)) ||
			(m_temporaryRegisters[i] == GetRegisterOfSize(m_framePointer, 4)) ||
			(m_temporaryRegisters[i] == GetRegisterOfSize(m_basePointer, 4)))
		{
			memmove(&m_temporaryRegisters[i], &m_temporaryRegisters[i + 1], sizeof(OperandType) *
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
#ifdef OUTPUT32
	if (offset & 3)
		offset += 4 - (offset & 3);
#else
	if (offset & 7)
		offset += 8 - (offset & 7);
#endif

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
	{
		// Because memory references with the stack pointer are one byte larger than those that
		// access the frame pointer, enable the frame pointer if there are going to be any
		// stack variable references
		m_framePointerEnabled = true;
	}

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
#ifdef OUTPUT32
		if (m_framePointerEnabled)
			m_stackFrame[*var] = offset + 8;
		else
			m_stackFrame[*var] = offset + m_stackFrameSize + 4;
#else
		if (m_framePointerEnabled)
			m_stackFrame[*var] = offset + 16;
		else
			m_stackFrame[*var] = offset + m_stackFrameSize + 8;
#endif

		if (m_settings.stackGrowsUp)
			m_stackFrame[*var] = -m_stackFrame[*var];

		// Adjust offset for next parameter
		offset += (*var)->GetType()->GetWidth();
#ifdef OUTPUT32
		if (offset & 3)
			offset += 4 - (offset & 3);
#else
		if (offset & 7)
			offset += 8 - (offset & 7);
#endif
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
			// Generate function prologue
			if ((!m_normalStack) && (func->GetName() == "_start"))
			{
				// If using alternate stack pointer, and this is the _start function, initialize stack pointer
				size_t stackAdjust = m_settings.stackGrowsUp ? -0x10000 : 0;
				EMIT_II(enter, 0, 0);
#ifdef OUTPUT32
				EMIT_RM(lea_32, m_stackPointer, X86_MEM(REG_EBP, -4 + stackAdjust));
				EMIT_MR(mov_32, X86_MEM(m_stackPointer, 0), m_framePointer);
				EMIT_RR(mov_32, m_framePointer, m_stackPointer);
#else
				EMIT_RM(lea_64, m_stackPointer, X86_MEM(REG_EBP, -8 + stackAdjust));
				EMIT_MR(mov_64, X86_MEM(m_stackPointer, 0), m_framePointer);
				EMIT_RR(mov_64, m_framePointer, m_stackPointer);
#endif
			}
			else if (m_framePointerEnabled)
			{
				if (m_normalStack)
					EMIT_R(push, m_framePointer);
				else
				{
#ifdef OUTPUT32
					EMIT_RM(lea_32, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 4 : -4));
					EMIT_MR(mov_32, X86_MEM(m_stackPointer, 0), m_framePointer);
#else
					EMIT_RM(lea_64, m_stackPointer, X86_MEM(m_stackPointer, m_settings.stackGrowsUp ? 8 : -8));
					EMIT_MR(mov_64, X86_MEM(m_stackPointer, 0), m_framePointer);
#endif
				}

#ifdef OUTPUT32
				EMIT_RR(mov_32, m_framePointer, m_stackPointer);
#else
				EMIT_RR(mov_64, m_framePointer, m_stackPointer);
#endif
			}

			if (m_stackFrameSize != 0)
			{
				if (m_settings.stackGrowsUp)
				{
#ifdef OUTPUT32
					EMIT_RI(add_32, m_stackPointer, m_stackFrameSize);
#else
					EMIT_RI(add_64, m_stackPointer, m_stackFrameSize);
#endif
				}
				else
				{
#ifdef OUTPUT32
					EMIT_RI(sub_32, m_stackPointer, m_stackFrameSize);
#else
					EMIT_RI(sub_64, m_stackPointer, m_stackFrameSize);
#endif
				}
			}

#ifdef OUTPUT32
			if (m_settings.positionIndependent && (func->GetName() == "_start"))
			{
				// Capture base of code at start
				size_t leaOffset = GetInstructionPointer(out, m_basePointer);
				Relocation reloc;
				reloc.type = CODE_RELOC_RELATIVE_8;
				reloc.overflow = LeaOverflowHandler;
				reloc.instruction = leaOffset;
				reloc.offset = out->len - 1;
				reloc.target = func->GetIL()[0];
				out->relocs.push_back(reloc);
			}
#endif

			first = false;
		}

		if (!GenerateCodeBlock(out, *i))
			return false;

		(*i)->SetOutputBlock(out);
	}

	return true;
}

#endif

