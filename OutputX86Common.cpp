// Copyright (c) 2011-2012 Rusty Wagner
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

#ifdef OUTPUT_CLASS_NAME

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "OutputX86Common.h"
#include "Struct.h"
#include "Function.h"
#include "asmx86.h"
#ifdef OUTPUT32
#include "X86SymInstr.h"
#define OUTPUT32
#else
#include "X64SymInstr.h"
#define OUTPUT64
#endif

#define UNSAFE_STACK_PIVOT 0x1000

#define X86_MEM_REF(ref) X86_SYM_MEM_INDEX((ref).base, (ref).index, (ref).scale, (ref).var, (ref).offset)
#define X86_MEM_REF_OFFSET(ref, ofs) X86_SYM_MEM_INDEX((ref).base, (ref).index, (ref).scale, (ref).var, (ref).offset + (ofs))

#define EMIT(name)               out->AddInstruction(X86_SYMINSTR_NAME(name)())
#define EMIT_R(name, a)          out->AddInstruction(X86_SYMINSTR_NAME_OP(name, R)(a))
#define EMIT_M(name, a)          out->AddInstruction(X86_SYMINSTR_NAME_OP(name, M)(a))
#define EMIT_I(name, a)          out->AddInstruction(X86_SYMINSTR_NAME_OP(name, I)(a))
#define EMIT_RR(name, a, b)      out->AddInstruction(X86_SYMINSTR_NAME_OP(name, RR)(a, b))
#define EMIT_RM(name, a, b)      out->AddInstruction(X86_SYMINSTR_NAME_OP(name, RM)(a, b))
#define EMIT_MR(name, a, b)      out->AddInstruction(X86_SYMINSTR_NAME_OP(name, MR)(a, b))
#define EMIT_RI(name, a, b)      out->AddInstruction(X86_SYMINSTR_NAME_OP(name, RI)(a, b))
#define EMIT_MI(name, a, b)      out->AddInstruction(X86_SYMINSTR_NAME_OP(name, MI)(a, b))
#define EMIT_II(name, a, b)      out->AddInstruction(X86_SYMINSTR_NAME_OP(name, II)(a, b))
#define EMIT_RRR(name, a, b, c)  out->AddInstruction(X86_SYMINSTR_NAME_OP(name, RRR)(a, b, c))
#define EMIT_RRI(name, a, b, c)  out->AddInstruction(X86_SYMINSTR_NAME_OP(name, RRI)(a, b, c))
#define EMIT_MRR(name, a, b, c)  out->AddInstruction(X86_SYMINSTR_NAME_OP(name, MRR)(a, b, c))
#define EMIT_MRI(name, a, b, c)  out->AddInstruction(X86_SYMINSTR_NAME_OP(name, MRI)(a, b, c))

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
		if (mem.var != ref.mem.var)
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


OUTPUT_CLASS_NAME::OUTPUT_CLASS_NAME(const Settings& settings, Function* startFunc): Output(settings, startFunc)
{
}


uint32_t OUTPUT_CLASS_NAME::GetRegisterByName(const string& name)
{
	if (name == "al")
		return SYMREG_NATIVE_REG(REG_EAX);
	if (name == "ah")
		return SYMREG_NATIVE_REG(REG_EAX);
	if (name == "ax")
		return SYMREG_NATIVE_REG(REG_EAX);
	if (name == "eax")
		return SYMREG_NATIVE_REG(REG_EAX);
	if (name == "rax")
		return SYMREG_NATIVE_REG(REG_EAX);
	if (name == "cl")
		return SYMREG_NATIVE_REG(REG_ECX);
	if (name == "ch")
		return SYMREG_NATIVE_REG(REG_ECX);
	if (name == "cx")
		return SYMREG_NATIVE_REG(REG_ECX);
	if (name == "ecx")
		return SYMREG_NATIVE_REG(REG_ECX);
	if (name == "rcx")
		return SYMREG_NATIVE_REG(REG_ECX);
	if (name == "dl")
		return SYMREG_NATIVE_REG(REG_EDX);
	if (name == "dh")
		return SYMREG_NATIVE_REG(REG_EDX);
	if (name == "dx")
		return SYMREG_NATIVE_REG(REG_EDX);
	if (name == "edx")
		return SYMREG_NATIVE_REG(REG_EDX);
	if (name == "rdx")
		return SYMREG_NATIVE_REG(REG_EDX);
	if (name == "bl")
		return SYMREG_NATIVE_REG(REG_EBX);
	if (name == "bh")
		return SYMREG_NATIVE_REG(REG_EBX);
	if (name == "bx")
		return SYMREG_NATIVE_REG(REG_EBX);
	if (name == "ebx")
		return SYMREG_NATIVE_REG(REG_EBX);
	if (name == "rbx")
		return SYMREG_NATIVE_REG(REG_EBX);
	if (name == "spl")
		return SYMREG_NATIVE_REG(REG_ESP);
	if (name == "sp")
		return SYMREG_NATIVE_REG(REG_ESP);
	if (name == "esp")
		return SYMREG_NATIVE_REG(REG_ESP);
	if (name == "rsp")
		return SYMREG_NATIVE_REG(REG_ESP);
	if (name == "bpl")
		return SYMREG_NATIVE_REG(REG_EBP);
	if (name == "bp")
		return SYMREG_NATIVE_REG(REG_EBP);
	if (name == "ebp")
		return SYMREG_NATIVE_REG(REG_EBP);
	if (name == "rbp")
		return SYMREG_NATIVE_REG(REG_EBP);
	if (name == "sil")
		return SYMREG_NATIVE_REG(REG_ESI);
	if (name == "si")
		return SYMREG_NATIVE_REG(REG_ESI);
	if (name == "esi")
		return SYMREG_NATIVE_REG(REG_ESI);
	if (name == "rsi")
		return SYMREG_NATIVE_REG(REG_ESI);
	if (name == "dil")
		return SYMREG_NATIVE_REG(REG_EDI);
	if (name == "di")
		return SYMREG_NATIVE_REG(REG_EDI);
	if (name == "edi")
		return SYMREG_NATIVE_REG(REG_EDI);
	if (name == "rdi")
		return SYMREG_NATIVE_REG(REG_EDI);
	if (name == "r8b")
		return SYMREG_NATIVE_REG(REG_R8D);
	if (name == "r8w")
		return SYMREG_NATIVE_REG(REG_R8D);
	if (name == "r8d")
		return SYMREG_NATIVE_REG(REG_R8D);
	if (name == "r8")
		return SYMREG_NATIVE_REG(REG_R8D);
	if (name == "r9b")
		return SYMREG_NATIVE_REG(REG_R9D);
	if (name == "r9w")
		return SYMREG_NATIVE_REG(REG_R9D);
	if (name == "r9d")
		return SYMREG_NATIVE_REG(REG_R9D);
	if (name == "r9")
		return SYMREG_NATIVE_REG(REG_R9D);
	if (name == "r10b")
		return SYMREG_NATIVE_REG(REG_R10D);
	if (name == "r10w")
		return SYMREG_NATIVE_REG(REG_R10D);
	if (name == "r10d")
		return SYMREG_NATIVE_REG(REG_R10D);
	if (name == "r10")
		return SYMREG_NATIVE_REG(REG_R10D);
	if (name == "r11b")
		return SYMREG_NATIVE_REG(REG_R11D);
	if (name == "r11w")
		return SYMREG_NATIVE_REG(REG_R11D);
	if (name == "r11d")
		return SYMREG_NATIVE_REG(REG_R11D);
	if (name == "r11")
		return SYMREG_NATIVE_REG(REG_R11D);
	if (name == "r12b")
		return SYMREG_NATIVE_REG(REG_R12D);
	if (name == "r12w")
		return SYMREG_NATIVE_REG(REG_R12D);
	if (name == "r12d")
		return SYMREG_NATIVE_REG(REG_R12D);
	if (name == "r12")
		return SYMREG_NATIVE_REG(REG_R12D);
	if (name == "r13b")
		return SYMREG_NATIVE_REG(REG_R13D);
	if (name == "r13w")
		return SYMREG_NATIVE_REG(REG_R13D);
	if (name == "r13d")
		return SYMREG_NATIVE_REG(REG_R13D);
	if (name == "r13")
		return SYMREG_NATIVE_REG(REG_R13D);
	if (name == "r14b")
		return SYMREG_NATIVE_REG(REG_R14D);
	if (name == "r14w")
		return SYMREG_NATIVE_REG(REG_R14D);
	if (name == "r14d")
		return SYMREG_NATIVE_REG(REG_R14D);
	if (name == "r14")
		return SYMREG_NATIVE_REG(REG_R14D);
	if (name == "r15b")
		return SYMREG_NATIVE_REG(REG_R15D);
	if (name == "r15w")
		return SYMREG_NATIVE_REG(REG_R15D);
	if (name == "r15d")
		return SYMREG_NATIVE_REG(REG_R15D);
	if (name == "r15")
		return SYMREG_NATIVE_REG(REG_R15D);
	return SYMREG_NONE;
}


void OUTPUT_CLASS_NAME::GetDataAddressFromInstructionPointer(SymInstrBlock* out, uint32_t reg, int64_t offset)
{
	if (m_normalStack)
	{
		// Normal stack, use call/pop method
		out->AddInstruction(X86_SYMINSTR_NAME(CallPopDataAddr)(reg, offset));
	}
	else
	{
		// Not a normal stack, must use fstenv method
#ifdef OUTPUT32
		out->AddInstruction(X86_SYMINSTR_NAME(FstenvDataAddr)(reg, offset));
#else
		out->AddInstruction(X86_SYMINSTR_NAME(FstenvDataAddr)(reg, offset));
#endif
	}
}


void OUTPUT_CLASS_NAME::GetCodeAddressFromInstructionPointer(SymInstrBlock* out, uint32_t reg, Function* func, ILBlock* block)
{
	if (m_normalStack)
	{
		// Normal stack, use call/pop method
		out->AddInstruction(X86_SYMINSTR_NAME(CallPopCodeAddr)(reg, func, block));
	}
	else
	{
		// Not a normal stack, must use fstenv method
#ifdef OUTPUT32
		out->AddInstruction(X86_SYMINSTR_NAME(FstenvCodeAddr)(reg, func, block));
#else
		out->AddInstruction(X86_SYMINSTR_NAME(FstenvCodeAddr)(reg, func, block));
#endif
	}
}


