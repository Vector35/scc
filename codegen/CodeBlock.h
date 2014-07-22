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

#ifndef __CODEBLOCK_H__
#define __CODEBLOCK_H__

#include <string>
#include <vector>
#include <map>
#include "RefCountObject.h"


enum CodeTokenType
{
	TOKEN_TEXT,
	TOKEN_VAR,
	TOKEN_VAR_LOW,
	TOKEN_VAR_HIGH,
	TOKEN_VAR_INT,
	TOKEN_VAR_BASE,
	TOKEN_VAR_OFFSET,
	TOKEN_VAR_TEMP,
	TOKEN_VAR_FUNCTION,
	TOKEN_VAR_BLOCK,
	TOKEN_INSTR_START,
	TOKEN_INSTR_END,
	TOKEN_INSTR_ENCODING,
	TOKEN_INSTR_ENCODING_VALUE
};

class CodeBlock;

struct CodeToken
{
	std::string fileName;
	int line;
	CodeTokenType type;
	std::string name;
	int offset;
	std::map< std::string, Ref<CodeBlock> > operands;

	static CodeToken* CreateTextToken(const std::string& file, int line, const std::string& text);
	static CodeToken* CreateVarToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarLowToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarHighToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarBaseToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarOffsetToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarTempToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarFunctionToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarBlockToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateVarIntToken(const std::string& file, int line, const std::string& name, int offset);
	static CodeToken* CreateInstrStartToken(const std::string& file, int line, const std::string& name);
	static CodeToken* CreateInstrEndToken(const std::string& file, int line);
	static CodeToken* CreateInstrEncodingToken(const std::string& file, int line, const std::string& name,
		const std::map< std::string, Ref<CodeBlock> >& operands);
	static CodeToken* CreateInstrEncodingValueToken(const std::string& file, int line, const std::string& name,
		const std::map< std::string, Ref<CodeBlock> >& operands);
};

struct OperandDefinition
{
	std::string name;
	Ref<CodeBlock> block;
};

class CodeBlock: public RefCountObject
{
	std::vector<CodeToken> m_tokens;

public:
	CodeBlock();
	CodeBlock(const CodeBlock& copy);

	void AddTextToken(const std::string& file, int line, const std::string& text);
	void AddInstrStartToken(const std::string& file, int line, const std::string& name);
	void AddInstrEndToken(const std::string& file, int line);
	void AddInstrEncodingToken(const std::string& file, int line, const std::string& name,
		const std::map< std::string, Ref<CodeBlock> >& operands);
	void AddInstrEncodingValueToken(const std::string& file, int line, const std::string& name,
		const std::map< std::string, Ref<CodeBlock> >& operands);

	void AddToken(const CodeToken& token);
	void AddTokens(const std::vector<CodeToken>& tokens);

	void ReplaceVar(const std::string& from, const std::string& to);

	const std::vector<CodeToken>& GetTokens() const { return m_tokens; }
};


#endif

