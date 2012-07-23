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
#include "QuarkSymInstr.h"
#include "Struct.h"
#include "asmquark.h"

using namespace std;


QuarkSymInstr::QuarkSymInstr()
{
}


void QuarkSymInstr::RelativeLoadOverflowHandler(OutputBlock* out, Relocation& reloc)
{
	size_t start = reloc.instruction;
	uint32_t* oldInstr = (uint32_t*)((size_t)out->code + start);

	if (reloc.bitSize == 17)
	{
		// Was a 17-bit immediate, use a full 32-bit immediate
		uint32_t instrs[3];
		uint32_t oldOpcode = (oldInstr[0] >> 22) & 0x3f;
		int32_t oldOffset = oldInstr[0] & 0x1ffff;
		int oldReg = (oldInstr[1] >> 17) & 31;
		if (oldOffset & 0x10000)
			oldOffset |= 0xfffe0000;

		oldOffset -= 4;

		instrs[0] = QUARK_EMIT_2(ldi, reloc.extra, oldOffset);
		instrs[1] = QUARK_EMIT_2(ldih, reloc.extra, (oldOffset < 0) ? -1 : 0);
		instrs[2] = __QUARK_INSTR(oldOpcode, oldReg, 31, reloc.extra, 0);

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
		uint32_t oldOpcode = (oldInstr[0] >> 22) & 0x3f;
		int32_t oldOffset = oldInstr[0] & 0x7ff;
		int oldReg = (oldInstr[0] >> 17) & 31;
		if (oldOffset & 0x400)
			oldOffset |= 0xfffff800;

		oldOffset -= 4;

		instrs[0] = QUARK_EMIT_2(ldi, reloc.extra, oldOffset);
		instrs[1] = __QUARK_INSTR(oldOpcode, oldReg, 31, reloc.extra, 0);

		out->ReplaceInstruction(start, 4, instrs, 8, 0);

		reloc.bitSize = 17;
	}
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


QuarkMovInstr::QuarkMovInstr(uint32_t a, uint32_t b, uint32_t shift)
{
	SetOperation(0x1f00);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddImmediateOperand(shift);
}


QuarkMovInstr::QuarkMovInstr(uint32_t a, int32_t immed)
{
	SetOperation(0x1f00);
	AddWriteRegisterOperand(a);
	AddImmediateOperand(immed);
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


QuarkStackLoadInstr::QuarkStackLoadInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddStackVarOperand(var, offset);
	AddTemporaryRegisterOperand(scratch);
}


QuarkGlobalLoadInstr::QuarkGlobalLoadInstr(uint32_t op, uint32_t a, uint32_t b, int64_t offset, uint32_t scratch)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddGlobalVarOperand(offset);
	AddTemporaryRegisterOperand(scratch);
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


QuarkStackStoreInstr::QuarkStackStoreInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddStackVarOperand(var, offset);
	AddTemporaryRegisterOperand(scratch);
}


