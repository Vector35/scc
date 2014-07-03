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


CodeToken* CodeToken::CreateTextToken(const string& text)
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_TEXT;
	token->name = text;
	return token;
}


CodeToken* CodeToken::CreateVarToken(const string& name)
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_VAR;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateVarLowToken(const string& name)
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_VAR_LOW;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateVarHighToken(const string& name)
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_VAR_HIGH;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateVarOffsetToken(const string& name, int offset)
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_VAR_OFFSET;
	token->name = name;
	token->offset = offset;
	return token;
}


CodeToken* CodeToken::CreateInstrStartToken(const string& name)
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_INSTR_START;
	token->name = name;
	return token;
}


CodeToken* CodeToken::CreateInstrEndToken()
{
	CodeToken* token = new CodeToken;
	token->type = TOKEN_INSTR_END;
	return token;
}


CodeBlock::CodeBlock()
{
}


void CodeBlock::AddTextToken(const string& text)
{
	CodeToken token;
	token.type = TOKEN_TEXT;
	token.name = text;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarToken(const string& name)
{
	CodeToken token;
	token.type = TOKEN_VAR;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarLowToken(const string& name)
{
	CodeToken token;
	token.type = TOKEN_VAR_LOW;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarHighToken(const string& name)
{
	CodeToken token;
	token.type = TOKEN_VAR_HIGH;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddVarOffsetToken(const string& name, int offset)
{
	CodeToken token;
	token.type = TOKEN_VAR_OFFSET;
	token.name = name;
	token.offset = offset;
	m_tokens.push_back(token);
}


void CodeBlock::AddInstrStartToken(const string& name)
{
	CodeToken token;
	token.type = TOKEN_INSTR_START;
	token.name = name;
	m_tokens.push_back(token);
}


void CodeBlock::AddInstrEndToken()
{
	CodeToken token;
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

