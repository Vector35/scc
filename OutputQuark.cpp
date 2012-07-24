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

#include <stdio.h>
#include <stdlib.h>
#include "OutputQuark.h"
#include "Function.h"
#include "Struct.h"

#define UNSAFE_STACK_PIVOT 0x1000

using namespace std;


OutputQuark::OutputQuark(const Settings& settings, Function* startFunc): Output(settings, startFunc)
{
}


bool OutputQuark::IsSigned11Bit(int64_t imm)
{
	if (imm < -1024)
		return false;
	if (imm > 1023)
		return false;
	return true;
}


bool OutputQuark::IsSigned17Bit(int64_t imm)
{
	if (imm < -65536)
		return false;
	if (imm > 65535)
		return false;
	return true;
}


bool OutputQuark::IsPowerOfTwo(int32_t imm, uint32_t& shiftCount)
{
	for (shiftCount = 0; shiftCount < 32; shiftCount++)
	{
		if (imm == (1 << shiftCount))
			return true;
	}
	return false;
}


void OutputQuark::LoadImm(SymInstrBlock* out, uint32_t dest, int32_t imm)
{
	if (IsSigned17Bit(imm))
		out->AddInstruction(QuarkLoadImm(dest, imm));
	else
	{
		out->AddInstruction(QuarkLoadImm(dest, imm & 0xffff));
		out->AddInstruction(QuarkLoadImmHigh(dest, imm >> 16));
	}
}


void OutputQuark::AddImm(SymInstrBlock* out, uint32_t dest, uint32_t src, int32_t imm)
{
	if ((imm == 0) && (dest == src))
		return;

	if (IsSigned11Bit(imm))
		out->AddInstruction(QuarkAdd(dest, src, imm));
	else
	{
		uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		LoadImm(out, temp, imm);
		out->AddInstruction(QuarkAdd(dest, src, temp, 0));
	}
}


void OutputQuark::SubImm(SymInstrBlock* out, uint32_t dest, uint32_t src, int32_t imm)
{
	if ((imm == 0) && (dest == src))
		return;

	if (IsSigned11Bit(imm))
		out->AddInstruction(QuarkSub(dest, src, imm));
	else
	{
		uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		LoadImm(out, temp, imm);
		out->AddInstruction(QuarkSub(dest, src, temp, 0));
	}
}


bool OutputQuark::AccessVariableStorage(SymInstrBlock* out, const ILParameter& param, OperandReference& ref)
{
	if (param.cls == ILPARAM_MEMBER)
	{
		if (!AccessVariableStorage(out, *param.parent, ref))
			return false;
		if (ref.type != OPERANDREF_MEM)
			return false;
		if (!param.structure)
			return false;

		const StructMember* member = param.structure->GetMember(param.stringValue);
		if (!member)
			return false;

		ref.offset += member->offset;
		return true;
	}

	if (param.cls != ILPARAM_VAR)
		return false;

	if (param.variable->IsGlobal())
	{
		ref.type = OPERANDREF_MEM;
		ref.sign = param.variable->GetType()->IsSigned();
		ref.memType = MEMORYREF_GLOBAL_VAR;
		ref.base = SYMREG_IP;
		ref.offset = param.variable->GetDataSectionOffset();
		ref.scratch = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		return true;
	}

	if (m_varReg.find(param.variable) != m_varReg.end())
	{
		// Variable has a register associated with it
		ref.type = OPERANDREF_REG;
		ref.sign = param.variable->GetType()->IsSigned();
		ref.reg = m_varReg[param.variable];
		if ((!param.IsFloat()) && (ref.width == 8))
			ref.highReg = ref.reg + 1;
		return true;
	}

	map<Variable*, int32_t>::iterator i = m_stackVar.find(param.variable);
	if (i == m_stackVar.end())
		return false;
	ref.type = OPERANDREF_MEM;
	ref.sign = param.variable->GetType()->IsSigned();
	ref.memType = MEMORYREF_STACK_VAR;
	ref.base = m_framePointerEnabled ? SYMREG_BP : SYMREG_SP;
	ref.var = i->second;
	ref.offset = 0;
	ref.scratch = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	return true;
}


bool OutputQuark::Load(SymInstrBlock* out, const ILParameter& param, OperandReference& ref, bool forceReg)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!AccessVariableStorage(out, param, ref))
			return false;

		if (ref.type == OPERANDREF_MEM)
		{
			// Load memory references into a register
			uint32_t reg, highReg = SYMREG_NONE;
			reg = m_symFunc->AddRegister(param.IsFloat() ? QUARKREGCLASS_FLOAT : QUARKREGCLASS_INTEGER);
			if ((!param.IsFloat()) && (ref.width == 8))
				highReg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);

			if (param.IsFloat())
			{
				switch (ref.width)
				{
				case 4:
					if (ref.memType == MEMORYREF_STACK_VAR)
						out->AddInstruction(QuarkLoadStackFS(reg, ref.base, ref.var, ref.offset, ref.scratch));
					else
						out->AddInstruction(QuarkLoadGlobalFS(reg, ref.base, ref.offset, ref.scratch));
					break;
				case 8:
					if (ref.memType == MEMORYREF_STACK_VAR)
						out->AddInstruction(QuarkLoadStackFD(reg, ref.base, ref.var, ref.offset, ref.scratch));
					else
						out->AddInstruction(QuarkLoadGlobalFD(reg, ref.base, ref.offset, ref.scratch));
					break;
				default:
					return false;
				}
			}
			else
			{
				switch (ref.width)
				{
				case 1:
					if (ref.sign)
					{
						if (ref.memType == MEMORYREF_STACK_VAR)
							out->AddInstruction(QuarkLoadStackSX8(reg, ref.base, ref.var, ref.offset, ref.scratch));
						else
							out->AddInstruction(QuarkLoadGlobalSX8(reg, ref.base, ref.offset, ref.scratch));
					}
					else
					{
						if (ref.memType == MEMORYREF_STACK_VAR)
							out->AddInstruction(QuarkLoadStack8(reg, ref.base, ref.var, ref.offset, ref.scratch));
						else
							out->AddInstruction(QuarkLoadGlobal8(reg, ref.base, ref.offset, ref.scratch));
					}
					break;
				case 2:
					if (ref.sign)
					{
						if (ref.memType == MEMORYREF_STACK_VAR)
							out->AddInstruction(QuarkLoadStackSX16(reg, ref.base, ref.var, ref.offset, ref.scratch));
						else
							out->AddInstruction(QuarkLoadGlobalSX16(reg, ref.base, ref.offset, ref.scratch));
					}
					else
					{
						if (ref.memType == MEMORYREF_STACK_VAR)
							out->AddInstruction(QuarkLoadStack16(reg, ref.base, ref.var, ref.offset, ref.scratch));
						else
							out->AddInstruction(QuarkLoadGlobal16(reg, ref.base, ref.offset, ref.scratch));
					}
					break;
				case 4:
					if (ref.memType == MEMORYREF_STACK_VAR)
						out->AddInstruction(QuarkLoadStack32(reg, ref.base, ref.var, ref.offset, ref.scratch));
					else
						out->AddInstruction(QuarkLoadGlobal32(reg, ref.base, ref.offset, ref.scratch));
					break;
				case 8:
					if (ref.memType == MEMORYREF_STACK_VAR)
					{
						out->AddInstruction(QuarkLoadStack32(reg, ref.base, ref.var, ref.offset, ref.scratch));
						out->AddInstruction(QuarkLoadStack32(highReg, ref.base, ref.var, ref.offset + 4, ref.scratch));
					}
					else
					{
						out->AddInstruction(QuarkLoadGlobal32(reg, ref.base, ref.offset, ref.scratch));
						out->AddInstruction(QuarkLoadGlobal32(highReg, ref.base, ref.offset + 4, ref.scratch));
					}
					break;
				default:
					return false;
				}
			}

			ref.type = OPERANDREF_REG;
			ref.reg = reg;
			ref.highReg = highReg;
		}
		return true;
	case ILPARAM_INT:
		if ((!forceReg) && IsSigned11Bit(param.integerValue))
		{
			ref.type = OPERANDREF_IMMED;
			ref.sign = true;
			ref.immed = param.integerValue;
			return true;
		}

		ref.type = OPERANDREF_REG;
		ref.sign = true;
		ref.reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		if (ref.width == 8)
			ref.highReg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);

		LoadImm(out, ref.reg, (int32_t)param.integerValue);
		if (ref.width == 8)
			LoadImm(out, ref.highReg, (int32_t)(param.integerValue >> 32));
		return true;
	case ILPARAM_BOOL:
		if (!forceReg)
		{
			ref.type = OPERANDREF_IMMED;
			ref.immed = param.boolValue ? 1 : 0;
			return true;
		}
		ref.type = OPERANDREF_REG;
		ref.sign = false;
		ref.reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		LoadImm(out, ref.reg, param.boolValue ? 1 : 0);
		return true;
	case ILPARAM_FUNC:
		ref.type = OPERANDREF_REG;
		ref.sign = false;
		ref.reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		out->AddInstruction(QuarkAddBlock(ref.reg, SYMREG_IP, param.function, param.function->GetIL()[0],
			m_symFunc->AddRegister(QUARKREGCLASS_INTEGER)));
		return true;
	case ILPARAM_UNDEFINED:
		ref.type = OPERANDREF_REG;
		ref.sign = true;
		ref.width = param.GetWidth();
		ref.reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		return true;
	default:
		return false;
	}
}


