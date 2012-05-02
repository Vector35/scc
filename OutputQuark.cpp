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


void OutputQuark::LoadImm(OutputBlock* out, int dest, int32_t imm)
{
	if (IsSigned17Bit(imm))
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

	if (IsSigned11Bit(imm))
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

	if (IsSigned11Bit(imm))
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


void OutputQuark::RelativeLoadOverflowHandler(OutputBlock* out, Relocation& reloc)
{
	size_t start = reloc.instruction;
	uint32_t* oldInstr = (uint32_t*)((size_t)out->code + start);

	if (reloc.bitSize == 17)
	{
		// Was a 17-bit immediate, use a full 32-bit immediate
		uint32_t instrs[3];
		int32_t oldOffset = oldInstr[0] & 0x1ffff;
		int oldReg = (oldInstr[1] >> 17) & 31;
		if (oldOffset & 0x10000)
			oldOffset |= 0xfffe0000;

		oldOffset -= 4;

		instrs[0] = QUARK_EMIT_2(ldi, reloc.extra, oldOffset);
		instrs[1] = QUARK_EMIT_2(ldih, reloc.extra, (oldOffset < 0) ? -1 : 0);
		instrs[2] = QUARK_EMIT_3R(add, oldReg, IP, reloc.extra, 0);

		out->ReplaceInstruction(start, 8, instrs, 12, 0);

		reloc.type = DATA_RELOC_RELATIVE_64_SPLIT_FIELD;
		reloc.bitSize = 16;
		reloc.secondBitOffset = 0;
		reloc.secondBitSize = 16;
		reloc.secondBitShift = 16;
	}
	else
	{
		// Was an 11-bit immediate, extend to a 17-bit immediate
		uint32_t instrs[2];
		int32_t oldOffset = oldInstr[0] & 0x7ff;
		int oldReg = (oldInstr[0] >> 17) & 31;
		if (oldOffset & 0x400)
			oldOffset |= 0xfffff800;

		oldOffset -= 4;

		instrs[0] = QUARK_EMIT_2(ldi, reloc.extra, oldOffset);
		instrs[1] = QUARK_EMIT_3R(add, oldReg, IP, reloc.extra, 0);

		out->ReplaceInstruction(start, 4, instrs, 8, 0);

		reloc.bitSize = 17;
	}
}


bool OutputQuark::AccessVariableStorage(OutputBlock* out, const ILParameter& param, MemoryReference& ref)
{
	if (param.cls == ILPARAM_MEMBER)
	{
		if (!AccessVariableStorage(out, *param.parent, ref))
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
		ref.base = AllocateTemporaryRegister(out);
		ref.offset = 0;

		EMIT_3I(add, ref.base, IP, 0);
		Relocation reloc;
		reloc.type = DATA_RELOC_RELATIVE_32_FIELD;
		reloc.bitOffset = 0;
		reloc.bitSize = 11;
		reloc.bitShift = 0;
		reloc.offset = out->len - 4;
		reloc.extra = AllocateTemporaryRegister(out);
		reloc.instruction = reloc.offset;
		reloc.dataOffset = param.variable->GetDataSectionOffset();
		reloc.overflow = RelativeLoadOverflowHandler;
		out->relocs.push_back(reloc);
		return true;
	}

	map<Variable*, int32_t>::iterator i = m_stackFrame.find(param.variable);
	if (i == m_stackFrame.end())
		return false;
	if (m_framePointerEnabled)
	{
		ref.base = m_framePointer;
		ref.offset = i->second;
	}
	else
	{
		ref.base = m_stackPointer;
		ref.offset = i->second;
	}
	return true;
}


bool OutputQuark::Load(OutputBlock* out, const ILParameter& param, OperandReference& ref, bool forceReg)
{
	MemoryReference mem;
	ref.width = param.GetWidth();

	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		ref.type = OPERANDREF_REG;
		ref.width = param.GetWidth();
		ref.reg = AllocateTemporaryRegister(out);
		if (ref.width == 8)
			ref.highReg = AllocateTemporaryRegister(out);

		if (!AccessVariableStorage(out, param, mem))
			return false;

		if ((!IsSigned11Bit(mem.offset)) || (!IsSigned11Bit(mem.offset + ref.width - 1)))
		{
			int addr = AllocateTemporaryRegister(out);
			AddImm(out, addr, mem.base, mem.offset);
			mem.base = addr;
			mem.offset = 0;
		}

		switch (ref.width)
		{
		case 1:
			EMIT_3I(ldb, ref.reg, mem.base, mem.offset);
			break;
		case 2:
			EMIT_3I(ldh, ref.reg, mem.base, mem.offset);
			break;
		case 4:
			EMIT_3I(ldw, ref.reg, mem.base, mem.offset);
			break;
		case 8:
			EMIT_3I(ldw, ref.reg, mem.base, mem.offset);
			EMIT_3I(ldw, ref.highReg, mem.base, mem.offset + 4);
			break;
		default:
			return false;
		}
		return true;
	case ILPARAM_INT:
		if ((!forceReg) && IsSigned11Bit(param.integerValue))
		{
			ref.type = OPERANDREF_IMMED;
			ref.immed = param.integerValue;
			return true;
		}

		ref.type = OPERANDREF_REG;
		ref.reg = AllocateTemporaryRegister(out);
		if (ref.width == 8)
			ref.highReg = AllocateTemporaryRegister(out);

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
		ref.reg = AllocateTemporaryRegister(out);
		LoadImm(out, ref.reg, param.boolValue ? 1 : 0);
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


bool OutputQuark::Store(OutputBlock* out, const ILParameter& param, OperandReference ref)
{
	MemoryReference mem;
	switch (param.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!AccessVariableStorage(out, param, mem))
			return false;

		if ((!IsSigned11Bit(mem.offset)) || (!IsSigned11Bit(mem.offset + ref.width - 1)))
		{
			int addr = AllocateTemporaryRegister(out);
			AddImm(out, addr, mem.base, mem.offset);
			mem.base = addr;
			mem.offset = 0;
		}

		if (ref.type == OPERANDREF_IMMED)
		{
			if (ref.width < 8)
			{
				int reg = AllocateTemporaryRegister(out);
				LoadImm(out, reg, (int32_t)ref.immed);
				ref.type = OPERANDREF_REG;
				ref.reg = reg;
			}
			else
			{
				int reg = AllocateTemporaryRegister(out);
				int highReg = AllocateTemporaryRegister(out);
				LoadImm(out, reg, (int32_t)ref.immed);
				LoadImm(out, highReg, (int32_t)(ref.immed >> 32));
				ref.type = OPERANDREF_REG;
				ref.reg = reg;
				ref.highReg = highReg;
			}
		}

		switch (ref.width)
		{
		case 1:
			EMIT_3I(stb, ref.reg, mem.base, mem.offset);
			break;
		case 2:
			EMIT_3I(sth, ref.reg, mem.base, mem.offset);
			break;
		case 4:
			EMIT_3I(stw, ref.reg, mem.base, mem.offset);
			break;
		case 8:
			EMIT_3I(stw, ref.reg, mem.base, mem.offset);
			EMIT_3I(stw, ref.highReg, mem.base, mem.offset + 4);
			break;
		default:
			return false;
		}
		return true;
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


void OutputQuark::ConditionalJump(OutputBlock* out, ILBlock* block, int cond, bool value)
{
	if (value)
		EMIT_COND_1(jmp, QUARK_IF_TRUE(cond), 0);
	else
		EMIT_COND_1(jmp, QUARK_IF_FALSE(cond), 0);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
	reloc.overflow = NULL;
	reloc.instruction = out->len - 4;
	reloc.offset = out->len - 4;
	reloc.target = block;
	reloc.bitOffset = 0;
	reloc.bitSize = 22;
	reloc.bitShift = 2;
	out->relocs.push_back(reloc);
}


void OutputQuark::UnconditionalJump(OutputBlock* out, ILBlock* block, bool canOmit)
{
	if (canOmit && (block->GetGlobalIndex() == (m_currentBlock->GetGlobalIndex() + 1)))
	{
		// The destination block is the one just after the current one, just fall through
		return;
	}

	EMIT_1(jmp, 0);

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
	reloc.overflow = NULL;
	reloc.instruction = out->len - 4;
	reloc.offset = out->len - 4;
	reloc.target = block;
	reloc.bitOffset = 0;
	reloc.bitSize = 22;
	reloc.bitShift = 2;
	out->relocs.push_back(reloc);
}


bool OutputQuark::GenerateAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference src;
	if (!Load(out, instr.params[1], src))
		return false;
	return Store(out, instr.params[0], src);
}


bool OutputQuark::GenerateAddressOf(OutputBlock* out, const ILInstruction& instr)
{
	MemoryReference ref;
	if (!AccessVariableStorage(out, instr.params[1], ref))
		return false;

	OperandReference addr;
	addr.type = OPERANDREF_REG;
	addr.width = 4;
	addr.reg = AllocateTemporaryRegister(out);
	AddImm(out, addr.reg, ref.base, ref.offset);
	return Store(out, instr.params[0], addr);
}


bool OutputQuark::GenerateAddressOfMember(OutputBlock* out, const ILInstruction& instr)
{
	MemoryReference ref;
	if (!AccessVariableStorage(out, instr.params[1], ref))
		return false;

	if (!instr.params[2].structure)
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	OperandReference addr;
	addr.type = OPERANDREF_REG;
	addr.width = 4;
	addr.reg = AllocateTemporaryRegister(out);
	AddImm(out, addr.reg, ref.base, ref.offset + member->offset);
	return Store(out, instr.params[0], addr);
}


bool OutputQuark::GenerateDeref(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference addr;
	if (!Load(out, instr.params[1], addr, true))
		return false;

	OperandReference value;
	value.type = OPERANDREF_REG;
	value.width = instr.params[0].GetWidth();
	value.reg = AllocateTemporaryRegister(out);
	if (value.width == 8)
		value.highReg = AllocateTemporaryRegister(out);

	switch (value.width)
	{
	case 1:
		EMIT_3I(ldb, value.reg, addr.reg, 0);
		break;
	case 2:
		EMIT_3I(ldh, value.reg, addr.reg, 0);
		break;
	case 4:
		EMIT_3I(ldw, value.reg, addr.reg, 0);
		break;
	case 8:
		EMIT_3I(ldw, value.reg, addr.reg, 0);
		EMIT_3I(ldw, value.highReg, addr.reg, 4);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], value);
}


bool OutputQuark::GenerateDerefMember(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference addr;
	if (!Load(out, instr.params[1], addr, true))
		return false;

	OperandReference value;
	value.type = OPERANDREF_REG;
	value.width = instr.params[0].GetWidth();
	value.reg = AllocateTemporaryRegister(out);
	if (value.width == 8)
		value.highReg = AllocateTemporaryRegister(out);

	if (!instr.params[2].structure)
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	int32_t offset = member->offset;
	if ((!IsSigned11Bit(offset)) || (!IsSigned11Bit(offset + value.width - 1)))
	{
		int reg = AllocateTemporaryRegister(out);
		LoadImm(out, reg, offset);
		EMIT_3R(add, addr.reg, addr.reg, reg, 0);
		offset = 0;
	}

	switch (value.width)
	{
	case 1:
		EMIT_3I(ldb, value.reg, addr.reg, offset);
		break;
	case 2:
		EMIT_3I(ldh, value.reg, addr.reg, offset);
		break;
	case 4:
		EMIT_3I(ldw, value.reg, addr.reg, offset);
		break;
	case 8:
		EMIT_3I(ldw, value.reg, addr.reg, offset);
		EMIT_3I(ldw, value.highReg, addr.reg, offset + 4);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], value);
}


