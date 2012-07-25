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

#ifndef __QUARKSYMINSTR_H__
#define __QUARKSYMINSTR_H__

#include "SymInstr.h"


enum QuarkRegisterClass
{
	QUARKREGCLASS_INTEGER,
	QUARKREGCLASS_FLOAT,
	QUARKREGCLASS_INTEGER_RETURN_VALUE,
	QUARKREGCLASS_INTEGER_RETURN_VALUE_HIGH,
	QUARKREGCLASS_INTEGER_PARAM_0,
	QUARKREGCLASS_INTEGER_PARAM_1,
	QUARKREGCLASS_INTEGER_PARAM_2,
	QUARKREGCLASS_INTEGER_PARAM_3,
	QUARKREGCLASS_INTEGER_PARAM_4,
	QUARKREGCLASS_INTEGER_PARAM_5,
	QUARKREGCLASS_INTEGER_PARAM_6,
	QUARKREGCLASS_INTEGER_PARAM_7,
	QUARKREGCLASS_FLOAT_RETURN_VALUE,
	QUARKREGCLASS_FLOAT_PARAM_0,
	QUARKREGCLASS_FLOAT_PARAM_1,
	QUARKREGCLASS_FLOAT_PARAM_2,
	QUARKREGCLASS_FLOAT_PARAM_3,
	QUARKREGCLASS_FLOAT_PARAM_4,
	QUARKREGCLASS_FLOAT_PARAM_5,
	QUARKREGCLASS_FLOAT_PARAM_6,
	QUARKREGCLASS_FLOAT_PARAM_7,
	QUARKREGCLASS_SYSCALL_PARAM_0,
	QUARKREGCLASS_SYSCALL_PARAM_1,
	QUARKREGCLASS_SYSCALL_PARAM_2,
	QUARKREGCLASS_SYSCALL_PARAM_3,
	QUARKREGCLASS_SYSCALL_PARAM_4,
	QUARKREGCLASS_SYSCALL_PARAM_5,
	QUARKREGCLASS_SYSCALL_PARAM_6,
	QUARKREGCLASS_SYSCALL_PARAM_7,
	QUARKREGCLASS_SYSCALL_RESULT_1,
	QUARKREGCLASS_SYSCALL_RESULT_2
};

#define QUARKREGCLASS_INTEGER_PARAM(n) ((QuarkRegisterClass)(QUARKREGCLASS_INTEGER_PARAM_0 + (n)))
#define QUARKREGCLASS_FLOAT_PARAM(n)   ((QuarkRegisterClass)(QUARKREGCLASS_FLOAT_PARAM_0 + (n)))
#define QUARKREGCLASS_SYSCALL_PARAM(n) ((QuarkRegisterClass)(QUARKREGCLASS_SYSCALL_PARAM_0 + (n)))


class QuarkSymInstr: public SymInstr
{
protected:
	static void RelativeLoadOverflowHandler(OutputBlock* out, Relocation& reloc);

public:
	QuarkSymInstr();
	const char* GetOperationName() const;
};

