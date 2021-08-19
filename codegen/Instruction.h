// Copyright (c) 2014 Rusty Wagner
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

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "CodeBlock.h"
#include <inttypes.h>
#include <string>

#define SYMFLAG_WRITES_FLAGS   1
#define SYMFLAG_USES_FLAGS     2
#define SYMFLAG_MEMORY_BARRIER 4
#define SYMFLAG_CONTROL_FLOW   8
#define SYMFLAG_CALL           0x10
#define SYMFLAG_COPY           0x20
#define SYMFLAG_STACK          0x40


enum InstructionTokenType
{
	INSTRTOKEN_TEXT,
	INSTRTOKEN_OPERAND
};

enum OperandType
{
	OPERAND_REG,
	OPERAND_REG_LIST,
	OPERAND_IMMED,
	OPERAND_STACK_VAR,
	OPERAND_GLOBAL_VAR,
	OPERAND_FUNCTION,
	OPERAND_TEMP
};

enum OperandAccessType
{
	ACCESS_DEFAULT,
	ACCESS_READ,
	ACCESS_WRITE,
	ACCESS_READ_WRITE
};

struct InstructionToken
{
	std::string fileName;
	int line;
	InstructionTokenType type;
	std::string name;
	OperandType operand;
	OperandAccessType access;

	static InstructionToken* CreateTextToken(
	    const std::string& fileName, int line, const std::string& name);
	static InstructionToken* CreateOperandToken(const std::string& fileName, int line,
	    const std::string& name, OperandType operand, OperandAccessType access);
};

class Instruction : public RefCountObject
{
	std::string m_name;
	uint32_t m_flags;
	std::vector<InstructionToken> m_tokens;
	Ref<CodeBlock> m_code;
	Ref<CodeBlock> m_update;

 public:
	Instruction(const std::string& name, uint32_t flags, const std::vector<InstructionToken>& tokens,
	    CodeBlock* code, CodeBlock* update);

	const std::string& GetName() const { return m_name; }
	uint32_t GetFlags() const { return m_flags; }
	const std::vector<InstructionToken>& GetTokens() const { return m_tokens; }
	CodeBlock* GetCode() const { return m_code; }
	CodeBlock* GetUpdate() const { return m_update; }
};


#endif
