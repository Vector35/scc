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

#include "Instruction.h"

using namespace std;


InstructionToken* InstructionToken::CreateTextToken(const string& fileName, int line, const string& name)
{
	InstructionToken* token = new InstructionToken;
	token->fileName = fileName;
	token->line = line;
	token->type = INSTRTOKEN_TEXT;
	token->name = name;
	return token;
}


InstructionToken* InstructionToken::CreateOperandToken(const string& fileName, int line, const string& name,
	OperandType operand, OperandAccessType access)
{
	InstructionToken* token = new InstructionToken;
	token->fileName = fileName;
	token->line = line;
	token->type = INSTRTOKEN_OPERAND;
	token->name = name;
	token->operand = operand;
	token->access = access;
	return token;
}


Instruction::Instruction(const string& name, uint32_t flags, const vector<InstructionToken>& tokens,
	CodeBlock* code, CodeBlock* update): m_name(name), m_flags(flags), m_tokens(tokens), m_code(code), m_update(update)
{
}