class Quark1OpInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class Quark1OpInstr: public Quark1OpInstrBase
{
public:
	Quark1OpInstr(uint32_t op, uint32_t a, uint32_t shift);
	Quark1OpInstr(uint32_t op, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class Quark1OpReadRegInstr: public Quark1OpInstrBase
{
public:
	Quark1OpReadRegInstr(uint32_t op, uint32_t a);
	virtual void Print(SymInstrFunction* func);
};

class Quark1OpWriteRegInstr: public Quark1OpInstrBase
{
public:
	Quark1OpWriteRegInstr(uint32_t op, uint32_t a);
	virtual void Print(SymInstrFunction* func);
};

class Quark2OpInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class Quark2OpInstr: public Quark2OpInstrBase
{
public:
	Quark2OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t shift);
	Quark2OpInstr(uint32_t op, uint32_t a, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class Quark2OpRegInstr: public Quark2OpInstrBase
{
public:
	Quark2OpRegInstr(uint32_t op, uint32_t a, uint32_t b);
	virtual void Print(SymInstrFunction* func);
};

class Quark3OpInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class Quark3OpInstr: public Quark3OpInstrBase
{
public:
	Quark3OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift);
	Quark3OpInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class Quark3OpExInstr: public Quark3OpInstrBase
{
public:
	Quark3OpExInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift);
	Quark3OpExInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkFloat3OpInstr: public Quark3OpInstrBase
{
public:
	QuarkFloat3OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c);
	virtual void Print(SymInstrFunction* func);
};

class Quark4OpInstr: public QuarkSymInstr
{
public:
	Quark4OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkMovInstr: public Quark2OpInstrBase
{
public:
	QuarkMovInstr(uint32_t a, uint32_t b, uint32_t shift);
	QuarkMovInstr(uint32_t a, int32_t immed);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
	virtual void Print(SymInstrFunction* func);
};

class QuarkLoadInstr: public Quark3OpInstrBase
{
public:
	QuarkLoadInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift);
	QuarkLoadInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkStackInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class QuarkStackLoadInstr: public QuarkStackInstrBase
{
public:
	QuarkStackLoadInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
	virtual void Print(SymInstrFunction* func);
};

class QuarkGlobalInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class QuarkGlobalLoadInstr: public QuarkGlobalInstrBase
{
public:
	QuarkGlobalLoadInstr(uint32_t op, uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
	virtual void Print(SymInstrFunction* func);
};

class QuarkLoadUpdateInstr: public Quark3OpInstrBase
{
public:
	QuarkLoadUpdateInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift);
	QuarkLoadUpdateInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkLoadImmInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class QuarkLoadImmInstr: public QuarkLoadImmInstrBase
{
public:
	QuarkLoadImmInstr(uint32_t a, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkLoadImmHighInstr: public QuarkLoadImmInstrBase
{
public:
	QuarkLoadImmHighInstr(uint32_t a, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkStoreInstr: public Quark3OpInstrBase
{
public:
	QuarkStoreInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift);
	QuarkStoreInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkStackStoreInstr: public QuarkStackInstrBase
{
public:
	QuarkStackStoreInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
	virtual void Print(SymInstrFunction* func);
};

class QuarkGlobalStoreInstr: public QuarkGlobalInstrBase
{
public:
	QuarkGlobalStoreInstr(uint32_t op, uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
	virtual void Print(SymInstrFunction* func);
};

class QuarkStoreUpdateInstr: public Quark3OpInstrBase
{
public:
	QuarkStoreUpdateInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c, uint32_t shift);
	QuarkStoreUpdateInstr(uint32_t op, uint32_t a, uint32_t b, int32_t immed);
	virtual void Print(SymInstrFunction* func);
};

class QuarkBranchInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class QuarkBranchInstr: public QuarkBranchInstrBase
{
public:
	QuarkBranchInstr(Function* func, ILBlock* block);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCondJumpInstr: public QuarkSymInstr
{
public:
	QuarkCondJumpInstr(uint32_t b, uint32_t val, Function* func, ILBlock* block);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCallInstr: public QuarkBranchInstrBase
{
public:
	QuarkCallInstr(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCallRegInstr: public Quark1OpInstrBase
{
public:
	QuarkCallRegInstr(uint32_t a, uint32_t retVal, uint32_t retValHigh);
	virtual void Print(SymInstrFunction* func);
};

class QuarkSyscallImmInstr: public QuarkSymInstr
{
public:
	QuarkSyscallImmInstr(int32_t imm, const std::vector<uint32_t>& writes, const std::vector<uint32_t>& reads);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkSyscallRegInstr: public Quark1OpInstrBase
{
public:
	QuarkSyscallRegInstr(uint32_t a, const std::vector<uint32_t>& writes, const std::vector<uint32_t>& reads);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCmpInstr: public QuarkSymInstr
{
public:
	QuarkCmpInstr(uint32_t op, uint32_t b, uint32_t cmp, uint32_t a, uint32_t c, uint32_t shift);
	QuarkCmpInstr(uint32_t op, uint32_t b, uint32_t cmp, uint32_t a, int32_t immed);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCondCmpInstr: public QuarkSymInstr
{
public:
	QuarkCondCmpInstr(uint32_t cc, uint32_t val, uint32_t op, uint32_t b, uint32_t cmp, uint32_t a, uint32_t c, uint32_t shift);
	QuarkCondCmpInstr(uint32_t cc, uint32_t val, uint32_t op, uint32_t b, uint32_t cmp, uint32_t a, int32_t immed);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkXchgInstr: public Quark2OpInstrBase
{
public:
	QuarkXchgInstr(uint32_t a, uint32_t b);
	virtual void Print(SymInstrFunction* func);
};

class QuarkAddStackInstr: public QuarkStackInstrBase
{
public:
	QuarkAddStackInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
	virtual void Print(SymInstrFunction* func);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
};

class QuarkAddGlobalInstr: public QuarkGlobalInstrBase
{
public:
	QuarkAddGlobalInstr(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
	virtual void Print(SymInstrFunction* func);
};

class QuarkAddBlockInstr: public QuarkSymInstr
{
public:
	QuarkAddBlockInstr(uint32_t a, uint32_t b, Function* func, ILBlock* block, uint32_t scratch);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCondBit1OpInstr: public QuarkSymInstr
{
public:
	QuarkCondBit1OpInstr(uint32_t op, uint32_t a);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCondBit2OpInstr: public QuarkSymInstr
{
public:
	QuarkCondBit2OpInstr(uint32_t op, uint32_t a, uint32_t b);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkCondBit3OpInstr: public QuarkSymInstr
{
public:
	QuarkCondBit3OpInstr(uint32_t op, uint32_t a, uint32_t b, uint32_t c);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkBreakpointInstr: public QuarkSymInstr
{
public:
	QuarkBreakpointInstr();
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};

class QuarkPseudoInstrBase: public QuarkSymInstr
{
public:
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
};

class QuarkSymReturnInstr: public QuarkPseudoInstrBase
{
public:
	QuarkSymReturnInstr(uint32_t retVal, uint32_t retValHigh);
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
	virtual void Print(SymInstrFunction* func);
};

class QuarkSaveCalleeSavedRegsInstr: public QuarkPseudoInstrBase
{
public:
	QuarkSaveCalleeSavedRegsInstr();
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
	virtual void Print(SymInstrFunction* func);
};

class QuarkRestoreCalleeSavedRegsInstr: public QuarkPseudoInstrBase
{
public:
	QuarkRestoreCalleeSavedRegsInstr();
	virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings, std::vector<SymInstr*>& replacement);
	virtual void Print(SymInstrFunction* func);
};

class QuarkAntiDisassemblyInstr: public QuarkSymInstr
{
public:
	QuarkAntiDisassemblyInstr(uint32_t reg);
	virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out);
	virtual void Print(SymInstrFunction* func);
};


class QuarkSymInstrFunction: public SymInstrFunction
{
public:
	QuarkSymInstrFunction(const Settings& settings, Function* func);
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


SymInstr* QuarkLoad8(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoad8(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoad16(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoad16(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoad32(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoad32(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadFS(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadFD(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadSX8(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadSX8(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadSX16(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadSX16(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadStack8(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadStack16(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadStack32(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadStackFS(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadStackFD(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadStackSX8(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadStackSX16(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobal8(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobal16(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobal32(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobalFS(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobalFD(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobalSX8(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadGlobalSX16(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkLoadUpdate8(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadUpdate8(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadUpdate16(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadUpdate16(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadUpdate32(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadUpdate32(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadUpdateFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadUpdateFS(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadUpdateFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadUpdateFD(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadSXUpdate8(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadSXUpdate8(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkLoadSXUpdate16(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadSXUpdate16(uint32_t a, uint32_t b, int32_t immed);

SymInstr* QuarkStore8(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStore8(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStore16(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStore16(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStore32(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStore32(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreFS(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreFD(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreStack8(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreStack16(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreStack32(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreStackFS(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreStackFD(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreGlobal8(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreGlobal16(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreGlobal32(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreGlobalFS(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreGlobalFD(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkStoreUpdate8(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreUpdate8(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreUpdate16(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreUpdate16(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreUpdate32(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreUpdate32(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreUpdateFS(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreUpdateFS(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreUpdateFD(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreUpdateFD(uint32_t a, uint32_t b, int32_t immed);

SymInstr* QuarkLoadMultipleUpdate(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkLoadMultipleUpdate(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkStoreMultipleUpdate(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkStoreMultipleUpdate(uint32_t a, uint32_t b, int32_t immed);

SymInstr* QuarkLoadImm(uint32_t a, int32_t immed);
SymInstr* QuarkLoadImmHigh(uint32_t a, int32_t immed);

SymInstr* QuarkJump(Function* func, ILBlock* block);
SymInstr* QuarkCondJump(uint32_t b, uint32_t value, Function* func, ILBlock* block);
SymInstr* QuarkCall(Function* func, ILBlock* block, uint32_t retVal, uint32_t retValHigh);
SymInstr* QuarkCall(uint32_t reg, uint32_t retVal, uint32_t retValHigh);
SymInstr* QuarkSyscallReg(uint32_t a, const std::vector<uint32_t>& writes, const std::vector<uint32_t>& reads);
SymInstr* QuarkSyscallImmed(int32_t immed, const std::vector<uint32_t>& writes, const std::vector<uint32_t>& reads);

SymInstr* QuarkAdd(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkAdd(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkAddStack(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkAddGlobal(uint32_t a, uint32_t b, int64_t offset, uint32_t scratch);
SymInstr* QuarkAddBlock(uint32_t a, uint32_t b, Function* func, ILBlock* block, uint32_t scratch);
SymInstr* QuarkSub(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkSub(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkSubStack(uint32_t a, uint32_t b, uint32_t var, int64_t offset, uint32_t scratch);
SymInstr* QuarkAddx(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkAddx(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkSubx(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkSubx(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkMulx(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
SymInstr* QuarkImulx(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
SymInstr* QuarkMul(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkMul(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkDiv(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkDiv(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkIdiv(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkIdiv(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkMod(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkMod(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkImod(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkImod(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkAnd(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkAnd(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkOr(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkOr(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkXor(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkXor(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkSar(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkSar(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkShl(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkShl(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkShr(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkShr(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkRol(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkRol(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkRor(uint32_t a, uint32_t b, uint32_t c, uint32_t s);
SymInstr* QuarkRor(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFadd(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFadd(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFsub(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFsub(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFmul(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFmul(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFdiv(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFdiv(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFmod(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFmod(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFpow(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFpow(uint32_t a, uint32_t b, int32_t immed);
SymInstr* QuarkFlog(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkFlog(uint32_t a, uint32_t b, int32_t immed);

SymInstr* QuarkCmp(uint32_t b, uint32_t op, uint32_t a, uint32_t c, uint32_t s);
SymInstr* QuarkCmp(uint32_t b, uint32_t op, uint32_t a, int32_t immed);
SymInstr* QuarkIcmp(uint32_t b, uint32_t op, uint32_t a, uint32_t c, uint32_t s);
SymInstr* QuarkIcmp(uint32_t b, uint32_t op, uint32_t a, int32_t immed);
SymInstr* QuarkFcmp(uint32_t b, uint32_t op, uint32_t a, uint32_t c, uint32_t s);
SymInstr* QuarkFcmp(uint32_t b, uint32_t op, uint32_t a, int32_t immed);
SymInstr* QuarkCondCmp(uint32_t cc, uint32_t val, uint32_t b, uint32_t op, uint32_t a, uint32_t c, uint32_t s);
SymInstr* QuarkCondCmp(uint32_t cc, uint32_t val, uint32_t b, uint32_t op, uint32_t a, int32_t immed);
SymInstr* QuarkCondIcmp(uint32_t cc, uint32_t val, uint32_t b, uint32_t op, uint32_t a, uint32_t c, uint32_t s);
SymInstr* QuarkCondIcmp(uint32_t cc, uint32_t val, uint32_t b, uint32_t op, uint32_t a, int32_t immed);

SymInstr* QuarkMov(uint32_t a, uint32_t b, uint32_t s);
SymInstr* QuarkMov(uint32_t a, int32_t immed);
SymInstr* QuarkXchg(uint32_t a, uint32_t b);
SymInstr* QuarkSxb(uint32_t a, uint32_t b);
SymInstr* QuarkSxh(uint32_t a, uint32_t b);
SymInstr* QuarkSwaph(uint32_t a, uint32_t b);
SymInstr* QuarkSwapw(uint32_t a, uint32_t b);
SymInstr* QuarkNeg(uint32_t a, uint32_t b);
SymInstr* QuarkNot(uint32_t a, uint32_t b);
SymInstr* QuarkZxb(uint32_t a, uint32_t b);
SymInstr* QuarkZxh(uint32_t a, uint32_t b);

SymInstr* QuarkLoadCR(uint32_t a);
SymInstr* QuarkStoreCR(uint32_t a);

SymInstr* QuarkSetCC(uint32_t a);
SymInstr* QuarkClrCC(uint32_t a);
SymInstr* QuarkNotCC(uint32_t a, uint32_t b);
SymInstr* QuarkAndCC(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkOrCC(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkXorCC(uint32_t a, uint32_t b, uint32_t c);
SymInstr* QuarkMovCC(uint32_t a, uint32_t b);

SymInstr* QuarkLoadFI(uint32_t a, uint32_t b, uint32_t s);
SymInstr* QuarkLoadFI(uint32_t a, int32_t immed);
SymInstr* QuarkStoreFI(uint32_t a, uint32_t b);
SymInstr* QuarkLoadPi(uint32_t a);
SymInstr* QuarkLoadE(uint32_t a);
SymInstr* QuarkEtoXReg(uint32_t a, uint32_t b);
SymInstr* QuarkEtoXImmed(uint32_t a, int32_t immed);
SymInstr* Quark2toXReg(uint32_t a, uint32_t b);
SymInstr* Quark2toXImmed(uint32_t a, int32_t immed);
SymInstr* Quark10toXReg(uint32_t a, uint32_t b);
SymInstr* Quark10toXImmed(uint32_t a, int32_t immed);
SymInstr* QuarkSqrt(uint32_t a, uint32_t b);
SymInstr* QuarkSqrt(uint32_t a, int32_t immed);
SymInstr* QuarkRecip(uint32_t a, uint32_t b);
SymInstr* QuarkRecip(uint32_t a, int32_t immed);
SymInstr* QuarkRecipSqrt(uint32_t a, uint32_t b);
SymInstr* QuarkRecipSqrt(uint32_t a, int32_t immed);
SymInstr* QuarkFneg(uint32_t a, uint32_t b);
SymInstr* QuarkFsin(uint32_t a, uint32_t b);
SymInstr* QuarkFcos(uint32_t a, uint32_t b);
SymInstr* QuarkFtan(uint32_t a, uint32_t b);
SymInstr* QuarkFloor(uint32_t a, uint32_t b);
SymInstr* QuarkFasin(uint32_t a, uint32_t b);
SymInstr* QuarkFacos(uint32_t a, uint32_t b);
SymInstr* QuarkFatan(uint32_t a, uint32_t b);
SymInstr* QuarkCeil(uint32_t a, uint32_t b);
SymInstr* QuarkFsinh(uint32_t a, uint32_t b);
SymInstr* QuarkFcosh(uint32_t a, uint32_t b);
SymInstr* QuarkFtanh(uint32_t a, uint32_t b);
SymInstr* QuarkRound(uint32_t a, uint32_t b);
SymInstr* QuarkFasinh(uint32_t a, uint32_t b);
SymInstr* QuarkFacosh(uint32_t a, uint32_t b);
SymInstr* QuarkFatanh(uint32_t a, uint32_t b);
SymInstr* QuarkFabs(uint32_t a, uint32_t b);
SymInstr* QuarkFln(uint32_t a, uint32_t b);
SymInstr* QuarkFln(uint32_t a, int32_t immed);
SymInstr* QuarkFlog2(uint32_t a, uint32_t b);
SymInstr* QuarkFlog2(uint32_t a, int32_t immed);
SymInstr* QuarkFlog10(uint32_t a, uint32_t b);
SymInstr* QuarkFlog10(uint32_t a, int32_t immed);
SymInstr* QuarkFmov(uint32_t a, uint32_t b);
SymInstr* QuarkFmov(uint32_t a, int32_t immed);

SymInstr* QuarkSymReturn(uint32_t retVal, uint32_t retValHigh);
SymInstr* QuarkSaveCalleeSavedRegs();
SymInstr* QuarkRestoreCalleeSavedRegs();
SymInstr* QuarkAntiDisassembly(uint32_t reg);


#endif

