#ifdef OUTPUT_CLASS_NAME

#include <stdio.h>
#include <string.h>
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


OUTPUT_CLASS_NAME::OUTPUT_CLASS_NAME()
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


OperandType OUTPUT_CLASS_NAME::AllocateTemporaryRegister(OutputBlock* out, size_t size)
{
	// Skip reserved registers
	while ((m_temporaryCount < 4) && (m_reserved[m_temporaryCount]))
		m_temporaryCount++;

	// Allocate the register
	switch (m_temporaryCount++)
	{
	case 0:
		return GetRegisterOfSize(REG_EAX, size);
	case 1:
		return GetRegisterOfSize(REG_EDX, size);
	case 2:
		return GetRegisterOfSize(REG_ECX, size);
	case 3:
		return GetRegisterOfSize(REG_EBX, size);
	default:
		return NONE;
	}
}


void OUTPUT_CLASS_NAME::ReserveRegister(OperandType reg)
{
	switch (reg)
	{
	case REG_EAX:
		m_reserved[0] = true;
		break;
	case REG_EDX:
		m_reserved[1] = true;
		break;
	case REG_ECX:
		m_reserved[2] = true;
		break;
	case REG_EBX:
		m_reserved[3] = true;
		break;
	default:
		break;
	}
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
		if (!param.parent->type->GetStruct())
			return false;

		const StructMember* member = param.parent->type->GetStruct()->GetMember(param.stringValue);
		if (!member)
			return false;

		parent.mem.offset += member->offset;
		ref = parent.mem;
		return true;
	}

	if (param.cls != ILPARAM_VAR)
		return false;
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
		ref.base = STACK_POINTER;
		ref.index = NONE;
		ref.scale = 1;
		ref.offset = i->second;
	}
	return true;
}


bool OUTPUT_CLASS_NAME::LoadCodePointer(OutputBlock* out, ILBlock* block, OperandReference& ref)
{
	ref.type = OPERANDREF_REG;
	ref.sign = false;
#ifdef OUTPUT32
	ref.width = 4;
#else
	ref.width = 8;
#endif
	ref.reg = AllocateTemporaryRegister(out, ref.width);

#ifdef OUTPUT32
	// On first pass, generate large relative code, as the first pass must generate
	// the maximum code size for this block
	bool large = true;
	if (m_finalPass)
	{
		// Final pass, so we know the maximum offset, check to see if a short reference is possible
		uint64_t curAddr = m_instrAddress + (out->len - m_instrStartLen) + 5;
		int64_t maxOffset = block->GetAddress() - curAddr;
		if ((maxOffset >= -0x80) && (maxOffset <= 0x7f))
			large = false;
	}

	if (large)
	{
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
		buffer[0] = 0xe8;
		*(uint32_t*)(&buffer[1]) = 0;
		out->FinishWrite(5);
		EMIT_R(pop, ref.reg);
		EMIT_RM(lea_32, ref.reg, X86_MEM(ref.reg, 0x11111111));
		*(int32_t*)((size_t)out->code + out->len - 4) = 7;

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}
	else
	{
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
		buffer[0] = 0xe8;
		*(uint32_t*)(&buffer[1]) = 0;
		out->FinishWrite(5);
		EMIT_R(pop, ref.reg);
		EMIT_RM(lea_32, ref.reg, X86_MEM(ref.reg, 0x11));
		*(int8_t*)((size_t)out->code + out->len - 1) = 4;

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_8;
		reloc.offset = out->len - 1;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}

	return true;
#else
	EMIT_RM(lea_64, ref.reg, X86_MEM(REG_RIP, 0));

	Relocation reloc;
	reloc.type = RELOC_RELATIVE_32;
	reloc.offset = out->len - 4;
	reloc.target = block;
	out->relocs.push_back(reloc);
	return true;
#endif
}


bool OUTPUT_CLASS_NAME::PrepareLoad(OutputBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.type->GetWidth();
	ref.sign = param.type->IsSigned();

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
	case ILPARAM_FUNC:
		return LoadCodePointer(out, param.function->GetIL()[0], ref);
	default:
		return false;
	}
}