QuarkGlobalStoreInstr::QuarkGlobalStoreInstr(uint32_t op, uint32_t a, uint32_t b, int64_t offset, uint32_t scratch)
{
	SetOperation(op);
	EnableFlag(SYMFLAG_MEMORY_BARRIER);
	AddReadRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddGlobalVarOperand(offset);
	AddTemporaryRegisterOperand(scratch);
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


QuarkBranchInstr::QuarkBranchInstr(Function* func, ILBlock* block)
{
	SetOperation(0x16);
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


QuarkCallInstr::QuarkCallInstr(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh)
{
	SetOperation(0x17);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	EnableFlag(SYMFLAG_CALL);
	AddBlockOperand(func, block);
	if (retVal != SYMREG_NONE)
		AddWriteRegisterOperand(retVal);
	if (retValHigh != SYMREG_NONE)
		AddWriteRegisterOperand(retValHigh);
}


QuarkCallRegInstr::QuarkCallRegInstr(uint32_t a, uint32_t retVal, uint32_t retValHigh)
{
	SetOperation(0x1f06);
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	EnableFlag(SYMFLAG_CALL);
	AddReadRegisterOperand(a);
	if (retVal != SYMREG_NONE)
		AddWriteRegisterOperand(retVal);
	if (retValHigh != SYMREG_NONE)
		AddWriteRegisterOperand(retValHigh);
}


QuarkSyscallImmInstr::QuarkSyscallImmInstr(int32_t immed, const vector<uint32_t>& writes, const vector<uint32_t>& reads)
{
	SetOperation(0x2c);
	AddImmediateOperand(immed);
	for (vector<uint32_t>::const_iterator i = writes.begin(); i != writes.end(); i++)
		AddWriteRegisterOperand(*i);
	for (vector<uint32_t>::const_iterator i = reads.begin(); i != reads.end(); i++)
		AddReadRegisterOperand(*i);
}


QuarkSyscallRegInstr::QuarkSyscallRegInstr(uint32_t a, const vector<uint32_t>& writes, const vector<uint32_t>& reads)
{
	SetOperation(0x1f10);
	AddReadRegisterOperand(a);
	for (vector<uint32_t>::const_iterator i = writes.begin(); i != writes.end(); i++)
		AddWriteRegisterOperand(*i);
	for (vector<uint32_t>::const_iterator i = reads.begin(); i != reads.end(); i++)
		AddReadRegisterOperand(*i);
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


QuarkAddStackInstr::QuarkAddStackInstr(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch)
{
	SetOperation(0x18);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddStackVarOperand(var, offset);
	AddTemporaryRegisterOperand(scratch);
}


QuarkAddGlobalInstr::QuarkAddGlobalInstr(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch)
{
	SetOperation(0x18);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddGlobalVarOperand(offset);
	AddTemporaryRegisterOperand(scratch);
}


QuarkAddBlockInstr::QuarkAddBlockInstr(uint32_t a, uint32_t b, Function* func, ILBlock* block, uint32_t scratch)
{
	SetOperation(0x18);
	AddWriteRegisterOperand(a);
	AddReadRegisterOperand(b);
	AddBlockOperand(func, block);
	AddTemporaryRegisterOperand(scratch);
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


QuarkSymReturnInstr::QuarkSymReturnInstr(uint32_t retVal, uint32_t retValHigh)
{
	EnableFlag(SYMFLAG_CONTROL_FLOW);
	if (retVal != SYMREG_NONE)
		AddReadRegisterOperand(retVal);
	if (retValHigh != SYMREG_NONE)
		AddReadRegisterOperand(retValHigh);
}


QuarkSaveCalleeSavedRegsInstr::QuarkSaveCalleeSavedRegsInstr()
{
}


QuarkRestoreCalleeSavedRegsInstr::QuarkRestoreCalleeSavedRegsInstr()
{
}


bool Quark1OpInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_INSTR(m_operation >> 8, m_operands[0].reg & 31, m_operation & 31, 0, 0));
	return true;
}


bool Quark2OpInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	if (m_operands[1].type == SYMOPERAND_REG)
		out->WriteUInt32(__QUARK_INSTR(m_operation >> 8, m_operands[0].reg & 31, m_operation & 31, m_operands[1].reg, 0));
	else
		out->WriteUInt32(__QUARK_IMM11(m_operation >> 8, m_operands[0].reg & 31, m_operation & 31, (uint32_t)m_operands[1].immed));
	return true;
}


bool Quark3OpInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	if (m_operands[2].type == SYMOPERAND_REG)
	{
		if (m_operands.size() > 3)
		{
			out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, m_operands[2].reg & 31,
				(uint32_t)m_operands[3].immed & 31));
		}
		else
		{
			out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, m_operands[2].reg & 31, 0));
		}
	}
	else
	{
		out->WriteUInt32(__QUARK_IMM11(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, (uint32_t)m_operands[2].immed));
	}
	return true;
}


bool Quark4OpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[1].reg & 31, m_operands[2].reg & 31,
		m_operands[3].reg & 31, m_operands[0].reg & 31));
	return true;
}