bool OUTPUT_CLASS_NAME::AccessVariableStorage(SymInstrBlock* out, const ILParameter& param, OperandReference& ref)
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
		ref.type = OPERANDREF_MEM;
		ref.mem = parent.mem;
		return true;
	}

	if (param.cls != ILPARAM_VAR)
		return false;

	if (param.variable->IsGlobal())
	{
		uint32_t ptr = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		ref.type = OPERANDREF_MEM;
		ref.mem.base = ptr;
		ref.mem.index = SYMREG_NONE;
		ref.mem.scale = 1;
		ref.mem.var = SYMREG_NONE;
		ref.mem.offset = 0;

#ifdef OUTPUT32
		if (!m_settings.positionIndependent)
		{
			out->AddInstruction(X86_SYMINSTR_NAME(MovDataPtrAbsolute)(ptr, param.variable->GetDataSectionOffset()));
			return true;
		}

		if (m_settings.basePointer != SYMREG_NONE)
		{
			out->AddInstruction(X86_SYMINSTR_NAME(MovDataPtrBaseRelative)(ptr, m_settings.basePointer,
				param.variable->GetDataSectionOffset()));
			return true;
		}

		GetDataAddressFromInstructionPointer(out, ptr, param.variable->GetDataSectionOffset());
#else
		out->AddInstruction(X86_SYMINSTR_NAME(MovDataPtrBaseRelative)(ptr, SYMREG_IP,
			param.variable->GetDataSectionOffset()));
#endif

		return true;
	}

	if (m_varReg.find(param.variable) != m_varReg.end())
	{
		// Variable has a register associated with it
		ref.type = OPERANDREF_REG;
		ref.reg = m_varReg[param.variable];
		if ((!param.IsFloat()) && (param.variable->GetType()->GetWidth() == 8))
			ref.highReg = ref.reg + 1;
		else
			ref.highReg = SYMREG_NONE;
		return true;
	}

	map<Variable*, int32_t>::iterator i = m_stackVar.find(param.variable);
	if (i == m_stackVar.end())
		return false;
	ref.type = OPERANDREF_MEM;
	if (m_framePointerEnabled)
	{
		ref.mem.base = SYMREG_BP;
		ref.mem.index = SYMREG_NONE;
		ref.mem.scale = 1;
		ref.mem.var = i->second;
		ref.mem.offset = 0;
	}
	else
	{
		ref.mem.base = SYMREG_SP;
		ref.mem.index = SYMREG_NONE;
		ref.mem.scale = 1;
		ref.mem.var = i->second;
		ref.mem.offset = 0;
	}
	return true;
}


bool OUTPUT_CLASS_NAME::LoadCodePointer(SymInstrBlock* out, Function* func, ILBlock* block, OperandReference& ref)
{
	ref.type = OPERANDREF_REG;
#ifdef OUTPUT32
	ref.width = 4;
#else
	ref.width = 8;
#endif
	ref.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);

#ifdef OUTPUT32
	if (m_settings.basePointer != SYMREG_NONE)
	{
		out->AddInstruction(X86_SYMINSTR_NAME(MovCodePtrBaseRelative)(ref.reg, m_settings.basePointer, func, block));
	}
	else if (m_settings.positionIndependent)
	{
		GetCodeAddressFromInstructionPointer(out, ref.reg, func, block);
	}
	else
	{
		out->AddInstruction(X86_SYMINSTR_NAME(MovCodePtrAbsolute)(ref.reg, func, block));
	}
#else
	out->AddInstruction(X86_SYMINSTR_NAME(MovCodePtrBaseRelative)(ref.reg, SYMREG_IP, func, block));
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


bool OUTPUT_CLASS_NAME::PrepareLoad(SymInstrBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!AccessVariableStorage(out, param, ref))
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
		return LoadCodePointer(out, param.function, param.function->GetIL()[0], ref);
	case ILPARAM_UNDEFINED:
		ref.type = OPERANDREF_REG;
		ref.width = param.GetWidth();
		ref.reg = m_symFunc->AddRegister((ref.width == 1) ? X86REGCLASS_INTEGER_8BIT : X86REGCLASS_INTEGER);
		return true;
	default:
		return false;
	}
}


bool OUTPUT_CLASS_NAME::PrepareStore(SymInstrBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!AccessVariableStorage(out, param, ref))
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
		uint32_t temp; \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), src.reg); \
			EMIT_MR(highOperation ## _32, X86_MEM_REF_OFFSET(dest.mem, 4), src.highReg); \
			return true; \
		case OPERANDREF_MEM: \
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
		uint32_t temp; \
		switch (src.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(operation ## _64, X86_MEM_REF(dest.mem), src.reg); return true; \
		case OPERANDREF_MEM: \
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
			EMIT_RM(mov_64, temp, X86_MEM_REF(src.mem)); \
			EMIT_MR(operation ## _64, X86_MEM_REF(dest.mem), temp); \
			return true; \
		case OPERANDREF_IMMED: \
			if ((src.immed < -0x80000000LL) || (src.immed >= 0x80000000LL)) \
			{ \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
			uint32_t temp; \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _8, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_MEM: \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT); \
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
			uint32_t temp; \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _16, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_MEM: \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
			uint32_t temp; \
			switch (src.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(operation ## _32, X86_MEM_REF(dest.mem), src.reg); return true; \
			case OPERANDREF_MEM: \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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


bool OUTPUT_CLASS_NAME::Move(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	if (dest == src)
		return true;
	IMPLEMENT_BINARY_OP(mov, mov);
}


bool OUTPUT_CLASS_NAME::Add(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(add, adc);
}


bool OUTPUT_CLASS_NAME::Sub(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(sub, sbb);
}


bool OUTPUT_CLASS_NAME::And(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(and, and);
}


bool OUTPUT_CLASS_NAME::Or(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_BINARY_OP(or, or);
}


bool OUTPUT_CLASS_NAME::Xor(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
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
				uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT); \
				EMIT_RR(xor_32, temp, temp); \
				if (sign) \
				{ \
					EMIT_RR(test_32, dest.highReg, dest.highReg); \
					EMIT_R(sets, temp); \
					EMIT_R(neg_32, temp); \
				} \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, dest.reg, dest.highReg); \
				EMIT_RR(cmovnz_32, dest.highReg, temp); \
				EMIT_RRR(shrd_32, dest.reg, dest.highReg, src.reg); \
				EMIT_RR(operation ## _32, dest.highReg, src.reg); \
			} \
			else \
			{ \
				uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
						EMIT_RI(operation ## _32, dest.highReg, 31); \
					else \
						EMIT_RR(xor_32, dest.highReg, dest.highReg); \
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
		uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
		switch (src.type) \
		{ \
		case OPERANDREF_REG: \
			if (right) \
			{ \
				uint32_t high = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT); \
				EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_RR(xor_32, high, high); \
				if (sign) \
				{ \
					EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(dest.mem, 4), high); \
					EMIT_R(sets, high); \
					EMIT_R(neg_32, high); \
				} \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, temp, high); \
				uint32_t low = high; \
				EMIT_RM(mov_32, low, X86_MEM_REF(dest.mem)); \
				EMIT_RM(cmovnz_32, low, X86_MEM_REF_OFFSET(dest.mem, 4)); \
				EMIT_RRR(shrd_32, low, temp, src.reg); \
				EMIT_RR(operation ## _32, temp, src.reg); \
				EMIT_MR(mov_32, X86_MEM_REF(dest.mem), low); \
				EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), temp); \
			} \
			else \
			{ \
				uint32_t low = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
				EMIT_RM(mov_32, temp, X86_MEM_REF(dest.mem)); \
				EMIT_RR(xor_32, low, low); \
				EMIT_RI(test_8, src.reg, 32); \
				EMIT_RR(cmovnz_32, temp, low); \
				uint32_t high = low; \
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


bool OUTPUT_CLASS_NAME::ShiftLeft(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(shl, false, false);
}


bool OUTPUT_CLASS_NAME::ShiftRightUnsigned(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
{
	IMPLEMENT_SHIFT_OP(shr, true, false);
}


bool OUTPUT_CLASS_NAME::ShiftRightSigned(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src)
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


bool OUTPUT_CLASS_NAME::Neg(SymInstrBlock* out, const OperandReference& dest)
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


bool OUTPUT_CLASS_NAME::Not(SymInstrBlock* out, const OperandReference& dest)
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


bool OUTPUT_CLASS_NAME::Increment(SymInstrBlock* out, const OperandReference& dest)
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


bool OUTPUT_CLASS_NAME::Decrement(SymInstrBlock* out, const OperandReference& dest)
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


void OUTPUT_CLASS_NAME::ConditionalJump(SymInstrBlock* out, ConditionalJumpType type, ILBlock* trueBlock, ILBlock* falseBlock)
{
	out->AddInstruction(X86_SYMINSTR_NAME(CondJump)((uint8_t)type, m_func, trueBlock));
	if (falseBlock)
		UnconditionalJump(out, falseBlock);
}


void OUTPUT_CLASS_NAME::UnconditionalJump(SymInstrBlock* out, ILBlock* block, bool canOmit)
{
	if (canOmit && (!m_settings.pad) && (block->GetGlobalIndex() == (m_currentBlock->GetGlobalIndex() + 1)))
	{
		// The destination block is the one just after the current one, just fall through
		return;
	}

	out->AddInstruction(X86_SYMINSTR_NAME(JumpRelative)(m_func, block));
}


bool OUTPUT_CLASS_NAME::GenerateAssign(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;
	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GenerateAddressOf(SymInstrBlock* out, const ILInstruction& instr)
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
		EMIT_RM(lea, dest.reg, X86_MEM_REF(src.mem));
		return true;
	}

	temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
	temp.width = 4;
#else
	temp.width = 8;
#endif
	temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);

	EMIT_RM(lea, temp.reg, X86_MEM_REF(src.mem));
	return Move(out, dest, temp);
}


bool OUTPUT_CLASS_NAME::GenerateAddressOfMember(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	// Load pointer into a register
	OperandReference temp;
	temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
	temp.width = 4;
#else
	temp.width = 8;
#endif
	temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
	deref.mem.index = SYMREG_NONE;
	deref.mem.var = SYMREG_NONE;
	deref.mem.offset = member->offset;

	temp.type = OPERANDREF_REG;
#ifdef OUTPUT32
	temp.width = 4;
#else
	temp.width = 8;
#endif
	temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);

	EMIT_RM(lea, temp.reg, X86_MEM_REF(deref.mem));
	return Move(out, dest, temp);
}


bool OUTPUT_CLASS_NAME::GenerateDeref(SymInstrBlock* out, const ILInstruction& instr)
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, src))
			return false;
		src = temp;
	}

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = dest.width;
	deref.mem.base = src.reg;
	deref.mem.scale = 1;
	deref.mem.index = SYMREG_NONE;
	deref.mem.var = SYMREG_NONE;
	deref.mem.offset = 0;
	return Move(out, dest, deref);
}


bool OUTPUT_CLASS_NAME::GenerateDerefMember(SymInstrBlock* out, const ILInstruction& instr)
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
	deref.mem.index = SYMREG_NONE;
	deref.mem.var = SYMREG_NONE;
	deref.mem.offset = member->offset;
	return Move(out, dest, deref);
}


bool OUTPUT_CLASS_NAME::GenerateDerefAssign(SymInstrBlock* out, const ILInstruction& instr)
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, dest))
			return false;
		dest = temp;
	}

	OperandReference deref;
	deref.type = OPERANDREF_MEM;
	deref.width = src.width;
	deref.mem.base = dest.reg;
	deref.mem.scale = 1;
	deref.mem.index = SYMREG_NONE;
	deref.mem.var = SYMREG_NONE;
	deref.mem.offset = 0;
	return Move(out, deref, src);
}


bool OUTPUT_CLASS_NAME::GenerateDerefMemberAssign(SymInstrBlock* out, const ILInstruction& instr)
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
	deref.mem.index = SYMREG_NONE;
	deref.mem.var = SYMREG_NONE;
	deref.mem.offset = member->offset;
	return Move(out, deref, src);
}


