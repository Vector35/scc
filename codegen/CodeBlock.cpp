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

#include "CodeBlock.h"

using namespace std;


CodeToken* CodeToken::CreateTextToken(const string& file, int line, const string& text)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_TEXT;
	token->name = text;
	return token;
}


CodeToken* CodeToken::CreateVarToken(const string& file, int line, const string& name)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_VAR;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateVarLowToken(const string& file, int line, const string& name)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_VAR_LOW;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateVarHighToken(const string& file, int line, const string& name)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_VAR_HIGH;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateVarOffsetToken(const string& file, int line, const string& name, int offset)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_VAR_OFFSET;
	token->name = name;
	token->offset = offset;
	return token;
}


CodeToken* CodeToken::CreateInstrStartToken(const string& file, int line, const string& name)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_INSTR_START;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateInstrEndToken(const string& file, int line)
{
	CodeToken* token = new CodeToken;
	token->fileName = file;
	token->line = line;
	token->type = TOKEN_INSTR_END;
	return token;
}


CodeBlock::CodeBlock()
{
}


CodeBlock::CodeBlock(const CodeBlock& copy)
{
	m_tokens = copy.m_tokens;
}


void CodeBlock::AddTextToken(const string& file, int line, const string& text)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_TEXT;
	token.name = text;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarToken(const string& file, int line, const string& name)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_VAR;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarLowToken(const string& file, int line, const string& name)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_VAR_LOW;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarHighToken(const string& file, int line, const string& name)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_VAR_HIGH;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarOffsetToken(const string& file, int line, const string& name, int offset)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_VAR_OFFSET;
	token.name = name;
	token.offset = offset;
	m_tokens.push_back(token);
}


void CodeBlock::AddInstrStartToken(const string& file, int line, const string& name)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_INSTR_START;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddInstrEndToken(const string& file, int line)
{
	CodeToken token;
	token.fileName = file;
	token.line = line;
	token.type = TOKEN_INSTR_END;
	m_tokens.push_back(token);
}


void CodeBlock::AddToken(const CodeToken& token)
{
	m_tokens.push_back(token);
}


void CodeBlock::AddTokens(const vector<CodeToken>& tokens)
{
	for (vector<CodeToken>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
		AddToken(*i);
}


void CodeBlock::ReplaceVar(const std::string& from, const std::string& to)
{
	for (vector<CodeToken>::iterator i = m_tokens.begin(); i != m_tokens.end(); ++i)
	{
		if ((i->type != TOKEN_VAR) && (i->type != TOKEN_VAR_LOW) && (i->type != TOKEN_VAR_HIGH) && (i->type == TOKEN_VAR_OFFSET))
			continue;
		if (i->name != from)
			continue;
		i->name = to;
	}
}