bool QuarkStackInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	int32_t offset = (int32_t)(func->GetStackVars()[m_operands[2].reg] + m_operands[2].immed);
	if ((offset >= -0x400) && (offset <= 0x3ff))
		out->WriteUInt32(__QUARK_IMM11(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, offset));
	else if ((offset >= -0x10000) && (offset <= 0xffff))
	{
		out->WriteUInt32(QUARK_EMIT_2(ldi, m_operands[3].reg & 31, offset));
		out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, m_operands[3].reg & 31, 0));
	}
	else
	{
		out->WriteUInt32(QUARK_EMIT_2(ldi, m_operands[3].reg & 31, offset));
		out->WriteUInt32(QUARK_EMIT_2(ldih, m_operands[3].reg & 31, offset >> 16));
		out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, m_operands[3].reg & 31, 0));
	}
	return true;
}


bool QuarkGlobalInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_IMM11(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, 0));

	Relocation reloc;
	reloc.type = DATA_RELOC_RELATIVE_32_FIELD;
	reloc.bitOffset = 0;
	reloc.bitSize = 11;
	reloc.bitShift = 0;
	reloc.offset = out->len - 4;
	reloc.extra = m_operands[3].reg & 31;
	reloc.instruction = reloc.offset;
	reloc.dataOffset = m_operands[2].immed;
	reloc.overflow = RelativeLoadOverflowHandler;
	out->relocs.push_back(reloc);
	return true;
}


bool QuarkAddBlockInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_IMM11(m_operation, m_operands[0].reg & 31, m_operands[1].reg & 31, 0));

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
	reloc.bitOffset = 0;
	reloc.bitSize = 11;
	reloc.bitShift = 0;
	reloc.offset = out->len - 4;
	reloc.extra = m_operands[3].reg & 31;
	reloc.instruction = reloc.offset;
	reloc.target = m_operands[2].block;
	reloc.overflow = RelativeLoadOverflowHandler;
	out->relocs.push_back(reloc);
	return true;
}


bool QuarkLoadImmInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_IMM17(m_operation, m_operands[0].reg & 31, (uint32_t)m_operands[1].immed));
	return true;
}


bool QuarkBranchInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_IMM22(m_operation, 0));

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
	reloc.overflow = NULL;
	reloc.instruction = out->len - 4;
	reloc.offset = out->len - 4;
	reloc.target = m_operands[0].block;
	reloc.bitOffset = 0;
	reloc.bitSize = 22;
	reloc.bitShift = 2;
	out->relocs.push_back(reloc);
	return true;
}


bool QuarkCondJumpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	if (m_operands[1].immed)
		out->WriteUInt32(QUARK_EMIT_COND_1(jmp, QUARK_IF_TRUE((uint32_t)m_operands[0].immed), 0));
	else
		out->WriteUInt32(QUARK_EMIT_COND_1(jmp, QUARK_IF_FALSE((uint32_t)m_operands[0].immed), 0));

	Relocation reloc;
	reloc.type = CODE_RELOC_RELATIVE_32_FIELD;
	reloc.overflow = NULL;
	reloc.instruction = out->len - 4;
	reloc.offset = out->len - 4;
	reloc.target = m_operands[2].block;
	reloc.bitOffset = 0;
	reloc.bitSize = 22;
	reloc.bitShift = 2;
	out->relocs.push_back(reloc);
	return true;
}


bool QuarkSyscallImmInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(QUARK_EMIT_1I(syscall, (uint32_t)m_operands[0].immed));
	return true;
}


bool QuarkCmpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	if (m_operands[3].type == SYMOPERAND_REG)
	{
		if (m_operands.size() > 4)
		{
			out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[2].reg & 31, QUARK_COND((uint32_t)m_operands[0].immed & 7,
				(uint32_t)m_operands[1].immed & 3), m_operands[3].reg & 31, (uint32_t)m_operands[4].immed & 31));
		}
		else
		{
			out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[2].reg & 31, QUARK_COND((uint32_t)m_operands[0].immed & 7,
				(uint32_t)m_operands[1].immed & 3), m_operands[3].reg & 31, 0));
		}
	}
	else
	{
		out->WriteUInt32(__QUARK_IMM11(m_operation, m_operands[2].reg & 31, QUARK_COND((uint32_t)m_operands[0].immed & 7,
			(uint32_t)m_operands[1].immed & 3), (uint32_t)m_operands[3].immed));
	}
	return true;
}