bool OutputQuark::PrepareStore(SymInstrBlock* out, const ILParameter& param, OperandReference& ref)
{
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		return AccessVariableStorage(out, param, ref);
	default:
		return false;
	}
}


bool OutputQuark::LoadIntoRegister(SymInstrBlock* out, const OperandReference& ref, OperandReference& reg)
{
	if (ref.type == OPERANDREF_REG)
	{
		reg = ref;
		return true;
	}

	if (ref.type == OPERANDREF_IMMED)
	{
		reg.type = OPERANDREF_REG;
		reg.width = ref.width;
		reg.reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		LoadImm(out, reg.reg, (int32_t)ref.immed);
		if (ref.width == 8)
		{
			reg.highReg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			LoadImm(out, reg.highReg, (int32_t)(ref.immed >> 32));
		}
		return true;
	}

	return false;
}


bool OutputQuark::GetDestRegister(SymInstrBlock* out, const OperandReference& dest, OperandReference& reg)
{
	if (dest.type == OPERANDREF_REG)
		reg = dest;
	else
	{
		reg.type = OPERANDREF_REG;
		reg.width = dest.width;
		reg.reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		if (reg.width == 8)
			reg.highReg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	}
	return true;
}


bool OutputQuark::Move(SymInstrBlock* out, const OperandReference& dest, const OperandReference& src, bool enforceSize)
{
	if (dest.type == OPERANDREF_REG)
	{
		if (src.type == OPERANDREF_IMMED)
		{
			LoadImm(out, dest.reg, (int32_t)src.immed);
			if (dest.width == 8)
				LoadImm(out, dest.highReg, (int32_t)(src.immed >> 32));
		}
		else if (enforceSize && (dest.width == 1))
		{
			if (dest.sign)
				out->AddInstruction(QuarkSxb(dest.reg, src.reg));
			else
				out->AddInstruction(QuarkZxb(dest.reg, src.reg));
		}
		else if (enforceSize && (dest.width == 2))
		{
			if (dest.sign)
				out->AddInstruction(QuarkSxh(dest.reg, src.reg));
			else
				out->AddInstruction(QuarkZxh(dest.reg, src.reg));
		}
		else
		{
			if (dest.reg != src.reg)
				out->AddInstruction(QuarkMov(dest.reg, src.reg, 0));
			if ((dest.width == 8) && (dest.highReg != src.highReg))
				out->AddInstruction(QuarkMov(dest.highReg, src.highReg, 0));
		}
		return true;
	}

	if (dest.type == OPERANDREF_MEM)
	{
		OperandReference srcReg;
		if (!LoadIntoRegister(out, src, srcReg))
			return false;

		switch (dest.width)
		{
		case 1:
			if (dest.memType == MEMORYREF_STACK_VAR)
				out->AddInstruction(QuarkStoreStack8(srcReg.reg, dest.base, dest.var, dest.offset, dest.scratch));
			else
				out->AddInstruction(QuarkStoreGlobal8(srcReg.reg, dest.base, dest.offset, dest.scratch));
			break;
		case 2:
			if (dest.memType == MEMORYREF_STACK_VAR)
				out->AddInstruction(QuarkStoreStack16(srcReg.reg, dest.base, dest.var, dest.offset, dest.scratch));
			else
				out->AddInstruction(QuarkStoreGlobal16(srcReg.reg, dest.base, dest.offset, dest.scratch));
			break;
		case 4:
			if (dest.memType == MEMORYREF_STACK_VAR)
				out->AddInstruction(QuarkStoreStack32(srcReg.reg, dest.base, dest.var, dest.offset, dest.scratch));
			else
				out->AddInstruction(QuarkStoreGlobal32(srcReg.reg, dest.base, dest.offset, dest.scratch));
			break;
		case 8:
			if (dest.memType == MEMORYREF_STACK_VAR)
			{
				out->AddInstruction(QuarkStoreStack32(srcReg.reg, dest.base, dest.var, dest.offset, dest.scratch));
				out->AddInstruction(QuarkStoreStack32(srcReg.highReg, dest.base, dest.var, dest.offset + 4, dest.scratch));
			}
			else
			{
				out->AddInstruction(QuarkStoreGlobal32(srcReg.reg, dest.base, dest.offset, dest.scratch));
				out->AddInstruction(QuarkStoreGlobal32(srcReg.highReg, dest.base, dest.offset + 4, dest.scratch));
			}
			break;
		default:
			return false;
		}

		return true;
	}

	return false;
}