bool OUTPUT_CLASS_NAME::GenerateArrayIndex(SymInstrBlock* out, const ILInstruction& instr)
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
	if (src.mem.index != SYMREG_NONE)
		return false;

	src.width = (size_t)instr.params[3].integerValue;

	if (index.type == OPERANDREF_IMMED)
		src.mem.offset += src.width * index.immed;
	else if ((src.width == 1) || (src.width == 2) || (src.width == 4) || (src.width == 8))
	{
		// Load index into a register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = index.width;
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_INDEX);
		if (!Move(out, temp, index))
			return false;
		index = temp;

		src.mem.scale = src.width;
		src.mem.index = index.reg;
	}
	else
	{
		return false;
	}

	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GenerateArrayIndexAssign(SymInstrBlock* out, const ILInstruction& instr)
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
	if (dest.mem.index != SYMREG_NONE)
		return false;

	dest.width = (size_t)instr.params[2].integerValue;

	if (index.type == OPERANDREF_IMMED)
		dest.mem.offset += src.width * index.immed;
	else if ((dest.width == 1) || (dest.width == 2) || (dest.width == 4) || (dest.width == 8))
	{
		// Load index into a register
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = index.width;
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_INDEX);
		if (!Move(out, temp, index))
			return false;
		index = temp;

		dest.mem.scale = dest.width;
		dest.mem.index = index.reg;
	}
	else
	{
		return false;
	}

	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GeneratePtrAdd(SymInstrBlock* out, const ILInstruction& instr)
{
	size_t width = (size_t)instr.params[3].integerValue;
	if (width == 1)
		return GenerateAdd(out, instr);

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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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

	// Load pointer into register
	OperandReference temp;
	temp.type = OPERANDREF_REG;
	temp.width = a.width;
	temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_INDEX);
	if (!Move(out, temp, a))
		return false;
	a = temp;

	if (dest.type == OPERANDREF_REG)
	{
		if ((width != 2) && (width != 4) && (width != 8))
		{
			uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#ifdef OUTPUT32
			EMIT_RRI(imul_32, temp, b.reg, width);
#else
			EMIT_RRI(imul_64, temp, b.reg, width);
#endif
			EMIT_RM(lea, dest.reg, X86_SYM_MEM_INDEX(a.reg, temp, 1, SYMREG_NONE, 0));
		}
		else
		{
			EMIT_RM(lea, dest.reg, X86_SYM_MEM_INDEX(a.reg, b.reg, width, SYMREG_NONE, 0));
		}
	}
	else
	{
		uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if ((width != 2) && (width != 4) && (width != 8))
		{
#ifdef OUTPUT32
			EMIT_RRI(imul_32, temp, b.reg, width);
			EMIT_RR(add_32, temp, a.reg);
			EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp);
#else
			EMIT_RRI(imul_64, temp, b.reg, width);
			EMIT_RR(add_64, temp, a.reg);
			EMIT_MR(mov_64, X86_MEM_REF(dest.mem), temp);
#endif
		}
		else
		{
			EMIT_RM(lea, temp, X86_SYM_MEM_INDEX(a.reg, b.reg, width, SYMREG_NONE, 0));
#ifdef OUTPUT32
			EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp);
#else
			EMIT_MR(mov_64, X86_MEM_REF(dest.mem), temp);
#endif
		}
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GeneratePtrSub(SymInstrBlock* out, const ILInstruction& instr)
{
	size_t width = (size_t)instr.params[3].integerValue;
	if (width == 1)
		return GenerateSub(out, instr);

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
	countTemp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_INDEX);
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, a))
			return false;
		a = temp;
	}

	if (dest.type == OPERANDREF_REG)
	{
		if ((width != 2) && (width != 4) && (width != 8))
		{
			uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#ifdef OUTPUT32
			EMIT_RRI(imul_32, temp, b.reg, width);
#else
			EMIT_RRI(imul_64, temp, b.reg, width);
#endif
			EMIT_RM(lea, dest.reg, X86_SYM_MEM_INDEX(a.reg, temp, 1, SYMREG_NONE, 0));
		}
		else
		{
			EMIT_RM(lea, dest.reg, X86_SYM_MEM_INDEX(a.reg, b.reg, width, SYMREG_NONE, 0));
		}
	}
	else
	{
		uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if ((width != 2) && (width != 4) && (width != 8))
		{
#ifdef OUTPUT32
			EMIT_RRI(imul_32, temp, b.reg, width);
			EMIT_RR(add_32, temp, a.reg);
			EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp);
#else
			EMIT_RRI(imul_64, temp, b.reg, width);
			EMIT_RR(add_64, temp, a.reg);
			EMIT_MR(mov_64, X86_MEM_REF(dest.mem), temp);
#endif
		}
		else
		{
			EMIT_RM(lea, temp, X86_SYM_MEM_INDEX(a.reg, b.reg, width, SYMREG_NONE, 0));
#ifdef OUTPUT32
			EMIT_MR(mov_32, X86_MEM_REF(dest.mem), temp);
#else
			EMIT_MR(mov_64, X86_MEM_REF(dest.mem), temp);
#endif
		}
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GeneratePtrDiff(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateAdd(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateSub(SymInstrBlock* out, const ILInstruction& instr)
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


#ifdef OUTPUT32
bool OUTPUT_CLASS_NAME::Mult64(SymInstrBlock* out, const OperandReference& dest,
	const OperandReference& a, const OperandReference& b)
{
	uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
	uint32_t eax = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	OperandReference result;
	result.type = OPERANDREF_REG;
	result.width = 8;
	result.reg = eax;
	result.highReg = edx;

	// Multiply low half of both and store 64-bit result in EDX:EAX
	if (a.type == OPERANDREF_REG)
		EMIT_RR(mov_32, eax, a.reg);
	else
		EMIT_RM(mov_32, eax, X86_MEM_REF(a.mem));
	if (b.type == OPERANDREF_REG)
		out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 32, R)(b.reg, eax, edx));
	else
		out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 32, M)(X86_MEM_REF(b.mem), eax, edx));

	// Multiply low half of a and high half of b and add to EDX
	if (a.type == OPERANDREF_REG)
		EMIT_RR(mov_32, temp, a.reg);
	else
		EMIT_RM(mov_32, temp, X86_MEM_REF(a.mem));
	if (b.type == OPERANDREF_REG)
		EMIT_RR(imul_32, temp, b.highReg);
	else
		EMIT_RM(imul_32, temp, X86_MEM_REF_OFFSET(b.mem, 4));
	EMIT_RR(add_32, edx, temp);

	// Multiply high half of a and low half of b and add to EDX
	if (a.type == OPERANDREF_REG)
		EMIT_RR(mov_32, temp, a.highReg);
	else
		EMIT_RM(mov_32, temp, X86_MEM_REF_OFFSET(a.mem, 4));
	if (b.type == OPERANDREF_REG)
		EMIT_RR(imul_32, temp, b.reg);
	else
		EMIT_RM(imul_32, temp, X86_MEM_REF(b.mem));
	EMIT_RR(add_32, edx, temp);

	// Store EDX:EAX into result
	return Move(out, dest, result);
}
#endif


bool OUTPUT_CLASS_NAME::GenerateSignedMult(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#ifdef OUTPUT32
		if (temp.width == 8)
			temp.highReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#endif
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

#ifdef OUTPUT32
	if (a.width == 8)
		return Mult64(out, dest, a, b);
#endif

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = m_symFunc->AddRegister(X86REGCLASS_EAX);
	if (!Move(out, eax, a))
		return false;

	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 8, R)(b.reg, eax.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 8, M)(X86_MEM_REF(b.mem), eax.reg));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 16, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 16, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 32, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 32, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 64, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(imul, 64, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedMult(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#ifdef OUTPUT32
		if (temp.width == 8)
			temp.highReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#endif
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

#ifdef OUTPUT32
	if (a.width == 8)
		return Mult64(out, dest, a, b);
#endif

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = m_symFunc->AddRegister(X86REGCLASS_EAX);
	if (!Move(out, eax, a))
		return false;

	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 8, R)(b.reg, eax.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 8, M)(X86_MEM_REF(b.mem), eax.reg));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 16, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 16, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 32, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 32, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 64, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(mul, 64, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateSignedDiv(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
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
	eax.reg = m_symFunc->AddRegister(X86REGCLASS_EAX);
	if (!Move(out, eax, a))
		return false;

	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	switch (a.width)
	{
	case 1:
		EMIT_RR(movsx_16_8, eax.reg, eax.reg);
		break;
	case 2:
		out->AddInstruction(X86_SYMINSTR_NAME(cwd)(eax.reg, edx));
		break;
	case 4:
		out->AddInstruction(X86_SYMINSTR_NAME(cdq)(eax.reg, edx));
		break;
#ifdef OUTPUT64
	case 8:
		out->AddInstruction(X86_SYMINSTR_NAME(cqo)(eax.reg, edx));
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 8, R)(b.reg, eax.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 8, M)(X86_MEM_REF(b.mem), eax.reg));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 16, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 16, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 32, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 32, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 64, R)(b.reg, eax.reg, edx));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 64, M)(X86_MEM_REF(b.mem), eax.reg, edx));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedDiv(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	uint32_t eaxReg = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t edxReg = m_symFunc->AddRegister(X86REGCLASS_EDX);
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	switch (a.width)
	{
	case 1:
		EMIT_RR(movzx_16_8, eaxReg, eaxReg);
		break;
	case 2:
		EMIT_RR(xor_16, edxReg, edxReg);
		break;
	case 4:
		EMIT_RR(xor_32, edxReg, edxReg);
		break;
#ifdef OUTPUT64
	case 8:
		EMIT_RR(xor_64, edxReg, edxReg);
		break;
#endif
	default:
		return false;
	}

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = eaxReg;
	if (!Move(out, eax, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 8, R)(b.reg, eax.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 8, M)(X86_MEM_REF(b.mem), eax.reg));
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 16, R)(b.reg, eax.reg, edxReg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 16, M)(X86_MEM_REF(b.mem), eax.reg, edxReg));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 32, R)(b.reg, eax.reg, edxReg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 32, M)(X86_MEM_REF(b.mem), eax.reg, edxReg));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 64, R)(b.reg, eax.reg, edxReg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 64, M)(X86_MEM_REF(b.mem), eax.reg, edxReg));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, eax);
}