bool QuarkCondCmpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	uint32_t cond;
	if (m_operands[1].immed)
		cond = QUARK_IF_TRUE((uint32_t)m_operands[0].immed);
	else
		cond = QUARK_IF_FALSE((uint32_t)m_operands[0].immed);

	if (m_operands[5].type == SYMOPERAND_REG)
	{
		if (m_operands.size() > 6)
		{
			out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[4].reg & 31, QUARK_COND((uint32_t)m_operands[2].immed & 7,
				(uint32_t)m_operands[3].immed & 3), m_operands[5].reg & 31, (uint32_t)m_operands[6].immed & 31) | (cond << 28));
		}
		else
		{
			out->WriteUInt32(__QUARK_INSTR(m_operation, m_operands[4].reg & 31, QUARK_COND((uint32_t)m_operands[2].immed & 7,
				(uint32_t)m_operands[3].immed & 3), m_operands[5].reg & 31, 0) | (cond << 28));
		}
	}
	else
	{
		out->WriteUInt32(__QUARK_IMM11(m_operation, m_operands[4].reg & 31, QUARK_COND((uint32_t)m_operands[2].immed & 7,
			(uint32_t)m_operands[3].immed & 3), (uint32_t)m_operands[5].immed) | (cond << 28));
	}
	return true;
}


bool QuarkCondBit1OpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_INSTR(m_operation >> 8, (uint32_t)m_operands[0].immed & 3, m_operation & 31, 0, 0));
	return true;
}


bool QuarkCondBit2OpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_INSTR(m_operation >> 8, (uint32_t)m_operands[0].immed & 3, m_operation & 31,
		(uint32_t)m_operands[1].immed & 31, 0));
	return true;
}


bool QuarkCondBit3OpInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(__QUARK_INSTR(m_operation >> 8, (uint32_t)m_operands[0].immed & 3, m_operation & 31,
		(uint32_t)m_operands[1].immed & 31, (uint32_t)m_operands[2].immed & 31));
	return true;
}


bool QuarkBreakpointInstr::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	out->WriteUInt32(QUARK_EMIT(bp));
	return true;
}


bool QuarkPseudoInstrBase::EmitInstruction(SymInstrFunction* func, OutputBlock* out)
{
	return false;
}


QuarkSymInstrFunction::QuarkSymInstrFunction(const Settings& settings): SymInstrFunction(settings)
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
	case 0x1f0a: return "zxb";
	case 0x1f0b: return "zxh";
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


bool QuarkMovInstr::UpdateInstruction(SymInstrFunction* func, const Settings& settings, vector<SymInstr*>& replacement)
{
	// Eliminate mov instructions that have the same source and destination
	if ((m_operands[0].type == SYMOPERAND_REG) && (m_operands[1].type == SYMOPERAND_REG) &&
		(m_operands[0].reg == m_operands[1].reg))
		return true;
	return false;
}


bool QuarkSymReturnInstr::UpdateInstruction(SymInstrFunction* func, const Settings& settings, vector<SymInstr*>& replacement)
{
	// This pseudo-instruction is only present for data flow analysis, replace with nothing
	return true;
}


bool QuarkSaveCalleeSavedRegsInstr::UpdateInstruction(SymInstrFunction* func, const Settings& settings,
	vector<SymInstr*>& replacement)
{
	vector<uint32_t> clobbered = func->GetClobberedCalleeSavedRegisters();
	uint32_t min = 29;
	for (vector<uint32_t>::iterator i = clobbered.begin(); i != clobbered.end(); i++)
	{
		uint32_t reg = (*i) & 31;
		if (reg < min)
			min = reg;
	}

	// TODO: Support non-default stack pointer
	if (settings.stackGrowsUp)
	{
		for (vector<uint32_t>::iterator i = clobbered.begin(); i != clobbered.end(); i++)
			replacement.push_back(QuarkStoreUpdate32(SYMREG_NATIVE_REG(*i), SYMREG_NATIVE_REG(0), 4));
	}
	else
	{
		replacement.push_back(QuarkStoreMultipleUpdate(SYMREG_NATIVE_REG(min), SYMREG_NATIVE_REG(0), (min - 31) * 4));
	}

	return true;
}


