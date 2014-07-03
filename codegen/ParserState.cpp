// Copyright (c) 2011-2014 Rusty Wagner
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
#include "ParserState.h"

using namespace std;


ParserState::ParserState(const string& name, void* scanner):
	m_fileName(name), m_scanner(scanner)
{
	m_errors = 0;
	m_line = 1;
}


ParserState::~ParserState()
{
}


Location ParserState::GetLocation()
{
	Location loc;
	loc.fileName = GetFileName();
	loc.lineNumber = GetLineNumber();
	return loc;
}


void ParserState::DefineRegisterClass(RegisterType type, const string& name, const string& symRegClass)
{
	if (m_regClasses.find(name) != m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' already defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return;
	}

	m_regClasses[name] = new RegisterClass(name, type, symRegClass);
}


void ParserState::DefineLargeRegisterClass(RegisterType type, const string& name,
	const string& lowRegClass, const string& highRegClass)
{
	if (m_regClasses.find(name) != m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' already defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return;
	}

	m_regClasses[name] = new RegisterClass(name, type, lowRegClass, highRegClass);
}


void ParserState::DefineTempRegisterClass(const string& name)
{
	if (m_regClasses.find(name) != m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' already defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return;
	}

	m_regClasses[name] = new RegisterClass(name);
}


void ParserState::DefineImmediateClass(const std::string& name, CodeBlock* code)
{
	if (m_immClasses.find(name) != m_immClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: immediate class '%s' already defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return;
	}

	m_immClasses[name] = code;
}


void ParserState::DefineFunction(CodeBlock* func)
{
	m_funcs.push_back(func);
}


void ParserState::DefineMatch(TreeNode* match, TreeNode* result, TreeNode* temp, CodeBlock* code)
{
	if (temp)
		m_matches.push_back(new Match(match, result, temp->GetChildNodes(), code));
	else
		m_matches.push_back(new Match(match, result, vector< Ref<TreeNode> >(), code));
}


RegisterClass* ParserState::GetRegisterClass(const string& name) const
{
	map< string, Ref<RegisterClass> >::const_iterator i = m_regClasses.find(name);
	if (i == m_regClasses.end())
		return NULL;
	return i->second;
}


CodeBlock* ParserState::GetImmediateClass(const string& name) const
{
	map< string, Ref<CodeBlock> >::const_iterator i = m_immClasses.find(name);
	if (i == m_immClasses.end())
		return NULL;
	return i->second;
}