void OutputQuark::ConditionalJump(SymInstrBlock* out, ILBlock* block, int cond, bool value)
{
	out->AddInstruction(QuarkCondJump(cond, value ? 1 : 0, m_func, block));
}


void OutputQuark::UnconditionalJump(SymInstrBlock* out, ILBlock* block, bool canOmit)
{
	if (canOmit && (block->GetGlobalIndex() == (m_currentBlock->GetGlobalIndex() + 1)))
	{
		// The destination block is the one just after the current one, just fall through
		return;
	}

	out->AddInstruction(QuarkJump(m_func, block));
}


bool OutputQuark::GenerateAssign(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, src;
	if (!Load(out, instr.params[1], src))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	return Move(out, dest, src);
}


bool OutputQuark::GenerateAddressOf(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference ref, dest, addr;
	if (!AccessVariableStorage(out, instr.params[1], ref))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, addr))
		return false;
	if (ref.type != OPERANDREF_MEM)
		return false;

	if (ref.memType == MEMORYREF_STACK_VAR)
		out->AddInstruction(QuarkAddStack(addr.reg, ref.base, ref.var, ref.offset, ref.scratch));
	else
		out->AddInstruction(QuarkAddGlobal(addr.reg, ref.base, ref.offset, ref.scratch));
	return Move(out, dest, addr);
}


bool OutputQuark::GenerateAddressOfMember(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference ref, dest, addr;
	if (!Load(out, instr.params[1], ref))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, addr))
		return false;

	if (!instr.params[2].structure)
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	if (ref.type == OPERANDREF_REG)
		out->AddInstruction(QuarkAdd(addr.reg, ref.reg, member->offset));
	else
		LoadImm(out, addr.reg, (uint32_t)ref.immed + member->offset);
	return Move(out, dest, addr);
}


bool OutputQuark::GenerateDeref(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference addr, dest, value;
	if (!Load(out, instr.params[1], addr, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, value))
		return false;

	switch (value.width)
	{
	case 1:
		out->AddInstruction(QuarkLoad8(value.reg, addr.reg, 0));
		break;
	case 2:
		out->AddInstruction(QuarkLoad16(value.reg, addr.reg, 0));
		break;
	case 4:
		out->AddInstruction(QuarkLoad32(value.reg, addr.reg, 0));
		break;
	case 8:
		out->AddInstruction(QuarkLoad32(value.reg, addr.reg, 0));
		out->AddInstruction(QuarkLoad32(value.highReg, addr.reg, 4));
		break;
	default:
		return false;
	}

	return Move(out, dest, value);
}


bool OutputQuark::GenerateDerefMember(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference addr, dest, value;
	if (!Load(out, instr.params[1], addr, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, value))
		return false;
	if (!instr.params[2].structure)
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	int32_t offset = member->offset;
	if ((!IsSigned11Bit(offset)) || (!IsSigned11Bit(offset + value.width - 1)))
	{
		int reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		LoadImm(out, reg, offset);
		out->AddInstruction(QuarkAdd(reg, addr.reg, reg, 0));
		addr.reg = reg;
		offset = 0;
	}

	switch (value.width)
	{
	case 1:
		out->AddInstruction(QuarkLoad8(value.reg, addr.reg, offset));
		break;
	case 2:
		out->AddInstruction(QuarkLoad16(value.reg, addr.reg, offset));
		break;
	case 4:
		out->AddInstruction(QuarkLoad32(value.reg, addr.reg, offset));
		break;
	case 8:
		out->AddInstruction(QuarkLoad32(value.reg, addr.reg, offset));
		out->AddInstruction(QuarkLoad32(value.highReg, addr.reg, offset + 4));
		break;
	default:
		return false;
	}

	return Move(out, dest, value);
}


bool OutputQuark::GenerateDerefAssign(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference addr, value;
	if (!Load(out, instr.params[0], addr, true))
		return false;
	if (!Load(out, instr.params[1], value, true))
		return false;

	switch (value.width)
	{
	case 1:
		out->AddInstruction(QuarkStore8(value.reg, addr.reg, 0));
		break;
	case 2:
		out->AddInstruction(QuarkStore16(value.reg, addr.reg, 0));
		break;
	case 4:
		out->AddInstruction(QuarkStore32(value.reg, addr.reg, 0));
		break;
	case 8:
		out->AddInstruction(QuarkStore32(value.reg, addr.reg, 0));
		out->AddInstruction(QuarkStore32(value.highReg, addr.reg, 4));
		break;
	default:
		return false;
	}

	return true;
}


bool OutputQuark::GenerateDerefMemberAssign(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference addr, value;
	if (!Load(out, instr.params[0], addr, true))
		return false;
	if (!Load(out, instr.params[2], value, true))
		return false;

	const StructMember* member = instr.params[1].structure->GetMember(instr.params[1].stringValue);
	if (!member)
		return false;

	int32_t offset = member->offset;
	if ((!IsSigned11Bit(offset)) || (!IsSigned11Bit(offset + value.width - 1)))
	{
		int reg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		LoadImm(out, reg, offset);
		out->AddInstruction(QuarkAdd(reg, addr.reg, reg, 0));
		addr.reg = reg;
		offset = 0;
	}

	switch (value.width)
	{
	case 1:
		out->AddInstruction(QuarkStore8(value.reg, addr.reg, offset));
		break;
	case 2:
		out->AddInstruction(QuarkStore16(value.reg, addr.reg, offset));
		break;
	case 4:
		out->AddInstruction(QuarkStore32(value.reg, addr.reg, offset));
		break;
	case 8:
		out->AddInstruction(QuarkStore32(value.reg, addr.reg, offset));
		out->AddInstruction(QuarkStore32(value.highReg, addr.reg, offset + 4));
		break;
	default:
		return false;
	}

	return true;
}