bool OUTPUT_CLASS_NAME::PrepareStore(OutputBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.type->GetWidth();

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
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _16, dest.reg, (uint8_t)src.immed); return true; \
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
			case OPERANDREF_IMMED:  EMIT_RI(operation ## _32, dest.reg, (uint8_t)src.immed); return true; \
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
#define IMPLEMENT_SHIFT_OP_64(operation, right) \
	if (dest.type == OPERANDREF_REG) \
	{ \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			if (right) \
			{ \
				EMIT_RRR(shrd_32, dest.reg, dest.highReg, src.reg); \
				EMIT_RR(operation ## _32, dest.highReg, src.reg); \
			} \
			else \
			{ \
				EMIT_RRR(shld_32, dest.highReg, dest.reg, src.reg); \
				EMIT_RR(operation ## _32, dest.reg, src.reg); \
			} \
			return true; \
		case OPERANDREF_IMMED: \
			if (right) \
			{ \
				EMIT_RRI(shrd_32, dest.reg, dest.highReg, src.immed); \
				EMIT_RI(operation ## _32, dest.highReg, src.immed); \
			} \
			else \
			{ \
				EMIT_RRI(shld_32, dest.highReg, dest.reg, src.immed); \
				EMIT_RI(operation ## _32, dest.reg, src.immed); \
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
				EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_MRR(shrd_32, X86_MEM_REF(dest.mem), temp, src.reg); \
				EMIT_MR(operation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), src.reg); \
			} \
			else \
			{ \
				EMIT_RM(mov_32, temp, X86_MEM_REF(dest.mem)); \
				EMIT_MRR(shld_32, X86_MEM_REF_OFFSET(dest.mem, 4), temp, src.reg); \
				EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), src.reg); \
			} \
			return true; \
		case OPERANDREF_IMMED: \
			if (right) \
			{ \
				EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_MRI(shrd_32, X86_MEM_REF(dest.mem), temp, src.immed); \
				EMIT_MI(operation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), src.immed); \
			} \
			else \
			{ \
				EMIT_RM(mov_32, temp, X86_MEM_REF(dest.mem)); \
				EMIT_MRI(shld_32, X86_MEM_REF_OFFSET(dest.mem, 4), temp, src.immed); \
				EMIT_MI(operation ## _32, X86_MEM_REF(dest.mem), src.immed); \
			} \
			return true; \
		default: \
			return false; \
		} \
	}
#else
#define IMPLEMENT_SHIFT_OP_64(operation, right) \
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

#define IMPLEMENT_SHIFT_OP(operation, right) \
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
		IMPLEMENT_SHIFT_OP_64(operation, right) \
	} \
	return false;


bool OUTPUT_CLASS_NAME::ShiftLeft(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(shl, false);
}


bool OUTPUT_CLASS_NAME::ShiftRightUnsigned(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(shr, true);
}


bool OUTPUT_CLASS_NAME::ShiftRightSigned(OutputBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(sar, true);
}


void OUTPUT_CLASS_NAME::ConditionalJump(OutputBlock* out, ConditionalJumpType type, ILBlock* trueBlock, ILBlock* falseBlock)
{
	// On first pass, generate large jump code, as the first pass must generate
	// the maximum code size for this block
	bool large = true;
	if (m_finalPass)
	{
		// Final pass, so we know the maximum offset, check to see if a short jump is possible
		uint64_t curAddr = m_instrAddress + (out->len - m_instrStartLen) + 2;
		int64_t maxOffset = trueBlock->GetAddress() - curAddr;
		if ((maxOffset >= -0x80) && (maxOffset <= 0x7f))
			large = false;
	}

	if (large)
	{
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(6);
		buffer[0] = 0x0f;
		buffer[1] = 0x80 + (uint8_t)type;
		*(uint32_t*)(&buffer[2]) = 0;
		out->FinishWrite(6);

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = trueBlock;
		out->relocs.push_back(reloc);
	}
	else
	{
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
		buffer[0] = 0x70 + (uint8_t)type;
		buffer[1] = 0;
		out->FinishWrite(2);

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_8;
		reloc.offset = out->len - 1;
		reloc.target = trueBlock;
		out->relocs.push_back(reloc);
	}

	if (falseBlock)
		UnconditionalJump(out, falseBlock);
}