bool OUTPUT_CLASS_NAME::GenerateSignedMod(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
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
	eax.reg = m_symFunc->AddRegister(X86REGCLASS_EAX);
	if (!Move(out, eax, a))
		return false;

	OperandReference edx;
	edx.type = OPERANDREF_REG;
	edx.width = a.width;
	edx.reg = m_symFunc->AddRegister(X86REGCLASS_EDX);

	switch (a.width)
	{
	case 1:
		edx.reg = eax.reg;
		EMIT_RR(movsx_16_8, eax.reg, eax.reg);
		break;
	case 2:
		out->AddInstruction(X86_SYMINSTR_NAME(cwd)(eax.reg, edx.reg));
		break;
	case 4:
		out->AddInstruction(X86_SYMINSTR_NAME(cdq)(eax.reg, edx.reg));
		break;
#ifdef OUTPUT64
	case 8:
		out->AddInstruction(X86_SYMINSTR_NAME(cqo)(eax.reg, edx.reg));
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
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 8, R)(b.reg, eax.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 8, M)(X86_MEM_REF(b.mem), eax.reg));
		EMIT_RI(shr_16, eax.reg, 8);
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 16, R)(b.reg, eax.reg, edx.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 16, M)(X86_MEM_REF(b.mem), eax.reg, edx.reg));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 32, R)(b.reg, eax.reg, edx.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 32, M)(X86_MEM_REF(b.mem), eax.reg, edx.reg));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 64, R)(b.reg, eax.reg, edx.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(idiv, 64, M)(X86_MEM_REF(b.mem), eax.reg, edx.reg));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, edx);
}


bool OUTPUT_CLASS_NAME::GenerateUnsignedMod(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], a))
		return false;
	if (!PrepareLoad(out, instr.params[2], b))
		return false;

	OperandReference eax;
	eax.type = OPERANDREF_REG;
	eax.width = a.width;
	eax.reg = m_symFunc->AddRegister(X86REGCLASS_EAX);

	OperandReference edx;
	edx.type = OPERANDREF_REG;
	edx.width = a.width;
	edx.reg = m_symFunc->AddRegister(X86REGCLASS_EDX);

	switch (a.width)
	{
	case 1:
		edx.reg = eax.reg;
		EMIT_RR(movzx_16_8, eax.reg, eax.reg);
		break;
	case 2:
		EMIT_RR(xor_16, edx.reg, edx.reg);
		break;
	case 4:
		EMIT_RR(xor_32, edx.reg, edx.reg);
		break;
#ifdef OUTPUT64
	case 8:
		EMIT_RR(xor_64, edx.reg, edx.reg);
		break;
#endif
	default:
		return false;
	}

	if (!Move(out, eax, a))
		return false;

	if (b.type == OPERANDREF_IMMED)
	{
		OperandReference temp;
		temp.type = OPERANDREF_REG;
		temp.width = a.width;
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		if (!Move(out, temp, b))
			return false;
		b = temp;
	}

	switch (a.width)
	{
	case 1:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 8, R)(b.reg, eax.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 8, M)(X86_MEM_REF(b.mem), eax.reg));
		EMIT_RI(shr_16, eax.reg, 8);
		break;
	case 2:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 16, R)(b.reg, eax.reg, edx.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 16, M)(X86_MEM_REF(b.mem), eax.reg, edx.reg));
		break;
	case 4:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 32, R)(b.reg, eax.reg, edx.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 32, M)(X86_MEM_REF(b.mem), eax.reg, edx.reg));
		break;
#ifdef OUTPUT64
	case 8:
		if (b.type == OPERANDREF_REG)
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 64, R)(b.reg, eax.reg, edx.reg));
		else
			out->AddInstruction(X86_SYMINSTR_NAME_SIZE_OP(div, 64, M)(X86_MEM_REF(b.mem), eax.reg, edx.reg));
		break;
#endif
	default:
		return false;
	}

	return Move(out, dest, edx);
}