bool QuarkRestoreCalleeSavedRegsInstr::UpdateInstruction(SymInstrFunction* func, const Settings& settings,
	vector<SymInstr*>& replacement)
{
	vector<uint32_t> clobbered = func->GetClobberedCalleeSavedRegisters();
	uint32_t min = 29;
	for (vector<uint32_t>::iterator i = clobbered.begin(); i != clobbered.end(); i++)
	{
		uint32_t reg = (*i) & 31;
		if (reg < min)
			min = reg;
	}

	// TODO: Support non-default stack pointer
	if (settings.stackGrowsUp)
	{
		int32_t offset = 0;
		for (vector<uint32_t>::iterator i = clobbered.begin(); i != clobbered.end(); i++, offset += 4)
			replacement.push_back(QuarkLoad32(SYMREG_NATIVE_REG(*i), SYMREG_NATIVE_REG(0), (clobbered.size() * -4) + offset));
		replacement.push_back(QuarkSub(SYMREG_NATIVE_REG(0), SYMREG_NATIVE_REG(0), clobbered.size() * 4));
	}
	else
	{
		replacement.push_back(QuarkLoadMultipleUpdate(SYMREG_NATIVE_REG(min), SYMREG_NATIVE_REG(0), 0));
	}

	return true;
}


void Quark1OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	if ((m_operands.size() > 1) && (m_operands[1].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[1].Print(func);
	}
}


void Quark1OpReadRegInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
}


void Quark1OpWriteRegInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
}


void Quark2OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	if ((m_operands.size() > 2) && (m_operands[2].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[2].Print(func);
	}
}


void Quark2OpRegInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


void Quark3OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print(func);
	}
}


void Quark3OpExInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print(func);
	}
}


void QuarkFloat3OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
}


void Quark4OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", ");
	m_operands[3].Print(func);
}


void QuarkMovInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	if ((m_operands.size() > 2) && (m_operands[2].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[2].Print(func);
	}
}


void QuarkLoadInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print(func);
	}
}


void QuarkStackLoadInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkGlobalLoadInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkLoadUpdateInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print(func);
	}
}


void QuarkLoadImmInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


void QuarkLoadImmHighInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


void QuarkStoreInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print(func);
	}
}


void QuarkStackStoreInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkGlobalStoreInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkStoreUpdateInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	if ((m_operands.size() > 3) && (m_operands[3].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[3].Print(func);
	}
}


void QuarkBranchInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
}


void QuarkCondJumpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "jmp.%d%s ", (int)m_operands[0].immed, m_operands[1].immed ? "t" : "f");
	m_operands[2].Print(func);
}


void QuarkCallInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	if (m_operands.size() > 1)
	{
		fprintf(stderr, ", ret ");
		m_operands[1].Print(func);
	}
	if (m_operands.size() > 2)
	{
		fprintf(stderr, ", high ");
		m_operands[2].Print(func);
	}
}


void QuarkCallRegInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	if (m_operands.size() > 1)
	{
		fprintf(stderr, ", ret ");
		m_operands[1].Print(func);
	}
	if (m_operands.size() > 2)
	{
		fprintf(stderr, ", high ");
		m_operands[2].Print(func);
	}
}


void QuarkSyscallImmInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	for (vector<SymInstrOperand>::iterator i = m_operands.begin(); i != m_operands.end(); i++)
	{
		i->Print(func);
		if ((i + 1) != m_operands.end())
			fprintf(stderr, ", ");
	}
}


void QuarkSyscallRegInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	for (vector<SymInstrOperand>::iterator i = m_operands.begin(); i != m_operands.end(); i++)
	{
		i->Print(func);
		if ((i + 1) != m_operands.end())
			fprintf(stderr, ", ");
	}
}


void QuarkCmpInstr::Print(SymInstrFunction* func)
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

	m_operands[2].Print(func);
	fprintf(stderr, ", ");
	m_operands[3].Print(func);
	if ((m_operands.size() > 4) && (m_operands[4].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[4].Print(func);
	}
}


void QuarkCondCmpInstr::Print(SymInstrFunction* func)
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

	m_operands[4].Print(func);
	fprintf(stderr, ", ");
	m_operands[5].Print(func);
	if ((m_operands.size() > 6) && (m_operands[6].immed != 0))
	{
		fprintf(stderr, "<<");
		m_operands[6].Print(func);
	}
}


void QuarkXchgInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


void QuarkAddStackInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkAddGlobalInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkAddBlockInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
	fprintf(stderr, ", scratch ");
	m_operands[3].Print(func);
}


void QuarkCondBit1OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
}


void QuarkCondBit2OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
}


void QuarkCondBit3OpInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "%s ", GetOperationName());
	m_operands[0].Print(func);
	fprintf(stderr, ", ");
	m_operands[1].Print(func);
	fprintf(stderr, ", ");
	m_operands[2].Print(func);
}


void QuarkBreakpointInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "bp");
}


void QuarkSymReturnInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "retval ");

	if (m_operands.size() == 0)
		fprintf(stderr, "none");
	else
		m_operands[0].Print(func);

	if (m_operands.size() > 1)
	{
		fprintf(stderr, ", ");
		m_operands[1].Print(func);
	}
}


void QuarkSaveCalleeSavedRegsInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "saveregs");
}


void QuarkRestoreCalleeSavedRegsInstr::Print(SymInstrFunction* func)
{
	fprintf(stderr, "restoreregs");
}


vector<uint32_t> QuarkSymInstrFunction::GetCallerSavedRegisters()
{
	vector<uint32_t> result;
	// TODO: Support non-default special registers
	for (uint32_t i = 1; i < 16; i++)
		result.push_back(SYMREG_NATIVE_REG(i));
	return result;
}


vector<uint32_t> QuarkSymInstrFunction::GetCalleeSavedRegisters()
{
	vector<uint32_t> result;
	// TODO: Support non-default special registers
	for (uint32_t i = 28; i >= 16; i--)
		result.push_back(SYMREG_NATIVE_REG(i));
	return result;
}


set<uint32_t> QuarkSymInstrFunction::GetRegisterClassInterferences(uint32_t cls)
{
	set<uint32_t> result;
	// TODO: Floating point
	// TODO: Support non-default special registers

#define ALL_EXCEPT(r) \
	for (uint32_t _i = 0; _i < 32; _i++) \
	{ \
		if (_i != (r)) \
			result.insert(SYMREG_NATIVE_REG(_i)); \
	}

	switch (cls)
	{
	case QUARKREGCLASS_INTEGER_RETURN_VALUE:
		ALL_EXCEPT(1);
		break;
	case QUARKREGCLASS_INTEGER_RETURN_VALUE_HIGH:
		ALL_EXCEPT(2);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_0:
		ALL_EXCEPT(1);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_1:
		ALL_EXCEPT(2);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_2:
		ALL_EXCEPT(3);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_3:
		ALL_EXCEPT(4);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_4:
		ALL_EXCEPT(5);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_5:
		ALL_EXCEPT(6);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_6:
		ALL_EXCEPT(7);
		break;
	case QUARKREGCLASS_INTEGER_PARAM_7:
		ALL_EXCEPT(8);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_0:
		ALL_EXCEPT(1);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_1:
		ALL_EXCEPT(2);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_2:
		ALL_EXCEPT(3);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_3:
		ALL_EXCEPT(4);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_4:
		ALL_EXCEPT(5);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_5:
		ALL_EXCEPT(6);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_6:
		ALL_EXCEPT(7);
		break;
	case QUARKREGCLASS_SYSCALL_PARAM_7:
		ALL_EXCEPT(8);
		break;
	case QUARKREGCLASS_SYSCALL_RESULT_1:
		ALL_EXCEPT(1);
		break;
	case QUARKREGCLASS_SYSCALL_RESULT_2:
		ALL_EXCEPT(2);
		break;
	default:
		break;
	}
	return result;
}


uint32_t QuarkSymInstrFunction::GetSpecialRegisterAssignment(uint32_t reg)
{
	// TODO: Support non-default special registers
	switch (reg)
	{
	case SYMREG_SP:
		return SYMREG_NATIVE_REG(0);
	case SYMREG_BP:
		return SYMREG_NATIVE_REG(29);
	case SYMREG_LR:
		return SYMREG_NATIVE_REG(30);
	case SYMREG_IP:
		return SYMREG_NATIVE_REG(31);
	default:
		return SYMREG_NONE;
	}
}