bool OutputQuark::GenerateArrayIndex(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference ref, i, dest, value;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!AccessVariableStorage(out, instr.params[1], ref))
		return false;
	if (!Load(out, instr.params[2], i))
		return false;
	if (!GetDestRegister(out, dest, value))
		return false;
	if (ref.type != OPERANDREF_MEM)
		return false;

	uint32_t shiftCount;
	uint32_t addr = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	ref.width = (uint32_t)instr.params[3].integerValue;
	switch (i.type)
	{
	case OPERANDREF_REG:
		if (!IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			return false;
		out->AddInstruction(QuarkAdd(addr, ref.base, i.reg, shiftCount));
		ref.base = addr;
		break;
	case OPERANDREF_IMMED:
		ref.offset += i.immed * instr.params[3].integerValue;
		break;
	default:
		return false;
	}

	switch (value.width)
	{
	case 1:
		if (ref.memType == MEMORYREF_STACK_VAR)
			out->AddInstruction(QuarkLoadStack8(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
		else
			out->AddInstruction(QuarkLoadGlobal8(value.reg, ref.base, ref.offset, ref.scratch));
		break;
	case 2:
		if (ref.memType == MEMORYREF_STACK_VAR)
			out->AddInstruction(QuarkLoadStack16(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
		else
			out->AddInstruction(QuarkLoadGlobal16(value.reg, ref.base, ref.offset, ref.scratch));
		break;
	case 4:
		if (ref.memType == MEMORYREF_STACK_VAR)
			out->AddInstruction(QuarkLoadStack32(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
		else
			out->AddInstruction(QuarkLoadGlobal32(value.reg, ref.base, ref.offset, ref.scratch));
		break;
	case 8:
		if (ref.memType == MEMORYREF_STACK_VAR)
		{
			out->AddInstruction(QuarkLoadStack32(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
			out->AddInstruction(QuarkLoadStack32(value.highReg, ref.base, ref.var, ref.offset + 4, ref.scratch));
		}
		else
		{
			out->AddInstruction(QuarkLoadGlobal32(value.reg, ref.base, ref.offset, ref.scratch));
			out->AddInstruction(QuarkLoadGlobal32(value.highReg, ref.base, ref.offset + 4, ref.scratch));
		}
		break;
	default:
		return false;
	}

	return Move(out, dest, value);
}


bool OutputQuark::GenerateArrayIndexAssign(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference ref, i, value;
	if (!AccessVariableStorage(out, instr.params[0], ref))
		return false;
	if (!Load(out, instr.params[1], i))
		return false;
	if (!Load(out, instr.params[3], value, true))
		return false;
	if (ref.type != OPERANDREF_MEM)
		return false;

	uint32_t shiftCount;
	uint32_t addr = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	ref.width = (uint32_t)instr.params[2].integerValue;
	switch (i.type)
	{
	case OPERANDREF_REG:
		if (!IsPowerOfTwo(instr.params[2].integerValue, shiftCount))
			return false;
		out->AddInstruction(QuarkAdd(addr, ref.base, i.reg, shiftCount));
		ref.base = addr;
		break;
	case OPERANDREF_IMMED:
		ref.offset += i.immed * instr.params[2].integerValue;
		break;
	default:
		return false;
	}

	switch (value.width)
	{
	case 1:
		if (ref.memType == MEMORYREF_STACK_VAR)
			out->AddInstruction(QuarkStoreStack8(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
		else
			out->AddInstruction(QuarkStoreGlobal8(value.reg, ref.base, ref.offset, ref.scratch));
		break;
	case 2:
		if (ref.memType == MEMORYREF_STACK_VAR)
			out->AddInstruction(QuarkStoreStack16(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
		else
			out->AddInstruction(QuarkStoreGlobal16(value.reg, ref.base, ref.offset, ref.scratch));
		break;
	case 4:
		if (ref.memType == MEMORYREF_STACK_VAR)
			out->AddInstruction(QuarkStoreStack32(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
		else
			out->AddInstruction(QuarkStoreGlobal32(value.reg, ref.base, ref.offset, ref.scratch));
		break;
	case 8:
		if (ref.memType == MEMORYREF_STACK_VAR)
		{
			out->AddInstruction(QuarkStoreStack32(value.reg, ref.base, ref.var, ref.offset, ref.scratch));
			out->AddInstruction(QuarkStoreStack32(value.highReg, ref.base, ref.var, ref.offset + 4, ref.scratch));
		}
		else
		{
			out->AddInstruction(QuarkStoreGlobal32(value.reg, ref.base, ref.offset, ref.scratch));
			out->AddInstruction(QuarkStoreGlobal32(value.highReg, ref.base, ref.offset + 4, ref.scratch));
		}
		break;
	default:
		return false;
	}

	return true;
}


bool OutputQuark::GeneratePtrAdd(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	uint32_t shiftCount;
	switch (right.type)
	{
	case OPERANDREF_REG:
		if (IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			out->AddInstruction(QuarkAdd(result.reg, left.reg, right.reg, shiftCount));
		else if (IsSigned11Bit(instr.params[3].integerValue))
		{
			uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			out->AddInstruction(QuarkMul(temp, right.reg, (int32_t)instr.params[3].integerValue));
			out->AddInstruction(QuarkAdd(result.reg, left.reg, temp, 0));
		}
		else
		{
			uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			LoadImm(out, temp, (int32_t)instr.params[3].integerValue);
			out->AddInstruction(QuarkMul(temp, right.reg, temp, 0));
			out->AddInstruction(QuarkAdd(result.reg, left.reg, temp, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (IsSigned11Bit(instr.params[3].integerValue * right.immed))
			out->AddInstruction(QuarkAdd(result.reg, left.reg, (int32_t)(instr.params[3].integerValue * right.immed)));
		else
		{
			uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			LoadImm(out, temp, (int32_t)(instr.params[3].integerValue * right.immed));
			out->AddInstruction(QuarkAdd(result.reg, left.reg, temp, 0));
		}
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GeneratePtrSub(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	uint32_t shiftCount;
	switch (right.type)
	{
	case OPERANDREF_REG:
		if (IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			out->AddInstruction(QuarkSub(result.reg, left.reg, right.reg, shiftCount));
		else if (IsSigned11Bit(instr.params[3].integerValue))
		{
			uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			out->AddInstruction(QuarkMul(temp, right.reg, (int32_t)instr.params[3].integerValue));
			out->AddInstruction(QuarkSub(result.reg, left.reg, temp, 0));
		}
		else
		{
			uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			LoadImm(out, temp, (int32_t)instr.params[3].integerValue);
			out->AddInstruction(QuarkMul(temp, right.reg, temp, 0));
			out->AddInstruction(QuarkSub(result.reg, left.reg, temp, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (IsSigned11Bit(instr.params[3].integerValue * right.immed))
			out->AddInstruction(QuarkSub(result.reg, left.reg, (int32_t)(instr.params[3].integerValue * right.immed)));
		else
		{
			uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			LoadImm(out, temp, (int32_t)(instr.params[3].integerValue * right.immed));
			out->AddInstruction(QuarkSub(result.reg, left.reg, temp, 0));
		}
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GeneratePtrDiff(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (instr.params[3].integerValue == 1)
		return GenerateSub(out, instr);

	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	uint32_t shiftCount;
	uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkSub(temp, left.reg, right.reg, 0));
		if (IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			out->AddInstruction(QuarkShr(result.reg, temp, shiftCount));
		else
			out->AddInstruction(QuarkDiv(result.reg, temp, instr.params[3].integerValue));
		break;
	case OPERANDREF_IMMED:
		LoadImm(out, temp, (uint32_t)right.immed);
		out->AddInstruction(QuarkSub(temp, left.reg, temp, 0));
		if (IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			out->AddInstruction(QuarkShr(result.reg, temp, shiftCount));
		else
			out->AddInstruction(QuarkDiv(result.reg, temp, instr.params[3].integerValue));
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateAdd(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (result.width == 8)
		{
			out->AddInstruction(QuarkClrCC(3));
			out->AddInstruction(QuarkAddx(result.reg, left.reg, right.reg, 0));
			out->AddInstruction(QuarkAddx(result.highReg, left.highReg, right.highReg, 0));
		}
		else
		{
			out->AddInstruction(QuarkAdd(result.reg, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (result.width == 8)
		{
			out->AddInstruction(QuarkClrCC(3));
			out->AddInstruction(QuarkAddx(result.reg, left.reg, (int32_t)right.immed));
			out->AddInstruction(QuarkAddx(result.highReg, left.highReg, (int32_t)(right.immed >> 32)));
		}
		else
		{
			out->AddInstruction(QuarkAdd(result.reg, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateSub(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (result.width == 8)
		{
			out->AddInstruction(QuarkClrCC(3));
			out->AddInstruction(QuarkSubx(result.reg, left.reg, right.reg, 0));
			out->AddInstruction(QuarkSubx(result.highReg, left.highReg, right.highReg, 0));
		}
		else
		{
			out->AddInstruction(QuarkSub(result.reg, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (result.width == 8)
		{
			out->AddInstruction(QuarkClrCC(3));
			out->AddInstruction(QuarkSubx(result.reg, left.reg, (int32_t)right.immed));
			out->AddInstruction(QuarkSubx(result.highReg, left.highReg, (int32_t)(right.immed >> 32)));
		}
		else
		{
			out->AddInstruction(QuarkSub(result.reg, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::Mult64(SymInstrBlock* out, const OperandReference& result, const OperandReference& left,
	const OperandReference& right)
{
	uint32_t temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	uint32_t outLow = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	uint32_t outHigh = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
	out->AddInstruction(QuarkMulx(outHigh, outLow, left.reg, right.reg));
	out->AddInstruction(QuarkMul(temp, left.reg, right.highReg, 0));
	out->AddInstruction(QuarkAdd(outHigh, outHigh, temp, 0));
	out->AddInstruction(QuarkMul(temp, left.highReg, right.reg, 0));
	out->AddInstruction(QuarkAdd(outHigh, outHigh, temp, 0));
	out->AddInstruction(QuarkMov(result.reg, outLow, 0));
	out->AddInstruction(QuarkMov(result.highReg, outHigh, 0));
	return true;
}


bool OutputQuark::GenerateSignedMult(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (!Load(out, instr.params[2], right, result.width == 8))
		return false;

	if (result.width == 8)
	{
		if (!Mult64(out, result, left, right))
			return false;
	}
	else
	{
		switch (right.type)
		{
		case OPERANDREF_REG:
			out->AddInstruction(QuarkMul(result.reg, left.reg, right.reg, 0));
			break;
		case OPERANDREF_IMMED:
			out->AddInstruction(QuarkMul(result.reg, left.reg, (int32_t)right.immed));
			break;
		default:
			return false;
		}
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateUnsignedMult(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (!Load(out, instr.params[2], right, result.width == 8))
		return false;

	if (result.width == 8)
	{
		if (!Mult64(out, result, left, right))
			return false;
	}
	else
	{
		switch (right.type)
		{
		case OPERANDREF_REG:
			out->AddInstruction(QuarkMul(result.reg, left.reg, right.reg, 0));
			break;
		case OPERANDREF_IMMED:
			out->AddInstruction(QuarkMul(result.reg, left.reg, (int32_t)right.immed));
			break;
		default:
			return false;
		}
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateSignedDiv(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkIdiv(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkIdiv(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateUnsignedDiv(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkDiv(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkDiv(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateSignedMod(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkImod(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkImod(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateUnsignedMod(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkMod(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkMod(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateAnd(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkAnd(result.reg, left.reg, right.reg, 0));
		if (result.width == 8)
			out->AddInstruction(QuarkAnd(result.highReg, left.highReg, right.highReg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkAnd(result.reg, left.reg, (int32_t)right.immed));
		if (result.width == 8)
			out->AddInstruction(QuarkAnd(result.highReg, left.highReg, (int32_t)(right.immed >> 32)));
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateOr(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkOr(result.reg, left.reg, right.reg, 0));
		if (result.width == 8)
			out->AddInstruction(QuarkOr(result.highReg, left.highReg, right.highReg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkOr(result.reg, left.reg, (int32_t)right.immed));
		if (result.width == 8)
			out->AddInstruction(QuarkOr(result.highReg, left.highReg, (int32_t)(right.immed >> 32)));
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateXor(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkXor(result.reg, left.reg, right.reg, 0));
		if (result.width == 8)
			out->AddInstruction(QuarkXor(result.highReg, left.highReg, right.highReg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkXor(result.reg, left.reg, (int32_t)right.immed));
		if (result.width == 8)
			out->AddInstruction(QuarkXor(result.highReg, left.highReg, (int32_t)(right.immed >> 32)));
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateShl(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkShl(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkShl(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateShr(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkShr(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkShr(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateSar(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, dest, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;
	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		out->AddInstruction(QuarkSar(result.reg, left.reg, right.reg, 0));
		break;
	case OPERANDREF_IMMED:
		out->AddInstruction(QuarkSar(result.reg, left.reg, (int32_t)right.immed));
		break;
	default:
		return false;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateNeg(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference src, dest, result;
	if (!Load(out, instr.params[1], src, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	if (result.width == 8)
	{
		out->AddInstruction(QuarkNot(result.reg, src.reg));
		out->AddInstruction(QuarkNot(result.highReg, src.highReg));
		out->AddInstruction(QuarkClrCC(3));
		out->AddInstruction(QuarkAddx(result.reg, result.reg, 1));
		out->AddInstruction(QuarkAddx(result.highReg, result.highReg, 0));
	}
	else
	{
		out->AddInstruction(QuarkNeg(result.reg, src.reg));
	}

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateNot(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference src, dest, result;
	if (!Load(out, instr.params[1], src, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	out->AddInstruction(QuarkNot(result.reg, src.reg));
	if (result.width == 8)
		out->AddInstruction(QuarkNot(result.highReg, src.highReg));

	return Move(out, dest, result, true);
}


bool OutputQuark::GenerateIfTrue(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference value;
	if (!Load(out, instr.params[0], value, true))
		return false;

	if (value.width == 8)
	{
		out->AddInstruction(QuarkCmp(QUARK_COND_NE, 0, value.reg, 0));
		out->AddInstruction(QuarkCmp(QUARK_COND_NE, 1 ,value.highReg, 0));
		out->AddInstruction(QuarkOrCC(0, 0, 1));
	}
	else
	{
		out->AddInstruction(QuarkCmp(QUARK_COND_NE, 0, value.reg, 0));
	}

	ConditionalJump(out, instr.params[1].block, 0, true);
	UnconditionalJump(out, instr.params[2].block);
	return true;
}


bool OutputQuark::GenerateIfLessThan(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left, true))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LT, 0, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkIcmp(QUARK_COND_EQ, 1, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LT, 0, left.reg, right.reg, 0));
		}
		else
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LT, 0, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LT, 0, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkIcmp(QUARK_COND_EQ, 1, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LT, 0, left.reg, (int32_t)right.immed));
		}
		else
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LT, 0, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfLessThanEqual(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left, true))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LT, 0, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkIcmp(QUARK_COND_EQ, 1, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LE, 0, left.reg, right.reg, 0));
		}
		else
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LE, 0, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LT, 0, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkIcmp(QUARK_COND_EQ, 1, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LE, 0, left.reg, (int32_t)right.immed));
		}
		else
		{
			out->AddInstruction(QuarkIcmp(QUARK_COND_LE, 0, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfBelow(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left, true))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LT, 0, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 1, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LT, 0, left.reg, right.reg, 0));
		}
		else
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LT, 0, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LT, 0, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 1, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LT, 0, left.reg, (int32_t)right.immed));
		}
		else
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LT, 0, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfBelowEqual(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left, true))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LT, 0, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 1, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LE, 0, left.reg, right.reg, 0));
		}
		else
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LE, 0, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LT, 0, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 1, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkCondCmp(1, 1, QUARK_COND_LE, 0, left.reg, (int32_t)right.immed));
		}
		else
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_LE, 0, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfEqual(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left, true))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 0, left.reg, right.reg, 0));
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 1, left.highReg, right.highReg, 0));
			out->AddInstruction(QuarkAndCC(0, 0, 1));
		}
		else
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 0, left.reg, right.reg, 0));
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 0, left.reg, (int32_t)right.immed));
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 1, left.highReg, (int32_t)(right.immed >> 32)));
			out->AddInstruction(QuarkAndCC(0, 0, 1));
		}
		else
		{
			out->AddInstruction(QuarkCmp(QUARK_COND_EQ, 0, left.reg, (int32_t)right.immed));
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateGoto(SymInstrBlock* out, const ILInstruction& instr)
{
	if (instr.params[0].cls != ILPARAM_BLOCK)
	{
		OperandReference value;
		if (!Load(out, instr.params[0], value))
			return false;

		switch (value.type)
		{
		case OPERANDREF_REG:
			out->AddInstruction(QuarkMov(SYMREG_IP, value.reg, 0));
			break;
		case OPERANDREF_IMMED:
			LoadImm(out, SYMREG_IP, (int32_t)value.immed);
			break;
		default:
			return false;
		}

		return true;
	}

	UnconditionalJump(out, instr.params[0].block);
	return true;
}


bool OutputQuark::GenerateCall(SymInstrBlock* out, const ILInstruction& instr)
{
	uint32_t retVal = SYMREG_NONE;
	uint32_t retValHigh = SYMREG_NONE;
	size_t pushSize = 0;

	if (instr.params[0].cls != ILPARAM_VOID)
	{
		retVal = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE);
		if (instr.params[0].GetWidth() == 8)
			retValHigh = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE_HIGH);
	}

	// Push parameters from right to left
	for (size_t i = instr.params.size() - 1; i >= 2; i--)
	{
		OperandReference param;
		if (!Load(out, instr.params[i], param, true))
			return false;

		if (param.width == 0)
		{
			// Indefinite width (used for immediates, for example), use native size
			param.width = 4;
		}

		if (m_settings.stackGrowsUp)
		{
			switch (param.width)
			{
			case 1:
			case 2:
			case 4:
				out->AddInstruction(QuarkStoreUpdate32(param.reg, SYMREG_SP, 4));
				pushSize += 4;
				break;
			case 8:
				out->AddInstruction(QuarkStore32(param.reg, SYMREG_SP, 4));
				out->AddInstruction(QuarkStoreUpdate32(param.highReg, SYMREG_SP, 8));
				pushSize += 8;
				break;
			default:
				return false;
			}
		}
		else
		{
			switch (param.width)
			{
			case 1:
			case 2:
			case 4:
				out->AddInstruction(QuarkStoreUpdate32(param.reg, SYMREG_SP, -4));
				pushSize += 4;
				break;
			case 8:
				out->AddInstruction(QuarkStore32(param.highReg, SYMREG_SP, -4));
				out->AddInstruction(QuarkStoreUpdate32(param.reg, SYMREG_SP, -8));
				pushSize += 8;
				break;
			default:
				return false;
			}
		}
	}

	// Perform function call
	if (instr.params[1].cls == ILPARAM_FUNC)
	{
		// Direct function call
		if (m_settings.encodePointers)
		{
			// Encoded pointer call, load return address then jump to function
			// FIXME: Symbolic representation of return address is not there yet
			return false;
			out->AddInstruction(QuarkAdd(SYMREG_LR, SYMREG_IP, 0));

			if (m_settings.encodePointers)
			{
				// Generate code to encode pointer
				ILParameter keyParam(m_settings.encodePointerKey);
				OperandReference key;
				if (!Load(out, keyParam, key, true))
					return false;
				out->AddInstruction(QuarkXor(SYMREG_LR, SYMREG_LR, key.reg, 0));
			}

			// Jump to function
			UnconditionalJump(out, instr.params[1].function->GetIL()[0], false);
		}
		else
		{
			// Normal call
			out->AddInstruction(QuarkCall(instr.params[1].function, instr.params[1].function->GetIL()[0], retVal, retValHigh));
		}
	}
	else
	{
		// Indirect function call
		OperandReference func, key;
		if (!Load(out, instr.params[1], func, true))
			return false;

		if (m_settings.encodePointers)
		{
			// Decode pointer before calling
			uint32_t encodedReg = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
			ILParameter keyParam(m_settings.encodePointerKey);
			if (!Load(out, keyParam, key, true))
				return false;
			out->AddInstruction(QuarkXor(encodedReg, func.reg, key.reg, 0));
			func.reg = encodedReg;

			// Load return address
			// FIXME: Symbolic representation of return address is not there yet
			return false;
			out->AddInstruction(QuarkAdd(SYMREG_LR, SYMREG_IP, 0));

			if (m_settings.encodePointers)
			{
				// Encode return address
				out->AddInstruction(QuarkXor(SYMREG_LR, SYMREG_LR, key.reg, 0));
			}

			// Jump to function
			out->AddInstruction(QuarkMov(SYMREG_IP, func.reg, 0));
		}
		else
		{
			out->AddInstruction(QuarkCall(func.reg, retVal, retValHigh));
		}
	}

	// Adjust stack pointer to pop off parameters
	if (pushSize != 0)
	{
		if (m_settings.stackGrowsUp)
			out->AddInstruction(QuarkSub(SYMREG_SP, SYMREG_SP, pushSize));
		else
			out->AddInstruction(QuarkAdd(SYMREG_SP, SYMREG_SP, pushSize));
	}

	// Store return value, if there is one
	if (instr.params[0].cls != ILPARAM_VOID)
	{
		OperandReference retValOperand;
		retValOperand.type = OPERANDREF_REG;
		retValOperand.width = instr.params[0].GetWidth();
		retValOperand.reg = retVal;
		retValOperand.highReg = retValHigh;

		OperandReference dest;
		if (!PrepareStore(out, instr.params[0], dest))
			return false;
		if (!Move(out, dest, retValOperand))
			return false;
	}

	return true;
}


bool OutputQuark::GenerateSignedConvert(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference src, dest, result;
	if (!Load(out, instr.params[1], src, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (src.width)
	{
	case 1:
		out->AddInstruction(QuarkSxb(result.reg, src.reg));
		break;
	case 2:
		out->AddInstruction(QuarkSxh(result.reg, src.reg));
		break;
	default:
		out->AddInstruction(QuarkMov(result.reg, src.reg, 0));
		break;
	}

	if (instr.params[0].GetWidth() == 8)
		out->AddInstruction(QuarkSar(result.highReg, result.reg, 31));

	return Move(out, dest, result);
}


bool OutputQuark::GenerateUnsignedConvert(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference src, dest, result;
	if (!Load(out, instr.params[1], src, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	switch (src.width)
	{
	case 1:
		out->AddInstruction(QuarkZxb(result.reg, src.reg));
		break;
	case 2:
		out->AddInstruction(QuarkZxh(result.reg, src.reg));
		break;
	default:
		out->AddInstruction(QuarkMov(result.reg, src.reg, 0));
		break;
	}

	if (instr.params[0].GetWidth() == 8)
		out->AddInstruction(QuarkLoadImm(result.highReg, 0));

	return Move(out, dest, result);
}


bool OutputQuark::GenerateReturn(SymInstrBlock* out, const ILInstruction& instr)
{
	uint32_t retVal = SYMREG_NONE;
	uint32_t retValHigh = SYMREG_NONE;
	OperandReference retValOperand;
	if (!Load(out, instr.params[0], retValOperand))
		return false;

	if (retValOperand.type == OPERANDREF_REG)
	{
		switch (retValOperand.width)
		{
		case 1:
		case 2:
		case 4:
			retVal = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE);
			out->AddInstruction(QuarkMov(retVal, retValOperand.reg, 0));
			break;
		case 8:
			retVal = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE);
			retValHigh = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE_HIGH);
			out->AddInstruction(QuarkMov(retVal, retValOperand.reg, 0));
			out->AddInstruction(QuarkMov(retValHigh, retValOperand.highReg, 0));
			break;
		default:
			return false;
		}
	}
	else
	{
		switch (retValOperand.width)
		{
		case 1:
		case 2:
		case 4:
			retVal = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE);
			LoadImm(out, retVal, (int32_t)retValOperand.immed);
			break;
		case 8:
			retVal = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE);
			retValHigh = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER_RETURN_VALUE_HIGH);
			LoadImm(out, retVal, (int32_t)retValOperand.immed);
			LoadImm(out, retValHigh, (int32_t)(retValOperand.immed >> 32));
			break;
		default:
			return false;
		}
	}

	if (!GenerateReturnVoid(out, instr))
		return false;

	out->AddInstruction(QuarkSymReturn(retVal, retValHigh));
	return true;
}


bool OutputQuark::GenerateReturnVoid(SymInstrBlock* out, const ILInstruction& instr)
{
	// Restore frame pointer (if present) and adjust stack
	if (m_framePointerEnabled)
	{
		out->AddInstruction(QuarkMov(SYMREG_SP, SYMREG_BP, 0));
	}
	else if (m_stackFrameSize != 0)
	{
		if (m_settings.stackGrowsUp)
			SubImm(out, SYMREG_SP, SYMREG_SP, m_stackFrameSize);
		else
			AddImm(out, SYMREG_SP, SYMREG_SP, m_stackFrameSize);
	}

	out->AddInstruction(QuarkRestoreCalleeSavedRegs());

	// Return to caller
	if (m_settings.encodePointers)
	{
		// Using encoded pointers, decode return address before returning
		OperandReference key;
		ILParameter keyParam(m_settings.encodePointerKey);
		if (!Load(out, keyParam, key, true))
			return false;

		out->AddInstruction(QuarkXor(SYMREG_LR, SYMREG_LR, key.reg, 0));
	}

	out->AddInstruction(QuarkMov(SYMREG_IP, SYMREG_LR, 0));
	return true;
}


bool OutputQuark::GenerateAlloca(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference dest, size;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!Load(out, instr.params[1], size))
		return false;

	OperandReference stack;
	stack.type = OPERANDREF_REG;
	stack.width = 4;
	stack.reg = SYMREG_SP;

	if (m_settings.stackGrowsUp)
	{
		out->AddInstruction(QuarkAdd(SYMREG_SP, SYMREG_SP, 4));
		if (!Move(out, dest, stack))
			return false;
		if (size.type == OPERANDREF_REG)
			out->AddInstruction(QuarkAdd(SYMREG_SP, SYMREG_SP, size.reg, 0));
		else
			out->AddInstruction(QuarkAdd(SYMREG_SP, SYMREG_SP, size.immed));
		out->AddInstruction(QuarkAnd(SYMREG_SP, SYMREG_SP, ~3));
	}
	else
	{
		if (size.type == OPERANDREF_REG)
			out->AddInstruction(QuarkSub(SYMREG_SP, SYMREG_SP, size.reg, 0));
		else
			out->AddInstruction(QuarkSub(SYMREG_SP, SYMREG_SP, size.immed));
		out->AddInstruction(QuarkAnd(SYMREG_SP, SYMREG_SP, ~3));
		if (!Move(out, dest, stack))
			return false;
	}

	return true;
}


bool OutputQuark::GenerateSyscall(SymInstrBlock* out, const ILInstruction& instr, bool twoDest)
{
	vector<uint32_t> writes, reads;
	size_t regIndex = 0;
	for (size_t i = twoDest ? 3 : 2; i < instr.params.size(); i++)
	{
		if (regIndex >= 8)
			return false;
		if (instr.params[i].cls == ILPARAM_UNDEFINED)
			continue;

		if (instr.params[i].GetWidth() == 8)
		{
			if (regIndex >= 7)
				return false;
		}

		OperandReference cur;
		if (!Load(out, instr.params[i], cur))
			return false;

		int reg, highReg;
		if (instr.params[i].GetWidth() == 8)
		{
			reg = m_symFunc->AddRegister(QUARKREGCLASS_SYSCALL_PARAM(regIndex));
			highReg = m_symFunc->AddRegister(QUARKREGCLASS_SYSCALL_PARAM(regIndex + 1));
			reads.push_back(reg);
			reads.push_back(highReg);
			regIndex += 2;
		}
		else
		{
			reg = m_symFunc->AddRegister(QUARKREGCLASS_SYSCALL_PARAM(regIndex));
			reads.push_back(reg);
			highReg = SYMREG_NONE;
			regIndex++;
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
	if (!Load(out, instr.params[twoDest ? 2 : 1], num))
		return false;

	OperandReference result, result2;
	result.type = OPERANDREF_REG;
	result.width = 4;
	result.reg = m_symFunc->AddRegister(QUARKREGCLASS_SYSCALL_RESULT_1);
	writes.push_back(result.reg);

	if (twoDest)
	{
		result2.type = OPERANDREF_REG;
		result2.width = 4;
		result2.reg = m_symFunc->AddRegister(QUARKREGCLASS_SYSCALL_RESULT_2);
		writes.push_back(result2.reg);
	}

	if (num.type == OPERANDREF_REG)
		out->AddInstruction(QuarkSyscallReg(num.reg, writes, reads));
	else
		out->AddInstruction(QuarkSyscallImmed(num.immed, writes, reads));

	OperandReference dest;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!Move(out, dest, result))
		return false;

	if (twoDest)
	{
		if (!PrepareStore(out, instr.params[1], dest))
			return false;
		if (!Move(out, dest, result2))
			return false;
	}

	return true;
}


bool OutputQuark::GenerateNextArg(SymInstrBlock* out, const ILInstruction& instr)
{
	if (m_settings.stackGrowsUp)
		return GenerateSub(out, instr);
	else
		return GenerateAdd(out, instr);
}


bool OutputQuark::GeneratePrevArg(SymInstrBlock* out, const ILInstruction& instr)
{
	if (m_settings.stackGrowsUp)
		return GenerateAdd(out, instr);
	else
		return GenerateSub(out, instr);
}


bool OutputQuark::GenerateByteSwap(SymInstrBlock* out, const ILInstruction& instr)
{
	OperandReference src, dest, result;
	if (!Load(out, instr.params[1], src, true))
		return false;
	if (!PrepareStore(out, instr.params[0], dest))
		return false;
	if (!GetDestRegister(out, dest, result))
		return false;

	uint32_t temp;
	switch (src.width)
	{
	case 2:
		out->AddInstruction(QuarkSwaph(result.reg, src.reg));
		break;
	case 4:
		out->AddInstruction(QuarkSwapw(result.reg, src.reg));
		break;
	case 8:
		temp = m_symFunc->AddRegister(QUARKREGCLASS_INTEGER);
		out->AddInstruction(QuarkSwapw(temp, src.reg));
		out->AddInstruction(QuarkSwapw(result.reg, src.highReg));
		result.highReg = temp;
		break;
	default:
		out->AddInstruction(QuarkMov(result.reg, src.reg, 0));
		break;
	}

	return Move(out, dest, result);
}


bool OutputQuark::GenerateBreakpoint(SymInstrBlock* out, const ILInstruction& instr)
{
	out->AddInstruction(new QuarkBreakpointInstr());
	return true;
}


bool OutputQuark::GenerateCodeBlock(SymInstrBlock* out, ILBlock* block)
{
	m_currentBlock = block;

	vector<ILInstruction>::iterator i;
	for (i = block->GetInstructions().begin(); i != block->GetInstructions().end(); i++)
	{
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
		case ILOP_SYSCALL:
			if (!GenerateSyscall(out, *i, false))
				goto fail;
			break;
		case ILOP_SYSCALL2:
			if (!GenerateSyscall(out, *i, true))
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


bool OutputQuark::GenerateCode(Function* func)
{
	QuarkSymInstrFunction symFunc(m_settings);
	m_func = func;
	m_symFunc = &symFunc;

	if (m_settings.stackGrowsUp)
	{
		// TODO: Fix this setting
		return false;
	}

	symFunc.InitializeBlocks(func);

	if (func->IsVariableSizedStackFrame())
		m_framePointerEnabled = true;
	else
		m_framePointerEnabled = false;

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

	// Create symbolic assembly registers for the variables in this function
	for (map<Variable*, int32_t>::iterator i = m_stackFrame.begin(); i != m_stackFrame.end(); i++)
	{
		m_stackVar[i->first] = m_symFunc->AddStackVar(i->second);

		if ((i->first->GetType()->GetClass() != TYPE_STRUCT) && (i->first->GetType()->GetClass() != TYPE_ARRAY))
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
					if (k->params[1].variable == i->first)
					{
						addressTaken = true;
						break;
					}
				}
			}

			if (addressTaken)
				continue;

			// Variable can be stored in a register
			uint32_t reg = m_symFunc->AddRegister((i->first->GetType()->GetClass() == TYPE_FLOAT) ?
				QUARKREGCLASS_FLOAT : QUARKREGCLASS_INTEGER, m_stackVar[i->first]);
			m_varReg[i->first] = reg;

			// 64-bit variables take two adjacent registers
			if ((i->first->GetType()->GetWidth() == 8) && (i->first->GetType()->GetClass() != TYPE_FLOAT))
				m_symFunc->AddRegister(QUARKREGCLASS_INTEGER, m_stackVar[i->first]);
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
			m_stackFrame[*var] = offset;
		else
			m_stackFrame[*var] = offset + m_stackFrameSize;

		if (m_settings.stackGrowsUp)
		{
			m_stackFrame[*var] = -m_stackFrame[*var];
			size_t paramSize = ((*var)->GetType()->GetWidth() + 3) & (~3);
			m_stackFrame[*var] += 4 - paramSize;
		}

		m_stackVar[*var] = m_symFunc->AddStackVar(m_stackFrame[*var]);

		// Adjust offset for next parameter
		offset += (*var)->GetType()->GetWidth();
		if (offset & 3)
			offset += 4 - (offset & 3);
	}

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
				SubImm(out, SYMREG_SP, SYMREG_SP, UNSAFE_STACK_PIVOT);
			}

			// Generate function prologue
			if (m_framePointerEnabled)
			{
				out->AddInstruction(QuarkSaveCalleeSavedRegs());
				out->AddInstruction(QuarkMov(SYMREG_BP, SYMREG_SP, 0));
				if (m_settings.stackGrowsUp)
					AddImm(out, SYMREG_SP, SYMREG_SP, m_stackFrameSize);
				else
					SubImm(out, SYMREG_SP, SYMREG_SP, m_stackFrameSize);
			}
			else
			{
				out->AddInstruction(QuarkSaveCalleeSavedRegs());
			}

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