bool OUTPUT_CLASS_NAME::GenerateAnd(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateOr(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateXor(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateShl(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b, count;
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.reg = m_symFunc->AddRegister(X86REGCLASS_ECX);

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


bool OUTPUT_CLASS_NAME::GenerateShr(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b, count;
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.reg = m_symFunc->AddRegister(X86REGCLASS_ECX);

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


bool OUTPUT_CLASS_NAME::GenerateSar(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, a, b, count;
	count.type = OPERANDREF_REG;
	count.width = 1;
	count.reg = m_symFunc->AddRegister(X86REGCLASS_ECX);

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


bool OUTPUT_CLASS_NAME::GenerateNeg(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateNot(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateIfTrue(SymInstrBlock* out, const ILInstruction& instr)
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
		uint32_t temp; \
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
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
		uint32_t temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(cmp_64, X86_MEM_REF(left.mem), right.reg); break; \
		case OPERANDREF_MEM: \
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
		uint32_t temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG: \
			EMIT_MR(cmp_32, X86_MEM_REF_OFFSET(left.mem, 4), right.highReg); \
			ConditionalJump(out, inverse, instr.params[3].block, NULL); \
			EMIT_MR(cmp_32, X86_MEM_REF(left.mem), right.reg); \
			ConditionalJump(out, pred, instr.params[2].block, instr.params[3].block); \
			break; \
		case OPERANDREF_MEM: \
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
		uint32_t temp; \
		switch (right.type) \
		{ \
		case OPERANDREF_REG:  EMIT_MR(cmp_64, X86_MEM_REF(left.mem), right.reg); break; \
		case OPERANDREF_MEM: \
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
			uint32_t temp; \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(cmp_8, X86_MEM_REF(left.mem), right.reg); break; \
			case OPERANDREF_MEM: \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT); \
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
			uint32_t temp; \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(cmp_16, X86_MEM_REF(left.mem), right.reg); break; \
			case OPERANDREF_MEM: \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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
			uint32_t temp; \
			switch (right.type) \
			{ \
			case OPERANDREF_REG:  EMIT_MR(cmp_32, X86_MEM_REF(left.mem), right.reg); break; \
			case OPERANDREF_MEM: \
				temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER); \
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

bool OUTPUT_CLASS_NAME::GenerateIfLessThan(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_LESS_THAN, CONDJUMP_BELOW, CONDJUMP_LESS_THAN, CONDJUMP_GREATER_THAN);
}


bool OUTPUT_CLASS_NAME::GenerateIfLessThanEqual(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_LESS_EQUAL, CONDJUMP_BELOW_EQUAL, CONDJUMP_LESS_THAN, CONDJUMP_GREATER_THAN);
}


bool OUTPUT_CLASS_NAME::GenerateIfBelow(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_BELOW, CONDJUMP_BELOW, CONDJUMP_BELOW, CONDJUMP_ABOVE);
}


bool OUTPUT_CLASS_NAME::GenerateIfBelowEqual(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_ORDERED_COMPARE(CONDJUMP_BELOW_EQUAL, CONDJUMP_BELOW_EQUAL, CONDJUMP_BELOW, CONDJUMP_ABOVE);
}


bool OUTPUT_CLASS_NAME::GenerateIfEqual(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!PrepareLoad(out, instr.params[0], left))
		return false;
	if (!PrepareLoad(out, instr.params[1], right))
		return false;
	IMPLEMENT_UNORDERED_COMPARE(CONDJUMP_EQUAL, CONDJUMP_NOT_EQUAL);
}


bool OUTPUT_CLASS_NAME::GenerateGoto(SymInstrBlock* out, const ILInstruction& instr)
{
	if (instr.params[0].cls != ILPARAM_BLOCK)
	{
		OperandReference target;
		if (!PrepareLoad(out, instr.params[0], target))
			return false;

		uint32_t temp;
		switch (target.type)
		{
		case OPERANDREF_REG:
			EMIT_R(jmpn, target.reg);
			break;
		case OPERANDREF_MEM:
			EMIT_M(jmpn, X86_MEM_REF(target.mem));
			break;
		case OPERANDREF_IMMED:
			temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#ifdef OUTPUT32
			EMIT_RI(mov_32, temp, target.immed);
#else
			EMIT_RI(mov_64, temp, target.immed);
#endif
			EMIT_R(jmpn, temp);
			break;
		default:
			return false;
		}

		return true;
	}

	UnconditionalJump(out, instr.params[0].block);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateCall(SymInstrBlock* out, const ILInstruction& instr)
{
	size_t pushSize = 0;

	// First try to place parameters in registers
#ifdef OUTPUT32
	uint32_t intParamRegs[4] = {X86REGCLASS_INTEGER_PARAM_0, X86REGCLASS_INTEGER_PARAM_1, X86REGCLASS_INTEGER_PARAM_2, SYMREG_NONE};
#else
	uint32_t intParamRegs[7] = {X86REGCLASS_INTEGER_PARAM_0, X86REGCLASS_INTEGER_PARAM_1, X86REGCLASS_INTEGER_PARAM_2,
		X86REGCLASS_INTEGER_PARAM_3, X86REGCLASS_INTEGER_PARAM_4, X86REGCLASS_INTEGER_PARAM_5, SYMREG_NONE};
#endif
	uint32_t curIntParamReg = 0;
	vector<bool> paramOnStack;
	vector<uint32_t> reads;

	for (size_t i = 3; i < instr.params.size(); i++)
	{
		if ((i - 3) >= (size_t)instr.params[2].integerValue)
		{
			// Additional parameters to variable argument functions must be on stack
			paramOnStack.push_back(true);
			continue;
		}

		size_t width = instr.params[i].GetWidth();
		if (width == 0)
		{
			// Indefinite width (used for immediates, for example), use native size
#ifdef OUTPUT32
			width = 4;
#else
			width = 8;
#endif
		}

		bool stackParam = false;
#ifdef OUTPUT32
		if (width == 8)
		{
			if ((intParamRegs[curIntParamReg] == SYMREG_NONE) || (intParamRegs[curIntParamReg + 1] == SYMREG_NONE))
				stackParam = true;
			else
			{
				OperandReference dest, src;
				if (!PrepareLoad(out, instr.params[i], src))
					return false;

				dest.type = OPERANDREF_REG;
				dest.width = src.width;
				dest.reg = m_symFunc->AddRegister(intParamRegs[curIntParamReg++]);
				dest.highReg = m_symFunc->AddRegister(intParamRegs[curIntParamReg++]);
				if (!Move(out, dest, src))
					return false;
				reads.push_back(dest.reg);
				reads.push_back(dest.highReg);
			}
		}
		else
#endif
		{
			if (intParamRegs[curIntParamReg] == SYMREG_NONE)
				stackParam = true;
			else
			{
				OperandReference dest, src;
				if (!PrepareLoad(out, instr.params[i], src))
					return false;

				dest.type = OPERANDREF_REG;
				dest.width = src.width;
				dest.reg = m_symFunc->AddRegister(intParamRegs[curIntParamReg++]);
				if (!Move(out, dest, src))
					return false;
				reads.push_back(dest.reg);
			}
		}

		paramOnStack.push_back(stackParam);
	}

	// Push parameters from right to left
	for (size_t i = instr.params.size() - 1; i >= 3; i--)
	{
		if (!paramOnStack[i - 3])
			continue;

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
#else
			size_t paramSize = (param.width + 7) & (~7);
#endif
			EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? paramSize : -paramSize));

			OperandReference dest;
			dest.type = OPERANDREF_MEM;
			dest.width = param.width;
			dest.mem.base = SYMREG_SP;
			dest.mem.index = SYMREG_NONE;
			dest.mem.scale = 1;
			dest.mem.var = SYMREG_NONE;
			if (m_settings.stackGrowsUp)
			{
#ifdef OUTPUT32
				dest.mem.offset = 4 - paramSize;
#else
				dest.mem.offset = 8 - paramSize;
#endif
			}
			else
			{
				dest.mem.offset = 0;
			}

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
					uint32_t reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
			EMIT_R(push, param.reg);
		else if (param.type == OPERANDREF_IMMED)
			EMIT_I(push, (int32_t)param.immed);
		else
		{
			// Load into register and then push native size
			OperandReference temp;
			temp.type = OPERANDREF_REG;
			temp.width = param.width;
			temp.reg = m_symFunc->AddRegister((param.width == 1) ? X86REGCLASS_INTEGER_8BIT : X86REGCLASS_INTEGER);
			if (!Move(out, temp, param))
				return false;

			EMIT_R(push, temp.reg);
		}

#ifdef OUTPUT32
		pushSize += 4;
#else
		pushSize += 8;
#endif
	}

	uint32_t keyReg = SYMREG_NONE;
	if (m_settings.encodePointers)
	{
		keyReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		ILParameter keyParam(m_settings.encodePointerKey);
		OperandReference key, keyDest;
		keyDest.type = OPERANDREF_REG;
#ifdef OUTPUT32
		keyDest.width = 4;
#else
		keyDest.width = 8;
#endif
		keyDest.reg = keyReg;
		if (!PrepareLoad(out, keyParam, key))
			return false;
		if (!Move(out, keyDest, key))
			return false;
	}

	uint32_t retValReg = SYMREG_NONE;
	uint32_t retValHighReg = SYMREG_NONE;
	if (instr.params[0].cls != ILPARAM_VOID)
	{
		retValReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_RETURN_VALUE);
#ifdef OUTPUT32
		if (instr.params[0].GetWidth() > 4)
			retValHighReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_RETURN_VALUE_HIGH);
#endif
	}

	// Perform function call
	uint32_t scratch = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
	if (instr.params[1].cls == ILPARAM_FUNC)
	{
		out->AddInstruction(X86_SYMINSTR_NAME(CallDirect)(instr.params[1].function, instr.params[1].function->GetIL()[0],
			retValReg, retValHighReg, keyReg, scratch, reads));
	}
	else
	{
		OperandReference func;
		if (!PrepareLoad(out, instr.params[1], func))
			return false;

		switch (func.type)
		{
		case OPERANDREF_REG:
			out->AddInstruction(X86_SYMINSTR_NAME(CallIndirectReg)(func.reg, retValReg, retValHighReg, keyReg, scratch, reads));
			break;
		case OPERANDREF_MEM:
			out->AddInstruction(X86_SYMINSTR_NAME(CallIndirectMem)(X86_MEM_REF(func.mem), retValReg, retValHighReg,
				keyReg, scratch, reads));
			break;
		default:
			return false;
		}
	}

	// Adjust stack pointer to pop off parameters
	if (pushSize != 0)
	{
		if (m_settings.stackGrowsUp)
		{
#ifdef OUTPUT32
			EMIT_RI(sub_32, SYMREG_SP, pushSize);
#else
			EMIT_RI(sub_64, SYMREG_SP, pushSize);
#endif
		}
		else
		{
#ifdef OUTPUT32
			EMIT_RI(add_32, SYMREG_SP, pushSize);
#else
			EMIT_RI(add_64, SYMREG_SP, pushSize);
#endif
		}
	}

	// Store return value, if there is one
	if (instr.params[0].cls != ILPARAM_VOID)
	{
		OperandReference dest, retVal;
		if (!PrepareStore(out, instr.params[0], dest))
			return false;

		retVal.type = OPERANDREF_REG;
		retVal.width = dest.width;
		retVal.reg = retValReg;
		retVal.highReg = retValHighReg;
		if (!Move(out, dest, retVal))
			return false;
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateSignedConvert(SymInstrBlock* out, const ILInstruction& instr)
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
			// Source register may not be able to be referenced as a single byte, must go
			// through a temporary to make this work, or transfer more than
			// necessary if the destination is also a register
			if (dest.type == OPERANDREF_REG)
			{
				// Destination is a register, transfer all four bytes of the register
				src.width = 4;
				dest.width = 4;
			}
			else
			{
				// Destination is not a register, must go through a temporary
				OperandReference tmp;
				tmp.type = OPERANDREF_REG;
				tmp.width = 4;
				tmp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT);
				src.width = 4;
				if (!Move(out, tmp, src))
					return false;
				tmp.width = 1;
				return Move(out, dest, tmp);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				EMIT_RR(mov_32, dest.highReg, dest.reg);
				EMIT_RI(sar_32, dest.highReg, 31);
				return true;
			}
			else
			{
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				EMIT_RR(mov_32, dest.highReg, dest.reg);
				EMIT_RI(sar_32, dest.highReg, 31);
				return true;
			}
			else
			{
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				EMIT_RR(mov_32, dest.highReg, dest.reg);
				EMIT_RI(sar_32, dest.highReg, 31);
				return true;
			}
			else
			{
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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


bool OUTPUT_CLASS_NAME::GenerateUnsignedConvert(SymInstrBlock* out, const ILInstruction& instr)
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
			// Source register may not be able to be referenced as a single byte, must go
			// through a temporary to make this work, or transfer more than
			// necessary if the destination is also a register
			if (dest.type == OPERANDREF_REG)
			{
				// Destination is a register, transfer all four bytes of the register
				src.width = 4;
				dest.width = 4;
			}
			else
			{
				// Destination is not a register, must go through a temporary
				OperandReference tmp;
				tmp.type = OPERANDREF_REG;
				tmp.width = 4;
				tmp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT);
				src.width = 4;
				if (!Move(out, tmp, src))
					return false;
				tmp.width = 1;
				return Move(out, dest, tmp);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
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
			{
				if (src.type == OPERANDREF_REG)
					EMIT_RR(mov_32, dest.reg, src.reg);
				else
					EMIT_RM(mov_32, dest.reg, X86_MEM_REF(src.mem));
				EMIT_RR(xor_32, dest.highReg, dest.highReg);
			}
			else
			{
				if (src.type == OPERANDREF_REG)
					EMIT_MR(mov_32, X86_MEM_REF(dest.mem), src.reg);
				else
				{
					uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
					EMIT_RM(mov_32, tmp, X86_MEM_REF(src.mem));
					EMIT_MR(mov_32, X86_MEM_REF(dest.mem), tmp);
				}
				EMIT_MI(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), 0);
			}
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
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_8, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_8, tmp, src.reg);
				else
					EMIT_RM(movzx_32_8, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 2)
		{
			if (dest.type == OPERANDREF_REG)
			{
				dest.width = 4;
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, dest.reg, src.reg);
				else
					EMIT_RM(movzx_32_16, dest.reg, X86_MEM_REF(src.mem));
				return true;
			}
			else
			{
				uint32_t tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
				if (src.type == OPERANDREF_REG)
					EMIT_RR(movzx_32_16, tmp, src.reg);
				else
					EMIT_RM(movzx_32_16, tmp, X86_MEM_REF(src.mem));
				EMIT_MR(mov_64, X86_MEM_REF(dest.mem), tmp);
				return true;
			}
		}
		else if (src.width == 4)
		{
			if (dest.type == OPERANDREF_REG)
			{
				dest.width = 4;
				return Move(out, dest, src);
			}
			else
			{
				uint32_t tmp;
				if (src.type == OPERANDREF_REG)
					tmp = src.reg;
				else
				{
					tmp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
					EMIT_RM(mov_32, tmp, X86_MEM_REF(src.mem));
				}
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


bool OUTPUT_CLASS_NAME::GenerateReturn(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	dest.type = OPERANDREF_REG;
	dest.width = instr.params[0].GetWidth();
	dest.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_RETURN_VALUE);
	dest.highReg = SYMREG_NONE;
#ifdef OUTPUT32
	if (dest.width == 8)
		dest.highReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_RETURN_VALUE_HIGH);
#endif

	if (!PrepareLoad(out, instr.params[0], src))
		return false;
	if (!Move(out, dest, src))
		return false;
	if (!GenerateReturnVoid(out, instr))
		return false;

	out->AddInstruction(X86_SYMINSTR_NAME(SymReturn)(dest.reg, dest.highReg));
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateReturnVoid(SymInstrBlock* out, const ILInstruction& instr)
{
	uint32_t temp = m_symFunc->AddRegister(X86REGCLASS_INTEGER);

	if (m_settings.encodePointers)
	{
		// Using encoded pointers, load decode key
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
		tempRef.reg = temp;
		if (!Move(out, tempRef, key))
			return false;
	}

	if (m_framePointerEnabled)
	{
		if ((m_settings.framePointer == DEFAULT_FRAME_POINTER) && m_normalStack)
			EMIT(leave);
		else if (m_normalStack)
		{
#ifdef OUTPUT32
			EMIT_RR(mov_32, SYMREG_SP, SYMREG_BP);
#else
			EMIT_RR(mov_64, SYMREG_SP, SYMREG_BP);
#endif
			EMIT_R(pop, SYMREG_BP);
		}
		else
		{
#ifdef OUTPUT32
			EMIT_RR(mov_32, SYMREG_SP, SYMREG_BP);
			EMIT_RM(mov_32, SYMREG_BP, X86_SYM_MEM(SYMREG_SP, 0));
			EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? -4 : 4));
#else
			EMIT_RR(mov_64, SYMREG_SP, SYMREG_BP);
			EMIT_RM(mov_64, SYMREG_BP, X86_SYM_MEM(SYMREG_SP, 0));
			EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? -8 : 8));
#endif
		}
	}
	else
	{
		// TODO: Support frames without a frame pointer
		return false;
	}

	out->AddInstruction(X86_SYMINSTR_NAME(RestoreCalleeSavedRegs)());

	if (m_normalStack)
	{
		if (m_settings.encodePointers)
		{
			// Using encoded pointers, decode return address before returning
#ifdef OUTPUT32
			EMIT_MR(xor_32, X86_SYM_MEM(SYMREG_SP, 0), temp);
#else
			EMIT_MR(xor_64, X86_SYM_MEM(SYMREG_SP, 0), temp);
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
			EMIT_RM(xor_32, temp, X86_SYM_MEM(SYMREG_SP, 0));
#else
			EMIT_RM(xor_64, temp, X86_SYM_MEM(SYMREG_SP, 0));
#endif
		}
		else
		{
#ifdef OUTPUT32
			EMIT_RM(mov_32, temp, X86_SYM_MEM(SYMREG_SP, 0));
#else
			EMIT_RM(mov_64, temp, X86_SYM_MEM(SYMREG_SP, 0));
#endif
		}

#ifdef OUTPUT32
		EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? -4 : 4));
#else
		EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? -8 : 8));
#endif
		EMIT_R(jmpn, temp);
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateAlloca(SymInstrBlock* out, const ILInstruction& instr)
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
	stack.reg = SYMREG_SP;

	if (m_settings.stackGrowsUp)
	{
#ifdef OUTPUT32
		EMIT_RI(add_32, SYMREG_SP, 4);
#else
		EMIT_RI(add_32, SYMREG_SP, 8);
#endif

		if (!Move(out, dest, stack))
			return false;
		if (!Add(out, stack, size))
			return false;

		EMIT_R(dec_32, SYMREG_SP);
#ifdef OUTPUT32
		EMIT_RI(and_32, SYMREG_SP, ~3);
#else
		EMIT_RI(and_64, SYMREG_SP, ~7);
#endif

		return true;
	}
	else
	{
		if (!Sub(out, stack, size))
			return false;

#ifdef OUTPUT32
		EMIT_RI(and_32, SYMREG_SP, ~3);
#else
		EMIT_RI(and_64, SYMREG_SP, ~7);
#endif

		return Move(out, dest, stack);
	}
}