void OUTPUT_CLASS_NAME::UnconditionalJump(OutputBlock* out, ILBlock* block)
{
	// On first pass, generate large jump code, as the first pass must generate
	// the maximum code size for this block
	bool large = true;
	if (m_finalPass)
	{
		// Final pass, so we know the maximum offset, check to see if a short jump is possible
		uint64_t curAddr = m_instrAddress + (out->len - m_instrStartLen) + 2;
		int64_t maxOffset = block->GetAddress() - curAddr;
		if ((maxOffset >= -0x80) && (maxOffset <= 0x7f))
			large = false;
	}

	if (large)
	{
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
		buffer[0] = 0xe9;
		*(uint32_t*)(&buffer[1]) = 0;
		out->FinishWrite(5);

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}
	else
	{
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(2);
		buffer[0] = 0xeb;
		buffer[1] = 0;
		out->FinishWrite(2);

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_8;
		reloc.offset = out->len - 1;
		reloc.target = block;
		out->relocs.push_back(reloc);
	}
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
	temp.sign = false;
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


bool OUTPUT_CLASS_NAME::GenerateDeref(OutputBlock* out, const ILInstruction& instr)
{
	return false;
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
		temp.sign = false;
		temp.reg = AllocateTemporaryRegister(out, temp.width);
		if (!Move(out, temp, src))
			return false;
		src = temp;
	}

	if (instr.params[1].type->GetClass() != TYPE_POINTER)
		return false;
	if (!instr.params[1].type->GetChildType()->GetStruct())
		return false;

	const StructMember* member = instr.params[1].type->GetChildType()->GetStruct()->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = dest.width;
	deref.sign = dest.sign;
	deref.mem.base = src.reg;
	deref.mem.scale = 1;
	deref.mem.index = NONE;
	deref.mem.offset = member->offset;
	return Move(out, dest, deref);
}