bool OutputQuark::GenerateDerefAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference addr, value;
	if (!Load(out, instr.params[0], addr, true))
		return false;
	if (!Load(out, instr.params[1], value, true))
		return false;

	switch (value.width)
	{
	case 1:
		EMIT_3I(stb, value.reg, addr.reg, 0);
		break;
	case 2:
		EMIT_3I(sth, value.reg, addr.reg, 0);
		break;
	case 4:
		EMIT_3I(stw, value.reg, addr.reg, 0);
		break;
	case 8:
		EMIT_3I(stw, value.reg, addr.reg, 0);
		EMIT_3I(stw, value.highReg, addr.reg, 4);
		break;
	default:
		return false;
	}

	return true;
}


bool OutputQuark::GenerateDerefMemberAssign(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference addr, value;
	if (!Load(out, instr.params[0], addr, true))
		return false;
	if (!Load(out, instr.params[1], value, true))
		return false;

	const StructMember* member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
	if (!member)
		return false;

	int32_t offset = member->offset;
	if ((!IsSigned11Bit(offset)) || (!IsSigned11Bit(offset + value.width - 1)))
	{
		int reg = AllocateTemporaryRegister(out);
		LoadImm(out, reg, offset);
		EMIT_3R(add, addr.reg, addr.reg, reg, 0);
		offset = 0;
	}

	switch (value.width)
	{
	case 1:
		EMIT_3I(stb, value.reg, addr.reg, offset);
		break;
	case 2:
		EMIT_3I(sth, value.reg, addr.reg, offset);
		break;
	case 4:
		EMIT_3I(stw, value.reg, addr.reg, offset);
		break;
	case 8:
		EMIT_3I(stw, value.reg, addr.reg, offset);
		EMIT_3I(stw, value.highReg, addr.reg, offset + 4);
		break;
	default:
		return false;
	}

	return true;
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
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	uint32_t shiftCount;
	switch (right.type)
	{
	case OPERANDREF_REG:
		if (IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			EMIT_3R(add, result.reg, left.reg, right.reg, shiftCount);
		else if (IsSigned11Bit(instr.params[3].integerValue))
		{
			EMIT_3I(mul, right.reg, right.reg, (int32_t)instr.params[3].integerValue);
			EMIT_3R(add, result.reg, left.reg, right.reg, 0);
		}
		else
		{
			int reg = AllocateTemporaryRegister(out);
			LoadImm(out, reg, (int32_t)instr.params[3].integerValue);
			EMIT_3R(mul, right.reg, right.reg, reg, 0);
			EMIT_3R(add, result.reg, left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (IsSigned11Bit(instr.params[3].integerValue * right.immed))
			EMIT_3I(add, result.reg, left.reg, (int32_t)(instr.params[3].integerValue * right.immed));
		else
		{
			int reg = AllocateTemporaryRegister(out);
			LoadImm(out, reg, (int32_t)(instr.params[3].integerValue * right.immed));
			EMIT_3R(add, result.reg, left.reg, reg, 0);
		}
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GeneratePtrSub(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	uint32_t shiftCount;
	switch (right.type)
	{
	case OPERANDREF_REG:
		if (IsPowerOfTwo(instr.params[3].integerValue, shiftCount))
			EMIT_3R(sub, result.reg, left.reg, right.reg, shiftCount);
		else if (IsSigned11Bit(instr.params[3].integerValue))
		{
			EMIT_3I(mul, right.reg, right.reg, (int32_t)instr.params[3].integerValue);
			EMIT_3R(sub, result.reg, left.reg, right.reg, 0);
		}
		else
		{
			int reg = AllocateTemporaryRegister(out);
			LoadImm(out, reg, (int32_t)instr.params[3].integerValue);
			EMIT_3R(mul, right.reg, right.reg, reg, 0);
			EMIT_3R(sub, result.reg, left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (IsSigned11Bit(instr.params[3].integerValue * right.immed))
			EMIT_3I(sub, result.reg, left.reg, (int32_t)(instr.params[3].integerValue * right.immed));
		else
		{
			int reg = AllocateTemporaryRegister(out);
			LoadImm(out, reg, (int32_t)(instr.params[3].integerValue * right.immed));
			EMIT_3R(sub, result.reg, left.reg, reg, 0);
		}
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GeneratePtrDiff(OutputBlock* out, const ILInstruction& instr)
{
	return false;
}


bool OutputQuark::GenerateAdd(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);
	if (result.width == 8)
		result.highReg = AllocateTemporaryRegister(out);

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (result.width == 8)
		{
			EMIT_1(clrcc, 3);
			EMIT_3R(addx, result.reg, left.reg, right.reg, 0);
			EMIT_3R(addx, result.highReg, left.highReg, right.highReg, 0);
		}
		else
		{
			EMIT_3R(add, result.reg, left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (result.width == 8)
		{
			EMIT_1(clrcc, 3);
			EMIT_3I(addx, result.reg, left.reg, (int32_t)right.immed);
			EMIT_3I(addx, result.highReg, left.highReg, (int32_t)(right.immed >> 32));
		}
		else
		{
			EMIT_3I(add, result.reg, left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateSub(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);
	if (result.width == 8)
		result.highReg = AllocateTemporaryRegister(out);

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (result.width == 8)
		{
			EMIT_1(clrcc, 3);
			EMIT_3R(subx, result.reg, left.reg, right.reg, 0);
			EMIT_3R(subx, result.highReg, left.highReg, right.highReg, 0);
		}
		else
		{
			EMIT_3R(sub, result.reg, left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (result.width == 8)
		{
			EMIT_1(clrcc, 3);
			EMIT_3I(subx, result.reg, left.reg, (int32_t)right.immed);
			EMIT_3I(subx, result.highReg, left.highReg, (int32_t)(right.immed >> 32));
		}
		else
		{
			EMIT_3I(sub, result.reg, left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateSignedMult(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(mul, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(mul, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateUnsignedMult(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(mul, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(mul, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateSignedDiv(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(idiv, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(idiv, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateUnsignedDiv(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(div, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(div, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateSignedMod(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(imod, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(imod, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateUnsignedMod(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(mod, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(mod, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateAnd(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);
	if (result.width == 8)
		result.highReg = AllocateTemporaryRegister(out);

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(and, result.reg, left.reg, right.reg, 0);
		if (result.width == 8)
			EMIT_3R(and, result.highReg, left.highReg, right.highReg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(and, result.reg, left.reg, (int32_t)right.immed);
		if (result.width == 8)
			EMIT_3I(and, result.highReg, left.highReg, (int32_t)(right.immed >> 32));
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateOr(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);
	if (result.width == 8)
		result.highReg = AllocateTemporaryRegister(out);

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(or, result.reg, left.reg, right.reg, 0);
		if (result.width == 8)
			EMIT_3R(or, result.highReg, left.highReg, right.highReg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(or, result.reg, left.reg, (int32_t)right.immed);
		if (result.width == 8)
			EMIT_3I(or, result.highReg, left.highReg, (int32_t)(right.immed >> 32));
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateXor(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);
	if (result.width == 8)
		result.highReg = AllocateTemporaryRegister(out);

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(xor, result.reg, left.reg, right.reg, 0);
		if (result.width == 8)
			EMIT_3R(xor, result.highReg, left.highReg, right.highReg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(xor, result.reg, left.reg, (int32_t)right.immed);
		if (result.width == 8)
			EMIT_3I(xor, result.highReg, left.highReg, (int32_t)(right.immed >> 32));
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateShl(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(shl, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(shl, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateShr(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(shr, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(shr, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateSar(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right, result;
	if (!Load(out, instr.params[1], left, true))
		return false;
	if (!Load(out, instr.params[2], right))
		return false;

	result.type = OPERANDREF_REG;
	result.width = left.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		EMIT_3R(sar, result.reg, left.reg, right.reg, 0);
		break;
	case OPERANDREF_IMMED:
		EMIT_3I(sar, result.reg, left.reg, (int32_t)right.immed);
		break;
	default:
		return false;
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateNeg(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference src, result;
	if (!Load(out, instr.params[1], src, true))
		return false;

	result.type = OPERANDREF_REG;
	result.width = src.width;
	result.reg = AllocateTemporaryRegister(out);

	if (result.width == 8)
	{
		result.highReg = AllocateTemporaryRegister(out);
		EMIT_2(not, result.reg, src.reg);
		EMIT_2(not, result.highReg, src.highReg);
		EMIT_1(clrcc, 3);
		EMIT_3I(addx, result.reg, result.reg, 1);
		EMIT_3I(addx, result.highReg, result.highReg, 0);
	}
	else
	{
		EMIT_2(not, result.reg, src.reg);
	}

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateNot(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference src, result;
	if (!Load(out, instr.params[1], src, true))
		return false;

	result.type = OPERANDREF_REG;
	result.width = src.width;
	result.reg = AllocateTemporaryRegister(out);
	if (result.width == 8)
		result.highReg = AllocateTemporaryRegister(out);

	EMIT_2(not, result.reg, src.reg);
	if (result.width == 8)
		EMIT_2(not, result.highReg, src.highReg);

	return Store(out, instr.params[0], result);
}


bool OutputQuark::GenerateIfTrue(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference value;
	if (!Load(out, instr.params[0], value))
		return false;

	if (value.width == 8)
	{
		EMIT_3I(cmp, QUARK_COND(QUARK_COND_NE, 0), value.reg, 0);
		EMIT_3I(cmp, QUARK_COND(QUARK_COND_NE, 1), value.highReg, 0);
		EMIT_3(orcc, 0, 0, 1);
	}
	else
	{
		EMIT_3I(cmp, QUARK_COND(QUARK_COND_NE, 0), value.reg, 0);
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfLessThan(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			EMIT_3R(icmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, right.reg, 0);
			EMIT_3R(icmp, QUARK_COND(QUARK_COND_EQ, 1), left.reg, right.reg, 0);
			EMIT_COND_3R(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LT, 0), left.highReg, right.highReg, 0);
		}
		else
		{
			EMIT_3R(icmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			EMIT_3I(icmp, QUARK_COND(QUARK_COND_LT, 0), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_3I(icmp, QUARK_COND(QUARK_COND_EQ, 1), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_COND_3I(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LT, 0), left.reg, (int32_t)right.immed);
		}
		else
		{
			EMIT_3I(icmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfLessThanEqual(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			EMIT_3R(icmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, right.reg, 0);
			EMIT_3R(icmp, QUARK_COND(QUARK_COND_EQ, 1), left.reg, right.reg, 0);
			EMIT_COND_3R(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LE, 0), left.highReg, right.highReg, 0);
		}
		else
		{
			EMIT_3R(icmp, QUARK_COND(QUARK_COND_LE, 0), left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			EMIT_3I(icmp, QUARK_COND(QUARK_COND_LT, 0), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_3I(icmp, QUARK_COND(QUARK_COND_EQ, 1), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_COND_3I(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LE, 0), left.reg, (int32_t)right.immed);
		}
		else
		{
			EMIT_3I(icmp, QUARK_COND(QUARK_COND_LE, 0), left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfBelow(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, right.reg, 0);
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_EQ, 1), left.reg, right.reg, 0);
			EMIT_COND_3R(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LT, 0), left.highReg, right.highReg, 0);
		}
		else
		{
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_LT, 0), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_EQ, 1), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_COND_3I(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LT, 0), left.reg, (int32_t)right.immed);
		}
		else
		{
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfBelowEqual(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_LT, 0), left.reg, right.reg, 0);
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_EQ, 1), left.reg, right.reg, 0);
			EMIT_COND_3R(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LE, 0), left.highReg, right.highReg, 0);
		}
		else
		{
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_LE, 0), left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_LT, 0), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_EQ, 1), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_COND_3I(cmp, QUARK_IF_TRUE(1), QUARK_COND(QUARK_COND_LE, 0), left.reg, (int32_t)right.immed);
		}
		else
		{
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_LE, 0), left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateIfEqual(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference left, right;
	if (!Load(out, instr.params[0], left))
		return false;
	if (!Load(out, instr.params[1], right))
		return false;

	switch (right.type)
	{
	case OPERANDREF_REG:
		if (right.width == 8)
		{
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_EQ, 0), left.reg, right.reg, 0);
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_EQ, 1), left.highReg, right.highReg, 0);
			EMIT_3(andcc, 0, 0, 1);
		}
		else
		{
			EMIT_3R(cmp, QUARK_COND(QUARK_COND_EQ, 0), left.reg, right.reg, 0);
		}
		break;
	case OPERANDREF_IMMED:
		if (right.width == 8)
		{
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_EQ, 0), left.reg, (int32_t)right.immed);
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_EQ, 1), left.highReg, (int32_t)(right.immed >> 32));
			EMIT_3(andcc, 0, 0, 1);
		}
		else
		{
			EMIT_3I(cmp, QUARK_COND(QUARK_COND_EQ, 0), left.reg, (int32_t)right.immed);
		}
		break;
	default:
		return false;
	}

	ConditionalJump(out, instr.params[2].block, 0, true);
	UnconditionalJump(out, instr.params[3].block);
	return true;
}


bool OutputQuark::GenerateGoto(OutputBlock* out, const ILInstruction& instr)
{
	if (instr.params[0].cls != ILPARAM_BLOCK)
	{
		OperandReference value;
		if (!Load(out, instr.params[0], value))
			return false;

		switch (value.type)
		{
		case OPERANDREF_REG:
			EMIT_2R(mov, IP, value.reg, 0);
			break;
		case OPERANDREF_IMMED:
			EMIT_2I(mov, IP, (int32_t)value.immed);
			break;
		default:
			return false;
		}

		return true;
	}

	UnconditionalJump(out, instr.params[0].block);
	return true;
}


bool OutputQuark::GenerateCall(OutputBlock* out, const ILInstruction& instr)
{
	size_t pushSize = 0;

	// Push parameters from right to left
	for (size_t i = instr.params.size() - 1; i >= 2; i--)
	{
		memset(m_alloc, 0, sizeof(m_alloc));
		ClearReservedRegisters(out);

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
				EMIT_3I(stwu, param.reg, m_stackPointer, 4);
				pushSize += 4;
			case 8:
				EMIT_3I(stw, param.reg, m_stackPointer, 4);
				EMIT_3I(stwu, param.highReg, m_stackPointer, 8);
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
				EMIT_3I(stwu, param.reg, m_stackPointer, -4);
				pushSize += 4;
			case 8:
				EMIT_3I(stw, param.highReg, m_stackPointer, -4);
				EMIT_3I(stwu, param.reg, m_stackPointer, -8);
				pushSize += 8;
				break;
			default:
				return false;
			}
		}
	}

	memset(m_alloc, 0, sizeof(m_alloc));
	ClearReservedRegisters(out);

	// Perform function call
	if (instr.params[1].cls == ILPARAM_FUNC)
	{
		// Direct function call
		if (m_settings.encodePointers)
		{
			// Encoded pointer call, load return address then jump to function
			EMIT_3I(add, LR, IP, 0);

			Relocation reloc;
			reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
			reloc.bitOffset = 0;
			reloc.bitSize = 11;
			reloc.bitShift = 0;
			reloc.overflow = NULL;
			reloc.offset = out->len - 4;
			reloc.target = NULL;

			size_t beforeLen = out->len;

			if (m_settings.encodePointers)
			{
				// Generate code to encode pointer
				ILParameter keyParam(m_settings.encodePointerKey);
				OperandReference key;
				if (!Load(out, keyParam, key, true))
					return false;
				EMIT_3R(xor, LR, LR, key.reg, 0);
			}

			// Jump to function
			UnconditionalJump(out, instr.params[1].function->GetIL()[0], false);

			// Fix up relocation to point to return address
			size_t afterLen = out->len;
			reloc.start = beforeLen;
			reloc.end = afterLen;
			*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
			out->relocs.push_back(reloc);
		}
		else
		{
			// Normal call
			EMIT_1I(call, 0);

			Relocation reloc;
			reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
			reloc.bitOffset = 0;
			reloc.bitSize = 22;
			reloc.bitShift = 2;
			reloc.overflow = NULL;
			reloc.offset = out->len - 4;
			reloc.target = instr.params[1].function->GetIL()[0];
			out->relocs.push_back(reloc);
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
			ILParameter keyParam(m_settings.encodePointerKey);
			if (!Load(out, keyParam, key, true))
				return false;
			EMIT_3R(xor, func.reg, func.reg, key.reg, 0);

			// Load return address
			EMIT_3I(add, LR, IP, 0);

			Relocation reloc;
			reloc.type = CODE_RELOC_RELATIVE_32;
			reloc.bitOffset = 0;
			reloc.bitSize = 11;
			reloc.bitShift = 0;
			reloc.overflow = NULL;
			reloc.offset = out->len - 4;
			reloc.target = NULL;

			size_t beforeLen = out->len;

			if (m_settings.encodePointers)
			{
				// Encode return address
				EMIT_3R(xor, LR, LR, key.reg, 0);
			}

			// Jump to function
			EMIT_2R(mov, IP, func.reg, 0);

			// Fix up relocation to point to return address
			size_t afterLen = out->len;
			reloc.start = beforeLen;
			reloc.end = afterLen;
			*(int32_t*)((size_t)out->code + reloc.offset) += (int32_t)(afterLen - beforeLen);
			out->relocs.push_back(reloc);
		}
		else
		{
			EMIT_1R(call, func.reg);
		}
	}

	// Adjust stack pointer to pop off parameters
	if (pushSize != 0)
	{
		if (m_settings.stackGrowsUp)
			EMIT_3I(sub, m_stackPointer, m_stackPointer, pushSize);
		else
			EMIT_3I(add, m_stackPointer, m_stackPointer, pushSize);
	}

	// Store return value, if there is one
	if (instr.params[0].cls != ILPARAM_VOID)
	{
		if (instr.params[0].GetWidth() > 4)
			ReserveRegisters(out, 1, 2, NONE);
		else
			ReserveRegisters(out, 1, NONE);

		OperandReference retVal;
		retVal.type = OPERANDREF_REG;
		retVal.width = instr.params[0].GetWidth();
		retVal.reg = 1;
		if (retVal.width == 8)
			retVal.highReg = 2;

		if (!Store(out, instr.params[0], retVal))
			return false;
	}

	return true;
}


bool OutputQuark::GenerateSignedConvert(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference src;
	if (!Load(out, instr.params[1], src, true))
		return false;

	switch (src.width)
	{
	case 1:
		EMIT_2(sxb, src.reg, src.reg);
		break;
	case 2:
		EMIT_2(sxh, src.reg, src.reg);
		break;
	default:
		break;
	}

	if (instr.params[0].GetWidth() == 8)
	{
		src.highReg = AllocateTemporaryRegister(out);
		EMIT_3I(sar, src.highReg, src.reg, 31);
	}
	src.width = instr.params[0].GetWidth();

	return Store(out, instr.params[0], src);
}


bool OutputQuark::GenerateUnsignedConvert(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference src;
	if (!Load(out, instr.params[1], src))
		return false;

	if (instr.params[0].GetWidth() == 8)
	{
		src.highReg = AllocateTemporaryRegister(out);
		EMIT_2(ldi, src.highReg, 0);
	}
	src.width = instr.params[0].GetWidth();

	return Store(out, instr.params[0], src);
}


bool OutputQuark::GenerateReturn(OutputBlock* out, const ILInstruction& instr)
{
	OperandReference retVal;
	if (!Load(out, instr.params[0], retVal))
		return false;

	switch (retVal.width)
	{
	case 1:
	case 2:
	case 4:
		EMIT_2R(mov, 1, retVal.reg, 0);
		break;
	case 8:
		EMIT_2R(mov, 1, retVal.reg, 0);
		EMIT_2R(mov, 2, retVal.highReg, 0);
		break;
	default:
		return false;
	}

	return GenerateReturnVoid(out, instr);
}


bool OutputQuark::GenerateReturnVoid(OutputBlock* out, const ILInstruction& instr)
{
	ReserveRegisters(out, 1, 2, NONE);

	// Restore frame pointer (if present) and adjust stack
	if (m_framePointerEnabled)
	{
		EMIT_2R(mov, m_stackPointer, m_framePointer, 0);
		if (m_settings.stackGrowsUp)
		{
			EMIT_3I(ldw, m_framePointer, m_stackPointer, 0);
			EMIT_3I(sub, m_stackPointer, m_stackPointer, 4);
		}
		else
		{
			EMIT_3I(ldwu, m_framePointer, m_stackPointer, 0);
		}
	}
	else if (m_stackFrameSize != 0)
	{
		if (m_settings.stackGrowsUp)
			SubImm(out, m_stackPointer, m_stackPointer, m_stackFrameSize);
		else
			AddImm(out, m_stackPointer, m_stackPointer, m_stackFrameSize);
	}

	// Pop return address
	if (m_settings.stackGrowsUp)
	{
		EMIT_3I(ldw, LR, m_stackPointer, 0);
		EMIT_3I(sub, m_stackPointer, m_stackPointer, 4);
	}
	else
	{
		EMIT_3I(ldwu, LR, m_stackPointer, 0);
	}

	if (m_settings.encodePointers)
	{
		// Using encoded pointers, decode return address before returning
		OperandReference key;
		ILParameter keyParam(m_settings.encodePointerKey);
		if (!Load(out, keyParam, key, true))
			return false;

		EMIT_3R(xor, LR, LR, key.reg, 0);
	}

	EMIT_2R(mov, IP, LR, 0);
	return true;
}


bool OutputQuark::GenerateAlloca(OutputBlock* out, const ILInstruction& instr)
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
				EMIT_3I(stwu, LR, m_stackPointer, m_settings.stackGrowsUp ? 4 : -4);
				EMIT_3I(stwu, m_framePointer, m_stackPointer, m_settings.stackGrowsUp ? 4 : -4);
				EMIT_2R(mov, m_framePointer, m_stackPointer, 0);
				if (m_settings.stackGrowsUp)
					AddImm(out, m_stackPointer, m_stackPointer, m_stackFrameSize);
				else
					SubImm(out, m_stackPointer, m_stackPointer, m_stackFrameSize);
			}
			else
			{
				EMIT_3I(stwu, LR, m_stackPointer, m_settings.stackGrowsUp ? 4 : -4);
			}

			first = false;
		}

		if (!GenerateCodeBlock(out, *i))
			return false;

		(*i)->SetOutputBlock(out);
	}

	return true;
}