bool OUTPUT_CLASS_NAME::GenerateMemcpy(SymInstrBlock* out, const ILInstruction& instr)
{
	uint32_t esi = m_symFunc->AddRegister(X86REGCLASS_ESI);
	uint32_t edi = m_symFunc->AddRegister(X86REGCLASS_EDI);
	uint32_t ecx = m_symFunc->AddRegister(X86REGCLASS_ECX);
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
		EMIT_RR(mov_32, esi, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, esi, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, esi, src.immed);
		break;
	default:
		return false;
	}

	switch (dest.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_32, edi, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, edi, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, edi, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_32, ecx, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, ecx, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, ecx, size.immed);
		break;
	default:
		return false;
	}
#else
	switch (src.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_64, esi, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, esi, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, esi, src.immed);
		break;
	default:
		return false;
	}

	switch (dest.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_64, edi, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, edi, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, edi, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_64, ecx, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, ecx, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, ecx, size.immed);
		break;
	default:
		return false;
	}
#endif

	out->AddInstruction(X86_SYMINSTR_NAME(rep_movsb)(edi, esi, ecx));
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateMemset(SymInstrBlock* out, const ILInstruction& instr)
{
	uint32_t edi = m_symFunc->AddRegister(X86REGCLASS_EDI);
	uint32_t eax = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t ecx = m_symFunc->AddRegister(X86REGCLASS_ECX);
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
		EMIT_RR(mov_8, eax, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_8, eax, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_8, eax, src.immed);
		break;
	default:
		return false;
	}

#ifdef OUTPUT32
	switch (dest.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_32, edi, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, edi, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, edi, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_32, ecx, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, ecx, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, ecx, size.immed);
		break;
	default:
		return false;
	}
#else
	switch (dest.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_64, edi, dest.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, edi, X86_MEM_REF(dest.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, edi, dest.immed);
		break;
	default:
		return false;
	}

	switch (size.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_64, ecx, size.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, ecx, X86_MEM_REF(size.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, ecx, size.immed);
		break;
	default:
		return false;
	}
#endif

	out->AddInstruction(X86_SYMINSTR_NAME(rep_stosb)(edi, eax, ecx));
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateStrlen(SymInstrBlock* out, const ILInstruction& instr)
{
	uint32_t edi = m_symFunc->AddRegister(X86REGCLASS_EDI);
	uint32_t eax = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t ecx = m_symFunc->AddRegister(X86REGCLASS_ECX);
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

#ifdef OUTPUT32
	switch (src.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_32, edi, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_32, edi, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_32, edi, src.immed);
		break;
	default:
		return false;
	}
#else
	switch (src.type)
	{
	case OPERANDREF_REG:
		EMIT_RR(mov_64, edi, src.reg);
		break;
	case OPERANDREF_MEM:
		EMIT_RM(mov_64, edi, X86_MEM_REF(src.mem));
		break;
	case OPERANDREF_IMMED:
		EMIT_RI(mov_64, edi, src.immed);
		break;
	default:
		return false;
	}
#endif

	EMIT_RR(xor_8, eax, eax);
	EMIT_RR(xor_32, ecx, ecx);
#ifdef OUTPUT32
	EMIT_R(dec_32, ecx);
#else
	EMIT_R(dec_64, ecx);
#endif

	out->AddInstruction(X86_SYMINSTR_NAME(repne_scasb)(edi, eax, ecx));

	OperandReference ecxOperand;
	ecxOperand.type = OPERANDREF_REG;
	EMIT_R(not_32, ecx);
	EMIT_R(dec_32, ecx);
#ifdef OUTPUT32
	ecxOperand.width = 4;
#else
	ecxOperand.width = 8;
#endif
	ecxOperand.reg = ecx;

	return Move(out, dest, ecxOperand);
}


bool OUTPUT_CLASS_NAME::GenerateSyscall(SymInstrBlock* out, const ILInstruction& instr, bool twoDest)
{
	size_t destCount = twoDest ? 2 : 1;

#ifdef OUTPUT32
	if ((m_settings.os == OS_FREEBSD) || (m_settings.os == OS_MAC))
	{
		// FreeBSD expects syscall parameters on a normal stack
		if (!m_normalStack)
			return false;

		size_t pushSize = 0;

		// Push parameters from right to left
		for (size_t i = instr.params.size() - 1; i >= 1 + destCount; i--)
		{
			OperandReference param;
			if (!PrepareLoad(out, instr.params[i], param))
				return false;

			if (param.width == 0)
			{
				// Indefinite width (used for immediates, for example), use native size
				param.width = 4;
			}

			// Check for native size parameters
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

			// Not native size
			if (param.type == OPERANDREF_REG)
			{
				// Push native size (upper bits are ignored)
				EMIT_R(push, param.reg);
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
				temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
				if (!Move(out, temp, param))
					return false;

				EMIT_R(push, temp.reg);
			}

			pushSize += 4;
		}

		// Place syscall number into EAX
		OperandReference syscallNum;
		if (!PrepareLoad(out, instr.params[destCount], syscallNum))
			return false;

		OperandReference eax;
		eax.type = OPERANDREF_REG;
		eax.width = 4;
		eax.reg = m_symFunc->AddRegister(X86REGCLASS_EAX);
		if (!Move(out, eax, syscallNum))
			return false;

		// An extra parameter needs to be pushed (it is ignored)
		EMIT_R(push, eax.reg);
		pushSize += 4;

		uint32_t resultReg1 = m_symFunc->AddRegister(X86REGCLASS_SYSCALL_RESULT_1);
		uint32_t resultReg2 = m_symFunc->AddRegister(X86REGCLASS_SYSCALL_RESULT_2);
		uint32_t ecx = m_symFunc->AddRegister(X86REGCLASS_ECX);

		vector<uint32_t> readRegs, spilledRegs; // Parameters are passed in on stack, so no read registers
		out->AddInstruction(X86_SYMINSTR_NAME(SyscallInt80)(resultReg1, resultReg2, ecx, readRegs, spilledRegs));
		out->AddInstruction(X86_SYMINSTR_NAME(SyscallCorrectErrorCode)(resultReg1));

		// Adjust stack pointer to pop off parameters
		if (pushSize != 0)
			EMIT_RI(add_32, SYMREG_SP, pushSize);

		if (twoDest)
		{
			OperandReference dest, result;
			if (!PrepareStore(out, instr.params[1], dest))
				return false;
			result.type = OPERANDREF_REG;
			result.width = dest.width;
			result.reg = resultReg2;
			if (!Move(out, dest, result))
				return false;
		}

		OperandReference dest, result;
		if (!PrepareStore(out, instr.params[0], dest))
			return false;
		result.type = OPERANDREF_REG;
		result.width = dest.width;
		result.reg = resultReg1;
		return Move(out, dest, result);
	}
#endif

	if ((m_settings.os != OS_LINUX) && (m_settings.os != OS_FREEBSD) && (m_settings.os != OS_MAC))
		return false;

	static const X86RegisterClass regs[] = {X86REGCLASS_SYSCALL_PARAM_0, X86REGCLASS_SYSCALL_PARAM_1, X86REGCLASS_SYSCALL_PARAM_2,
		X86REGCLASS_SYSCALL_PARAM_3, X86REGCLASS_SYSCALL_PARAM_4, X86REGCLASS_SYSCALL_PARAM_5, X86REGCLASS_SYSCALL_PARAM_6};

	vector<uint32_t> readRegs;
	vector<uint32_t> spilledRegs;
	size_t regIndex = 0;
	for (size_t i = destCount; i < instr.params.size(); i++)
	{
		if (regIndex >= 7)
			return false;
		if (instr.params[i].cls == ILPARAM_UNDEFINED)
			continue;

#ifdef OUTPUT32
		if (instr.params[i].GetWidth() == 8)
		{
			if ((regIndex + 1) >= 7)
				return false;
		}
#endif

		OperandReference cur;
		if (!PrepareLoad(out, instr.params[i], cur))
			return false;

		uint32_t regClass, highRegClass;
#ifdef OUTPUT32
		if (instr.params[i].GetWidth() == 8)
		{
			regClass = regs[regIndex++];
			highRegClass = regs[regIndex++];
		}
		else
		{
			regClass = regs[regIndex++];
			highRegClass = SYMREG_NONE;
		}
#else
		regClass = regs[regIndex++];
		highRegClass = SYMREG_NONE;
#endif

		if (((regClass != SYMREG_NONE) && (m_symFunc->DoesRegisterClassConflictWithSpecialRegisters(regClass))) ||
			((highRegClass != SYMREG_NONE) && (m_symFunc->DoesRegisterClassConflictWithSpecialRegisters(highRegClass))))
		{
			// Target register conflicts with stack or frame pointer, must use stack until ready for syscall
			uint32_t reg, highReg = SYMREG_NONE;
			reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
			spilledRegs.push_back(regClass);
			if (highRegClass != SYMREG_NONE)
			{
				highReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
				spilledRegs.push_back(highRegClass);
			}

			OperandReference dest;
			dest.type = OPERANDREF_REG;
			dest.width = cur.width;
			dest.reg = reg;
			dest.highReg = highReg;

			if (!Move(out, dest, cur))
				return false;

			EMIT_R(push, reg);
			if (highReg != SYMREG_NONE)
				EMIT_R(push, highReg);
		}
		else
		{
			// Parameter can be stored directly in the target register
			uint32_t reg, highReg = SYMREG_NONE;
			reg = m_symFunc->AddRegister(regClass);
			readRegs.push_back(reg);
			if (highRegClass != SYMREG_NONE)
			{
				highReg = m_symFunc->AddRegister(highRegClass);
				readRegs.push_back(highReg);
			}

			OperandReference dest;
			dest.type = OPERANDREF_REG;
			dest.width = cur.width;
			dest.reg = reg;
			dest.highReg = highReg;

			if (!Move(out, dest, cur))
				return false;
		}
	}

	uint32_t resultReg1 = m_symFunc->AddRegister(X86REGCLASS_SYSCALL_RESULT_1);
	uint32_t resultReg2 = m_symFunc->AddRegister(X86REGCLASS_SYSCALL_RESULT_2);
	uint32_t ecx = m_symFunc->AddRegister(X86REGCLASS_ECX);

#ifdef OUTPUT32
	out->AddInstruction(X86_SYMINSTR_NAME(SyscallInt80)(resultReg1, resultReg2, ecx, readRegs, spilledRegs));
#else
	if (m_settings.os == OS_FREEBSD)
		out->AddInstruction(X86_SYMINSTR_NAME(SyscallInt80)(resultReg1, resultReg2, ecx, readRegs, spilledRegs));
	else
		out->AddInstruction(X86_SYMINSTR_NAME(Syscall)(resultReg1, resultReg2, ecx, readRegs, spilledRegs));
#endif

	if ((m_settings.os == OS_FREEBSD) || (m_settings.os == OS_MAC))
		out->AddInstruction(X86_SYMINSTR_NAME(SyscallCorrectErrorCode)(resultReg1));

	if (twoDest)
	{
		OperandReference dest, result;
		if (!PrepareStore(out, instr.params[1], dest))
			return false;
		result.type = OPERANDREF_REG;
		result.width = dest.width;
		result.reg = resultReg2;
		if (!Move(out, dest, result))
			return false;
	}

	OperandReference dest, result;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	result.type = OPERANDREF_REG;
	result.width = dest.width;
	result.reg = resultReg1;
	return Move(out, dest, result);
}