bool OUTPUT_CLASS_NAME::GenerateDerefAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateDerefMemberAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateArrayIndex(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateArrayIndexAssign(OutputBlock* out, const ILInstruction& instr)
{
	return false;
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


bool OUTPUT_CLASS_NAME::GenerateMult(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateDiv(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateMod(OutputBlock* out, const ILInstruction& instr)
{
	return false;
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

	ReserveRegister(REG_ECX);
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.sign = false;
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

	ReserveRegister(REG_ECX);
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.sign = false;
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

	ReserveRegister(REG_ECX);
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.sign = false;
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
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateNot(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateIfTrue(OutputBlock* out, const ILInstruction& instr)
{
	return false;
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
		m_temporaryCount = 0;
		memset(m_reserved, 0, sizeof(m_reserved));

		OperandReference param;
		if (!PrepareLoad(out, instr.params[i], param))
			return false;

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
			temp.sign = false;
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

	m_temporaryCount = 0;
	memset(m_reserved, 0, sizeof(m_reserved));

	// Perform function call
	if (instr.params[1].cls == ILPARAM_FUNC)
	{
		// Direct function call
		uint8_t* buffer = (uint8_t*)out->PrepareWrite(5);
		buffer[0] = 0xe8;
		*(uint32_t*)(&buffer[1]) = 0;
		out->FinishWrite(5);

		Relocation reloc;
		reloc.type = RELOC_RELATIVE_32;
		reloc.offset = out->len - 4;
		reloc.target = instr.params[1].function->GetIL()[0];
		out->relocs.push_back(reloc);
	}
	else
	{
		// Indirect function call
		OperandReference func;
		if (!PrepareLoad(out, instr.params[1], func))
			return false;

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

	// Adjust stack pointer to pop off parameters
	if (pushSize != 0)
	{
#ifdef OUTPUT32
		EMIT_RI(add_32, STACK_POINTER, pushSize);
#else
		EMIT_RI(add_64, STACK_POINTER, pushSize);
#endif
	}

	// Store return value, if there is one
	if (instr.params[0].cls != ILPARAM_VOID)
	{
		OperandReference dest, retVal;
		if (!PrepareStore(out, instr.params[0], dest))
			return false;

		retVal.type = OPERANDREF_REG;
		retVal.sign = dest.sign;
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


bool OUTPUT_CLASS_NAME::GenerateConvert(OutputBlock* out, const ILInstruction& instr)
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
			if (src.sign)
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
			if (src.sign)
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
			else
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
		}
		else if (src.width == 2)
		{
			if (src.sign)
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
			if (src.sign)
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
			else
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
		}
		else if (src.width == 2)
		{
			if (src.sign)
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
			else
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
		}
		else if (src.width == 4)
		{
			if (src.sign)
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
				if (dest.type == OPERANDREF_REG)
					EMIT_RR(xor_32, dest.highReg, dest.highReg);
				else
					EMIT_MI(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
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
			if (src.sign)
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
			else
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
		}
		else if (src.width == 2)
		{
			if (src.sign)
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
			else
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
		}
		else if (src.width == 4)
		{
			if (src.sign)
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
	dest.width = instr.params[0].type->GetWidth();
#ifdef OUTPUT32
	if (dest.width == 8)
	{
		dest.reg = REG_EAX;
		dest.reg = REG_EDX;
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
	if (m_framePointerEnabled)
	{
		EMIT(leave);
	}
	else
	{
		if (m_stackFrameSize != 0)
		{
#ifdef OUTPUT32
			EMIT_RI(add_32, STACK_POINTER, m_stackFrameSize);
#else
			EMIT_RI(add_64, STACK_POINTER, m_stackFrameSize);
#endif
		}
	}
	EMIT(retn);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateAlloca(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateMemcpy(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateMemset(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OUTPUT_CLASS_NAME::GenerateCodeBlock(OutputBlock* out, ILBlock* block)
{
	m_blockAddress = block->GetAddress();

	vector<ILInstruction>::iterator i;
	size_t blockOffset = out->len;
	for (i = block->GetInstructions().begin(); i != block->GetInstructions().end(); i++)
	{
		i->offset = out->len - blockOffset;
		m_instrAddress = m_blockAddress + i->offset;
		m_instrStartLen = out->len;

		m_temporaryCount = 0;
		memset(m_reserved, 0, sizeof(m_reserved));

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
		case ILOP_ADD:
			if (!GenerateAdd(out, *i))
				goto fail;
			break;
		case ILOP_SUB:
			if (!GenerateSub(out, *i))
				goto fail;
			break;
		case ILOP_MULT:
			if (!GenerateMult(out, *i))
				goto fail;
			break;
		case ILOP_DIV:
			if (!GenerateDiv(out, *i))
				goto fail;
			break;
		case ILOP_MOD:
			if (!GenerateMod(out, *i))
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
		case ILOP_CONVERT:
			if (!GenerateConvert(out, *i))
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


bool OUTPUT_CLASS_NAME::GenerateCode(Function* func, bool finalPass)
{
	// Generate stack frame
	m_framePointer = DEFAULT_FRAME_POINTER;
	if (func->IsVariableSizedStackFrame())
		m_framePointerEnabled = true;
	else
		m_framePointerEnabled = false;

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
			i->second -= m_stackFrameSize;
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
			fprintf(stderr, "error: parameter %d not found in variable list\n", (int)i);
			return false;
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
	m_finalPass = finalPass;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		OutputBlock* out = new OutputBlock;
		out->code = NULL;
		out->len = 0;
		out->maxLen = 0;

		if (first)
		{
			// Generate function prologue
			if (m_framePointerEnabled)
			{
				EMIT_R(push, m_framePointer);
#ifdef OUTPUT32
				EMIT_RR(mov_32, m_framePointer, STACK_POINTER);
#else
				EMIT_RR(mov_64, m_framePointer, STACK_POINTER);
#endif
			}

			if (m_stackFrameSize != 0)
			{
#ifdef OUTPUT32
				EMIT_RI(sub_32, STACK_POINTER, m_stackFrameSize);
#else
				EMIT_RI(sub_64, STACK_POINTER, m_stackFrameSize);
#endif
			}

			first = false;
		}

		if (!GenerateCodeBlock(out, *i))
			return false;

		if (finalPass)
		{
			// Perform sanity check on size, it should never get bigger.  If there are bugs
			// which cause the code generator to have nondeterministic output (remeber that
			// pseudorandom number generators are deterministic with a given seed) then it
			// could get bigger and cause relocations to be out of range.
			if (out->len > (*i)->GetOutputBlock()->len)
			{
				fprintf(stderr, "error: block grew during final pass\n");
				return false;
			}
		}

		(*i)->SetOutputBlock(out);
	}

	return true;
}

#endif