void QuarkSymInstrFunction::AdjustStackFrame()
{
	// Analyze callee saved registers
	uint32_t min = 29;
	for (vector<uint32_t>::iterator i = m_clobberedCalleeSavedRegs.begin(); i != m_clobberedCalleeSavedRegs.end(); i++)
	{
		uint32_t reg = (*i) & 31;
		if (reg < min)
			min = reg;
	}

	// Adjust parameter locations to account for callee saved registers
	int64_t adjust;
	if (m_settings.stackGrowsUp)
		adjust = m_clobberedCalleeSavedRegs.size() * 4;
	else
		adjust = (31 - min) * 4;

	for (vector<int64_t>::iterator i = m_stackVarOffsets.begin(); i != m_stackVarOffsets.end(); i++)
	{
		if (m_settings.stackGrowsUp)
		{
			if (*i <= 0)
				*i -= adjust;
		}
		else
		{
			if (*i >= 0)
				*i += adjust;
		}
	}
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
	case QUARKREGCLASS_SYSCALL_RESULT_1:
		fprintf(stderr, "sysresult1");
		break;
	case QUARKREGCLASS_SYSCALL_RESULT_2:
		fprintf(stderr, "sysresult2");
		break;
	default:
		fprintf(stderr, "invalid");
		break;
	}
}


void QuarkSymInstrFunction::PrintRegister(uint32_t reg)
{
	if ((reg >= SYMREG_NATIVE_REG(0)) && (reg < SYMREG_NATIVE_REG(32)))
		fprintf(stderr, "r%d", reg & 31);
	else
		SymInstrFunction::PrintRegister(reg);
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
SymInstr* QuarkLoadStack8(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x00, a, b, var, o, scratch); }
SymInstr* QuarkLoadStack16(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x01, a, b, var, o, scratch); }
SymInstr* QuarkLoadStack32(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x02, a, b, var, o, scratch); }
SymInstr* QuarkLoadStackFS(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x30, a, b, var, o, scratch); }
SymInstr* QuarkLoadStackFD(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x31, a, b, var, o, scratch); }
SymInstr* QuarkLoadStackSX8(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x10, a, b, var, o, scratch); }
SymInstr* QuarkLoadStackSX16(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackLoadInstr(0x11, a, b, var, o, scratch); }
SymInstr* QuarkLoadGlobal8(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x00, a, b, offset, scratch); }
SymInstr* QuarkLoadGlobal16(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x01, a, b, offset, scratch); }
SymInstr* QuarkLoadGlobal32(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x02, a, b, offset, scratch); }
SymInstr* QuarkLoadGlobalFS(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x30, a, b, offset, scratch); }
SymInstr* QuarkLoadGlobalFD(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x31, a, b, offset, scratch); }
SymInstr* QuarkLoadGlobalSX8(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x10, a, b, offset, scratch); }
SymInstr* QuarkLoadGlobalSX16(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalLoadInstr(0x11, a, b, offset, scratch); }
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
SymInstr* QuarkStoreStack8(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackStoreInstr(0x04, a, b, var, o, scratch); }
SymInstr* QuarkStoreStack16(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackStoreInstr(0x05, a, b, var, o, scratch); }
SymInstr* QuarkStoreStack32(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackStoreInstr(0x06, a, b, var, o, scratch); }
SymInstr* QuarkStoreStackFS(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackStoreInstr(0x32, a, b, var, o, scratch); }
SymInstr* QuarkStoreStackFD(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkStackStoreInstr(0x33, a, b, var, o, scratch); }
SymInstr* QuarkStoreGlobal8(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalStoreInstr(0x04, a, b, offset, scratch); }
SymInstr* QuarkStoreGlobal16(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalStoreInstr(0x05, a, b, offset, scratch); }
SymInstr* QuarkStoreGlobal32(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalStoreInstr(0x06, a, b, offset, scratch); }
SymInstr* QuarkStoreGlobalFS(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalStoreInstr(0x32, a, b, offset, scratch); }
SymInstr* QuarkStoreGlobalFD(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkGlobalStoreInstr(0x33, a, b, offset, scratch); }
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