bool OUTPUT_CLASS_NAME::GenerateRdtsc(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	uint32_t eax = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	EMIT_RR(rdtsc_32, eax, edx);

	if (dest.type == OPERANDREF_REG)
	{
#ifdef OUTPUT32
		EMIT_RR(mov_32, dest.reg, eax);
		EMIT_RR(mov_32, dest.highReg, edx);
#else
		EMIT_RR(mov_64, dest.reg, edx);
		EMIT_RI(shl_64, dest.reg, 32);
		EMIT_RR(or_64, dest.reg, eax);
#endif
	}
	else
	{
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), eax);
		EMIT_MR(mov_32, X86_MEM_REF_OFFSET(dest.mem, 4), edx);
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateRdtscLow(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	uint32_t eax = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	EMIT_RR(rdtsc_32, eax, edx);

	if (dest.type == OPERANDREF_REG)
		EMIT_RR(mov_32, dest.reg, eax);
	else
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), eax);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateRdtscHigh(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	uint32_t eax = m_symFunc->AddRegister(X86REGCLASS_EAX);
	uint32_t edx = m_symFunc->AddRegister(X86REGCLASS_EDX);
	EMIT_RR(rdtsc_32, eax, edx);

	if (dest.type == OPERANDREF_REG)
		EMIT_RR(mov_32, dest.reg, edx);
	else
		EMIT_MR(mov_32, X86_MEM_REF(dest.mem), edx);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateInitialVararg(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;

	src.type = OPERANDREF_REG;
#ifdef OUTPUT32
	src.width = 4;
#else
	src.width = 8;
#endif
	src.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);

	EMIT_RM(lea, src.reg, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, m_varargStart, 0));
	return Move(out, dest, src);
}


bool OUTPUT_CLASS_NAME::GenerateNextArg(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GeneratePrevArg(SymInstrBlock* out, const ILInstruction& instr)
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


bool OUTPUT_CLASS_NAME::GenerateByteSwap(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!PrepareLoad(out, instr.params[1], src))
		return false;

	OperandReference temp = dest;
	if (dest.type != OPERANDREF_REG)
	{
		temp.type = OPERANDREF_REG;
		temp.width = dest.width;
#ifdef OUTPUT32
		if (temp.width == 8)
		{
			temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
			temp.highReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		}
		else
		{
			temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
		}
#else
		temp.reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#endif
	}

	if (!Move(out, temp, src))
		return false;

	switch (dest.width)
	{
	case 1:
		break;
	case 2:
		EMIT_RI(rol_16, temp.reg, 8);
		break;
	case 4:
		EMIT_R(bswap_32, temp.reg);
		break;
	case 8:
#ifdef OUTPUT32
		EMIT_RR(xchg_32, temp.reg, temp.highReg);
		EMIT_R(bswap_32, temp.reg);
		EMIT_R(bswap_32, temp.highReg);
#else
		EMIT_R(bswap_64, temp.reg);
#endif
		break;
	default:
		return false;
	}

	if (dest.type != OPERANDREF_REG)
	{
		if (!Move(out, dest, temp))
			return false;
	}

	return true;
}


bool OUTPUT_CLASS_NAME::GenerateBreakpoint(SymInstrBlock* out, const ILInstruction& instr)
{
	EMIT(int3);
	return true;
}


bool OUTPUT_CLASS_NAME::GenerateCodeBlock(SymInstrBlock* out, ILBlock* block)
{
	m_currentBlock = block;

	vector<ILInstruction>::iterator i;
	for (i = block->GetInstructions().begin(); i != block->GetInstructions().end(); i++)
	{
		if (m_settings.antiDisasm && ((rand() % m_settings.antiDisasmFrequency) == 0))
		{
			uint32_t reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_NO_REX);
			out->AddInstruction(X86_SYMINSTR_NAME(AntiDisassembly)(reg));
		}

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
			if (!GenerateSyscall(out, *i, false))
				goto fail;
			break;
		case ILOP_SYSCALL2:
			if (!GenerateSyscall(out, *i, true))
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
		case ILOP_INITIAL_VARARG:
			if (!GenerateInitialVararg(out, *i))
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
		case ILOP_BREAKPOINT:
			if (!GenerateBreakpoint(out, *i))
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
	m_settings.framePointer = DEFAULT_FRAME_POINTER;
	m_settings.stackPointer = DEFAULT_STACK_POINTER;
	m_settings.basePointer = SYMREG_NONE;

	if (m_settings.stackGrowsUp)
	{
		// TODO: Fix this setting
		return false;
	}

	// TODO: Stack variable system does not yet support frames without a frame pointer
	m_framePointerEnabled = true;

#ifdef OUTPUT32
	if (m_settings.stackRegName.size() != 0)
		m_settings.stackPointer = GetRegisterByName(m_settings.stackRegName);
	if (m_settings.frameRegName.size() != 0)
		m_settings.framePointer = GetRegisterByName(m_settings.frameRegName);
#else
	if (m_settings.stackRegName.size() != 0)
		m_settings.stackPointer = GetRegisterByName(m_settings.stackRegName);
	if (m_settings.frameRegName.size() != 0)
		m_settings.framePointer = GetRegisterByName(m_settings.frameRegName);
#endif

	if (m_settings.stackPointer == SYMREG_NONE)
	{
		fprintf(stderr, "error: invalid stack pointer register\n");
		return false;
	}

	if (m_settings.framePointer == SYMREG_NONE)
	{
		fprintf(stderr, "error: invalid frame pointer register\n");
		return false;
	}

	if (m_settings.stackPointer == m_settings.framePointer)
	{
		fprintf(stderr, "error: stack pointer and frame pointer cannot be the same register\n");
		return false;
	}

	m_normalStack = true;
	if ((m_settings.stackPointer != DEFAULT_STACK_POINTER) || m_settings.stackGrowsUp)
		m_normalStack = false;

	// Initialize symbolic assembly function
	X86_SYMINSTR_NAME(Function) symFunc(m_settings, func);
	m_func = func;
	m_symFunc = &symFunc;

	symFunc.InitializeBlocks(func);

	if (m_settings.positionIndependent)
		m_settings.basePointer = symFunc.AddRegister(X86REGCLASS_INTEGER);

	// Generate stack frame
	for (vector< Ref<Variable> >::const_iterator i = func->GetVariables().begin(); i != func->GetVariables().end(); i++)
	{
		if ((*i)->IsParameter())
			continue;

		if (((*i)->GetType()->GetClass() != TYPE_STRUCT) && ((*i)->GetType()->GetClass() != TYPE_ARRAY))
		{
			// If the variable has its address taken, it cannot be stored in a register
			bool addressTaken = false;
			for (vector<ILBlock*>::const_iterator j = m_func->GetIL().begin(); j != m_func->GetIL().end(); j++)
			{
				for (vector<ILInstruction>::const_iterator k = (*j)->GetInstructions().begin();
					k != (*j)->GetInstructions().end(); k++)
				{
					if (k->operation != ILOP_ADDRESS_OF)
						continue;
					if (k->params[1].variable == *i)
					{
						addressTaken = true;
						break;
					}
				}
			}

			if (addressTaken)
			{
				m_stackVar[*i] = m_symFunc->AddStackVar(0, false, (*i)->GetType()->GetWidth(),
					ILParameter::ReduceType((*i)->GetType()));
				continue;
			}

			// Variable can be stored in a register
			uint32_t reg;
			if ((*i)->GetType()->GetClass() == TYPE_FLOAT)
				reg = m_symFunc->AddRegister(X86REGCLASS_FLOAT, ILParameter::ReduceType((*i)->GetType()));
			else if ((*i)->GetType()->GetWidth() == 1)
				reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT, ILParameter::ReduceType((*i)->GetType()));
			else
				reg = m_symFunc->AddRegister(X86REGCLASS_INTEGER, ILParameter::ReduceType((*i)->GetType()));
			m_varReg[*i] = reg;

			// 64-bit variables take two adjacent registers
			if (((*i)->GetType()->GetWidth() == 8) && ((*i)->GetType()->GetClass() != TYPE_FLOAT))
				m_symFunc->AddRegister(X86REGCLASS_INTEGER, ILParameter::ReduceType((*i)->GetType()), 4);
		}
		else
		{
			// Structures and arrays cannot be stored in a register
			m_stackVar[*i] = m_symFunc->AddStackVar(0, false, (*i)->GetType()->GetWidth(), ILTYPE_VOID);
		}
	}

	// Generate parameter offsets
	int64_t offset = 0;

	if ((func->GetName() == "_start") && (m_settings.unsafeStack))
		offset += UNSAFE_STACK_PIVOT;

#ifdef OUTPUT32
	uint32_t intParamRegs[4] = {X86REGCLASS_INTEGER_PARAM_0, X86REGCLASS_INTEGER_PARAM_1, X86REGCLASS_INTEGER_PARAM_2, SYMREG_NONE};
#else
	uint32_t intParamRegs[7] = {X86REGCLASS_INTEGER_PARAM_0, X86REGCLASS_INTEGER_PARAM_1, X86REGCLASS_INTEGER_PARAM_2,
		X86REGCLASS_INTEGER_PARAM_3, X86REGCLASS_INTEGER_PARAM_4, X86REGCLASS_INTEGER_PARAM_5, SYMREG_NONE};
#endif
	uint32_t curIntParamReg = 0;
	vector<IncomingParameterCopy> paramCopy;

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

		// See if a register is used for this parameter
		// TODO: Floating point registers
		uint32_t reg = SYMREG_NONE;
#ifdef OUTPUT32
		size_t paramSize = ((*var)->GetType()->GetWidth() + 3) & (~3);
		if (paramSize <= 4)
		{
			if (intParamRegs[curIntParamReg] != SYMREG_NONE)
			{
				uint32_t regClass = intParamRegs[curIntParamReg++];
				if (var != func->GetVariables().end())
					reg = m_symFunc->AddRegister(regClass, ILParameter::ReduceType((*var)->GetType()));
			}
		}
		else
		{
			if ((intParamRegs[curIntParamReg] != SYMREG_NONE) && (intParamRegs[curIntParamReg + 1] != SYMREG_NONE))
			{
				uint32_t regClass = intParamRegs[curIntParamReg++];
				uint32_t highRegClass = intParamRegs[curIntParamReg++];
				if (var != func->GetVariables().end())
				{
					reg = m_symFunc->AddRegister(regClass, ILParameter::ReduceType((*var)->GetType()));
					m_symFunc->AddRegister(highRegClass, ILParameter::ReduceType((*var)->GetType()), 4);
				}
			}
		}