SymInstr* QuarkLoadMultipleUpdate(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkLoadUpdateInstr(0x0b, a, b, c, s); }
SymInstr* QuarkLoadMultipleUpdate(uint32_t a, uint32_t b, int32_t immed) { return new QuarkLoadUpdateInstr(0x0b, a, b, immed); }
SymInstr* QuarkStoreMultipleUpdate(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new QuarkStoreUpdateInstr(0x0f, a, b, c, s); }
SymInstr* QuarkStoreMultipleUpdate(uint32_t a, uint32_t b, int32_t immed) { return new QuarkStoreUpdateInstr(0x0f, a, b, immed); }

SymInstr* QuarkLoadImm(uint32_t a, int32_t immed) { return new QuarkLoadImmInstr(a, immed); }
SymInstr* QuarkLoadImmHigh(uint32_t a, int32_t immed) { return new QuarkLoadImmHighInstr(a, immed); }

SymInstr* QuarkJump(Function* func, ILBlock* block) { return new QuarkBranchInstr(func, block); }
SymInstr* QuarkCondJump(uint32_t b, uint32_t value, Function* func, ILBlock* block) { return new QuarkCondJumpInstr(b, value, func, block); }
SymInstr* QuarkCall(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh) { return new QuarkCallInstr(func, block, retVal, retValHigh); }
SymInstr* QuarkCall(uint32_t reg, uint32_t retVal, uint32_t retValHigh) { return new QuarkCallRegInstr(reg, retVal, retValHigh); }
SymInstr* QuarkSyscallReg(uint32_t a, const vector<uint32_t>& writes, const vector<uint32_t>& reads) { return new QuarkSyscallRegInstr(a, writes, reads); }
SymInstr* QuarkSyscallImmed(int32_t immed, const vector<uint32_t>& writes, const vector<uint32_t>& reads) { return new QuarkSyscallImmInstr(immed, writes, reads); }

SymInstr* QuarkAdd(uint32_t a, uint32_t b, uint32_t c, uint32_t s) { return new Quark3OpInstr(0x18, a, b, c, s); }
SymInstr* QuarkAdd(uint32_t a, uint32_t b, int32_t immed) { return new Quark3OpInstr(0x18, a, b, immed); }
SymInstr* QuarkAddStack(uint32_t a, uint32_t b, uint32_t var, int64_t o, uint32_t scratch) { return new QuarkAddStackInstr(a, b, var, o, scratch); }
SymInstr* QuarkAddGlobal(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch) { return new QuarkAddGlobalInstr(a, b, offset, scratch); }
SymInstr* QuarkAddBlock(uint32_t a, uint32_t b, Function* func, ILBlock* block, uint32_t scratch) { return new QuarkAddBlockInstr(a, b, func, block, scratch); }
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

SymInstr* QuarkMov(uint32_t a, uint32_t b, uint32_t s) { return new QuarkMovInstr(a, b, s); }
SymInstr* QuarkMov(uint32_t a, int32_t immed) { return new QuarkMovInstr(a, immed); }
SymInstr* QuarkXchg(uint32_t a, uint32_t b) { return new QuarkXchgInstr(a, b); }
SymInstr* QuarkSxb(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f02, a, b); }
SymInstr* QuarkSxh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f03, a, b); }
SymInstr* QuarkSwaph(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f04, a, b); }
SymInstr* QuarkSwapw(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f05, a, b); }
SymInstr* QuarkNeg(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f08, a, b); }
SymInstr* QuarkNot(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f09, a, b); }
SymInstr* QuarkZxb(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f0a, a, b); }
SymInstr* QuarkZxh(uint32_t a, uint32_t b) { return new Quark2OpRegInstr(0x1f0b, a, b); }

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

SymInstr* QuarkSymReturn(uint32_t retVal, uint32_t retValHigh) { return new QuarkSymReturnInstr(retVal, retValHigh); }
SymInstr* QuarkSaveCalleeSavedRegs() { return new QuarkSaveCalleeSavedRegsInstr(); }
SymInstr* QuarkRestoreCalleeSavedRegs() { return new QuarkRestoreCalleeSavedRegsInstr(); }