#else
		if (intParamRegs[curIntParamReg] != SYMREG_NONE)
		{
			uint32_t regClass = intParamRegs[curIntParamReg++];
			if (var != func->GetVariables().end())
				reg = m_symFunc->AddRegister(regClass, ILParameter::ReduceType((*var)->GetType()));
		}
#endif

		if (var == func->GetVariables().end())
		{
			// Variable not named, so it won't be referenced
			continue;
		}

		if (reg != SYMREG_NONE)
		{
			// If the variable has its address taken, it must be stored on the stack
			bool addressTaken = false;
			for (vector<ILBlock*>::const_iterator j = m_func->GetIL().begin(); j != m_func->GetIL().end(); j++)
			{
				for (vector<ILInstruction>::const_iterator k = (*j)->GetInstructions().begin();
					k != (*j)->GetInstructions().end(); k++)
				{
					if (k->operation != ILOP_ADDRESS_OF)
						continue;
					if (k->params[1].variable == *var)
					{
						addressTaken = true;
						break;
					}
				}
			}

			if (addressTaken)
			{
				// Must spill register to stack
				m_stackVar[*var] = m_symFunc->AddStackVar(0, false, (*var)->GetType()->GetWidth(),
					ILParameter::ReduceType((*var)->GetType()));

				IncomingParameterCopy copy;
				copy.var = *var;
				copy.incomingReg = reg;
				copy.stackVar = m_stackVar[*var];
				paramCopy.push_back(copy);
				continue;
			}

			m_varReg[*var] = reg;

			IncomingParameterCopy copy;
			copy.var = *var;
			copy.incomingReg = reg;
			copy.stackVar = SYMREG_NONE;
			paramCopy.push_back(copy);
			continue;
		}

		// Allocate stack space for this parameter
#ifdef OUTPUT32
		int64_t paramOffset = offset + 4;
#else
		int64_t paramOffset = offset + 8;
#endif

		if (m_settings.stackGrowsUp)
		{
			paramOffset = -paramOffset;
#ifdef OUTPUT32
			size_t paramSize = ((*var)->GetType()->GetWidth() + 3) & (~3);
			paramOffset += 4 - paramSize;
#else
			size_t paramSize = ((*var)->GetType()->GetWidth() + 7) & (~7);
			paramOffset += 8 - paramSize;
#endif
		}

		m_stackVar[*var] = m_symFunc->AddStackVar(paramOffset, true, (*var)->GetType()->GetWidth(),
			ILParameter::ReduceType((*var)->GetType()));

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

	// Generate a variable to mark the start of additional paramaters
#ifdef OUTPUT32
	int64_t paramOffset = offset + 4;
#else
	int64_t paramOffset = offset + 8;
#endif
	if (m_settings.stackGrowsUp)
		paramOffset = -paramOffset;
	m_varargStart = m_symFunc->AddStackVar(paramOffset, true, 0, ILTYPE_VOID);

	// Generate code
	bool first = true;
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		SymInstrBlock* out = m_symFunc->GetBlock(*i);

		if (first)
		{
			if ((func->GetName() == "_start") && m_settings.unsafeStack)
			{
				// This is the start function, and we can't assume we have a safe stack (the code may be
				// at or near the stack pointer), pivot the stack to make it safe
#ifdef OUTPUT32
				EMIT_RI(sub_32, SYMREG_SP, UNSAFE_STACK_PIVOT);
#else
				EMIT_RI(sub_64, SYMREG_SP, UNSAFE_STACK_PIVOT);
#endif
			}

			// Generate function prologue
			if ((!m_normalStack) && (func->GetName() == "_start"))
			{
				// If using alternate stack pointer, and this is the _start function, initialize stack pointer
				size_t stackAdjust = m_settings.stackGrowsUp ? -0x10000 : 0;

#ifdef OUTPUT32
				if (m_func->DoesReturn())
					stackAdjust -= 4;
#else
				if (m_func->DoesReturn())
					stackAdjust -= 8;
#endif

				EMIT_II(enter, 0, 0);
#ifdef OUTPUT32
				EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_BP, -4 + stackAdjust));
				out->AddInstruction(X86_SYMINSTR_NAME(SaveCalleeSavedRegs)());
				if (m_func->DoesReturn())
					EMIT_MR(mov_32, X86_SYM_MEM(SYMREG_SP, 0), SYMREG_BP);
				EMIT_RR(mov_32, SYMREG_BP, SYMREG_SP);
#else
				EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_BP, -8 + stackAdjust));
				out->AddInstruction(X86_SYMINSTR_NAME(SaveCalleeSavedRegs)());
				if (m_func->DoesReturn())
					EMIT_MR(mov_64, X86_SYM_MEM(SYMREG_SP, 0), SYMREG_BP);
				EMIT_RR(mov_64, SYMREG_BP, SYMREG_SP);
#endif
			}
			else if (m_framePointerEnabled)
			{
				if (m_func->DoesReturn())
				{
					out->AddInstruction(X86_SYMINSTR_NAME(SaveCalleeSavedRegs)());
					if (m_normalStack)
						EMIT_R(push, SYMREG_BP);
					else
					{
#ifdef OUTPUT32
						EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? 4 : -4));
						EMIT_MR(mov_32, X86_SYM_MEM(SYMREG_SP, 0), SYMREG_BP);
#else
						EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM(SYMREG_SP, m_settings.stackGrowsUp ? 8 : -8));
						EMIT_MR(mov_64, X86_SYM_MEM(SYMREG_SP, 0), SYMREG_BP);
#endif
					}
				}

#ifdef OUTPUT32
				EMIT_RR(mov_32, SYMREG_BP, SYMREG_SP);
#else
				EMIT_RR(mov_64, SYMREG_BP, SYMREG_SP);
#endif
			}
			else if (m_func->DoesReturn())
			{
				out->AddInstruction(X86_SYMINSTR_NAME(SaveCalleeSavedRegs)());
			}

			if (m_settings.stackGrowsUp)
				EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM_INDEX(SYMREG_SP, SYMREG_NONE, 1, SYMVAR_FRAME_SIZE, 0));
			else
				EMIT_RM(lea, SYMREG_SP, X86_SYM_MEM_INDEX(SYMREG_SP, SYMREG_NONE, 1, SYMVAR_NEG_FRAME_SIZE, 0));

			// Generate a pseudo instruction to ensure the incoming parameters are defined
			vector<uint32_t> incomingRegs;
			for (vector<IncomingParameterCopy>::iterator j = paramCopy.begin(); j != paramCopy.end(); j++)
			{
				if (j->stackVar != SYMREG_NONE)
					continue;

				incomingRegs.push_back(j->incomingReg);
#ifdef OUTPUT32
				if (j->var->GetType()->GetWidth() == 8)
					incomingRegs.push_back(j->incomingReg + 1);
#endif
			}

			if (incomingRegs.size() != 0)
				out->AddInstruction(X86_SYMINSTR_NAME(RegParam)(incomingRegs));

			// Copy parameters into variables so that they can be spilled if needed
			for (vector<IncomingParameterCopy>::iterator j = paramCopy.begin(); j != paramCopy.end(); j++)
			{
				if (j->stackVar != SYMREG_NONE)
				{
					// Parameter was spilled onto stack
					switch (j->var->GetType()->GetWidth())
					{
					case 1:
						EMIT_MR(mov_8, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, j->stackVar, 0), j->incomingReg);
						break;
					case 2:
						EMIT_MR(mov_16, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, j->stackVar, 0), j->incomingReg);
						break;
					case 4:
						EMIT_MR(mov_32, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, j->stackVar, 0), j->incomingReg);
						break;
#ifdef OUTPUT32
					case 8:
						EMIT_MR(mov_32, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, j->stackVar, 0), j->incomingReg);
						EMIT_MR(mov_32, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, j->stackVar, 4), j->incomingReg + 1);
						break;
#else
					case 8:
						EMIT_MR(mov_64, X86_SYM_MEM_INDEX(SYMREG_BP, SYMREG_NONE, 1, j->stackVar, 0), j->incomingReg);
						break;
#endif
					default:
						fprintf(stderr, "error: spilling invalid parameter\n");
						return false;
					}
				}
				else
				{
					// Parameter is in a register
					uint32_t newReg;
					if (j->var->GetType()->GetWidth() == 1)
						newReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER_8BIT);
					else
						newReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#ifdef OUTPUT32
					uint32_t newHighReg = SYMREG_NONE;
					if (j->var->GetType()->GetWidth() == 8)
						newHighReg = m_symFunc->AddRegister(X86REGCLASS_INTEGER);
#endif

					switch (j->var->GetType()->GetWidth())
					{
					case 1:
						EMIT_RR(mov_8, newReg, j->incomingReg);
						break;
					case 2:
						EMIT_RR(mov_16, newReg, j->incomingReg);
						break;
					case 4:
						EMIT_RR(mov_32, newReg, j->incomingReg);
						break;
#ifdef OUTPUT32
					case 8:
						EMIT_RR(mov_32, newReg, j->incomingReg);
						EMIT_RR(mov_32, newHighReg, j->incomingReg + 1);
						break;
#else
					case 8:
						EMIT_RR(mov_64, newReg, j->incomingReg);
						break;
#endif
					default:
						fprintf(stderr, "error: invalid parameter\n");
						return false;
					}

					m_varReg[j->var] = newReg;
				}
			}

#ifdef OUTPUT32
			if (m_settings.positionIndependent)
			{
				// Capture base of code at start
				GetCodeAddressFromInstructionPointer(out, m_settings.basePointer, m_startFunc, m_startFunc->GetIL()[0]);
			}
#endif

			first = false;
		}

		if (!GenerateCodeBlock(out, *i))
			return false;
	}

	if (m_settings.internalDebug)
		fprintf(stderr, "\n%s:\n", func->GetName().c_str());

	// Allocate registers for symbolic code to produce final assembly
	if (!m_symFunc->AllocateRegisters())
	{
		if (m_settings.internalDebug)
		{
			fprintf(stderr, "\n%s:\n", func->GetName().c_str());
			m_symFunc->Print();
		}
		return false;
	}

	// Emit machine code for each block
	for (vector<ILBlock*>::const_iterator i = func->GetIL().begin(); i != func->GetIL().end(); i++)
	{
		OutputBlock* out = new OutputBlock;
		out->code = NULL;
		out->len = 0;
		out->maxLen = 0;
		out->randomLen = 0;
		out->bigEndian = false;

		if (!m_symFunc->GetBlock(*i)->EmitCode(m_symFunc, out))
		{
			delete out;
			return false;
		}

		(*i)->SetOutputBlock(out);
	}

	if (m_settings.internalDebug)
	{
		fprintf(stderr, "\n%s:\n", func->GetName().c_str());
		m_symFunc->Print();
	}
	return true;
}


TreeNode* OUTPUT_CLASS_NAME::GenerateCall(TreeBlock* block, TreeNode* func, size_t fixedParams, const vector< Ref<TreeNode> >& params,
	TreeNodeType resultType)
{
	return NULL;
}


TreeNode* OUTPUT_CLASS_NAME::GenerateSyscall(TreeBlock* block, TreeNode* num, const vector< Ref<TreeNode> >& params,
	TreeNodeType resultType)
{
	return NULL;
}

#endif

