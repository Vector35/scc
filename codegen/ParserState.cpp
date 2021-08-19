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

#include "ParserState.h"
#include <stdio.h>

using namespace std;


ParserState::ParserState(const string& name, void* scanner) : m_fileName(name), m_scanner(scanner)
{
	m_errors = 0;
	m_line = 1;
	m_archBits = 32;
}


ParserState::~ParserState() {}


Location ParserState::GetLocation()
{
	Location loc;
	loc.fileName = GetFileName();
	loc.lineNumber = GetLineNumber();
	return loc;
}


void ParserState::DefineRegisterClass(
    uint32_t sizeFlags, const string& name, CodeBlock* fixed, bool isDefault)
{
	if (m_regClasses.find(name) != m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' already defined\n", GetFileName().c_str(),
		    GetLineNumber(), name.c_str());
		return;
	}

	m_regClasses[name] = new RegisterClassDef(REGCLASS_NORMAL, sizeFlags, name, name, fixed);

	if (isDefault)
	{
		if (m_defaultRegClass)
		{
			Error();
			fprintf(stderr, "%s:%d: error: more than one default register class defined\n",
			    GetFileName().c_str(), GetLineNumber());
			return;
		}

		m_defaultRegClass = m_regClasses[name];
	}

	m_regClassNames.insert(name);
}


void ParserState::DefineLargeRegisterClass(
    uint32_t sizeFlags, const string& name, const string& lowRegClass, const string& highRegClass)
{
	if (m_regClasses.find(name) != m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' already defined\n", GetFileName().c_str(),
		    GetLineNumber(), name.c_str());
		return;
	}

	if (m_regClasses.find(lowRegClass) == m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' not defined\n", GetFileName().c_str(),
		    GetLineNumber(), lowRegClass.c_str());
		return;
	}

	if (m_regClasses.find(highRegClass) == m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' not defined\n", GetFileName().c_str(),
		    GetLineNumber(), highRegClass.c_str());
		return;
	}

	m_regClasses[name] = new RegisterClassDef(sizeFlags, name, lowRegClass, highRegClass);
}


void ParserState::DefineTempRegisterClass(
    uint32_t sizeFlags, const string& name, const string& symRegClass)
{
	if (m_regClasses.find(name) != m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' already defined\n", GetFileName().c_str(),
		    GetLineNumber(), name.c_str());
		return;
	}

	if (m_regClasses.find(symRegClass) == m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' not defined\n", GetFileName().c_str(),
		    GetLineNumber(), symRegClass.c_str());
		return;
	}

	m_regClasses[name] = new RegisterClassDef(REGCLASS_TEMP, sizeFlags, name, symRegClass, NULL);
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


void ParserState::DefineRegisterSubclass(const std::string& name, const std::string& base)
{
	if (m_regClasses.find(base) == m_regClasses.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: register class '%s' not defined\n", GetFileName().c_str(),
		    GetLineNumber(), base.c_str());
		return;
	}

	m_regSubclasses[base].push_back(name);
}


void ParserState::DefineFunction(CodeBlock* func)
{
	m_funcs.push_back(func);
}


void ParserState::DefineInstrFunction(CodeBlock* func)
{
	m_instrFuncs.push_back(func);
}


void ParserState::DefineArchFunction(CodeBlock* func)
{
	m_archFuncs.push_back(func);
}


void ParserState::DefineStaticFunction(CodeBlock* func)
{
	m_staticFuncs.push_back(func);
}


void ParserState::DefineVariable(CodeBlock* var)
{
	m_vars.push_back(var);
}


void ParserState::DefineMatch(const string& file, int line, TreeNode* match, TreeNode* result,
    TreeNode* temp, CodeBlock* code)
{
	if (match->GetClass() == NODE_REG)
	{
		if (!result)
		{
			Error();
			fprintf(
			    stderr, "%s:%d: error: result match required\n", GetFileName().c_str(), GetLineNumber());
			return;
		}

		RegisterClassDef* regClass = GetRegisterClass(match->GetTypeName());
		if (regClass && (regClass->GetClassType() == REGCLASS_TEMP))
		{
			if (temp)
				m_tempRegMatches.push_back(
				    new Match(file, line, match, result, temp->GetChildNodes(), code));
			else
				m_tempRegMatches.push_back(
				    new Match(file, line, match, result, vector<Ref<TreeNode>>(), code));
			return;
		}
	}
	else if (result)
	{
		match->SetSizeFlags(match->GetSizeFlags() & result->GetSizeFlags());
	}

	if (temp)
		m_matches.push_back(new Match(file, line, match, result, temp->GetChildNodes(), code));
	else
		m_matches.push_back(new Match(file, line, match, result, vector<Ref<TreeNode>>(), code));
}


void ParserState::DefineEncoding(const std::string& name, Encoding* encoding)
{
	if (m_encodings.find(name) != m_encodings.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: encoding '%s' already defined\n", GetFileName().c_str(),
		    GetLineNumber(), name.c_str());
		return;
	}

	m_encodings[name] = encoding;

	if ((encoding->GetWidth() != 8) && (encoding->GetWidth() != 16) && (encoding->GetWidth() != 32) &&
	    (encoding->GetWidth() != 64))
	{
		Error();
		fprintf(stderr, "%s:%d: error: encoding '%s' has invalid size %d\n", GetFileName().c_str(),
		    GetLineNumber(), name.c_str(), (int)encoding->GetWidth());
		return;
	}
}


void ParserState::DefineInstruction(Instruction* instr)
{
	m_instrs.push_back(instr);
}


void ParserState::AddCallerSavedRegs(const vector<Ref<CodeBlock>>& regs)
{
	m_callerSavedRegs.insert(m_callerSavedRegs.end(), regs.begin(), regs.end());
}


void ParserState::AddCalleeSavedRegs(const vector<Ref<CodeBlock>>& regs)
{
	m_calleeSavedRegs.insert(m_calleeSavedRegs.end(), regs.begin(), regs.end());
}


void ParserState::DefineSpecialReg(const std::string& name, CodeBlock* reg)
{
	if (m_specialRegs.find(name) != m_specialRegs.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: special register '%s' already defined\n", GetFileName().c_str(),
		    GetLineNumber(), name.c_str());
		return;
	}

	m_specialRegs[name] = reg;
}


void ParserState::ExpandTempRegisterClasses()
{
	vector<Ref<Match>> expanded;

	for (vector<Ref<Match>>::iterator i = m_matches.begin(); i != m_matches.end(); ++i)
	{
		if (!(*i)->GetResult())
		{
			expanded.push_back(*i);
			continue;
		}

		RegisterClassDef* regClass = GetRegisterClass((*i)->GetResult()->GetTypeName());
		if (regClass->GetClassType() != REGCLASS_TEMP)
		{
			expanded.push_back(*i);
			continue;
		}

		// Rule has a result of a temporary register class, expand using the temporary register class
		// rules
		for (vector<Ref<Match>>::iterator j = m_tempRegMatches.begin(); j != m_tempRegMatches.end();
		     ++j)
		{
			if ((*j)->GetCode()->GetTokens().size() == 0)
			{
				// No code for this rule, create a replacement rule that simply changes the register class
				Ref<TreeNode> finalResult = new TreeNode(*(*j)->GetResult());
				Ref<Match> newMatch = new Match((*i)->GetFileName(), (*i)->GetLineNumber(),
				    (*i)->GetMatch(), finalResult, (*i)->GetTemps(), (*i)->GetCode());
				expanded.push_back(newMatch);
				continue;
			}

			vector<Ref<TreeNode>> temp = (*i)->GetTemps();
			for (vector<Ref<TreeNode>>::const_iterator k = (*j)->GetTemps().begin();
			     k != (*j)->GetTemps().end(); ++k)
			{
				TreeNode* newTemp = new TreeNode(**k);
				newTemp->SetName(string("_t_") + newTemp->GetName());
				temp.push_back(newTemp);
			}

			Ref<TreeNode> intermediate = new TreeNode(*(*j)->GetMatch());
			intermediate->SetName(string("_t_") + intermediate->GetName());
			temp.push_back(intermediate);

			Ref<TreeNode> finalResult = new TreeNode(*(*j)->GetResult());
			finalResult->SetName(string("_t_") + finalResult->GetName());

			Ref<CodeBlock> code = new CodeBlock(*(*i)->GetCode());
			code->ReplaceVar((*i)->GetResult()->GetName(), intermediate->GetName());

			Ref<CodeBlock> tempRegCode = new CodeBlock(*(*j)->GetCode());
			tempRegCode->ReplaceVar((*j)->GetMatch()->GetName(), intermediate->GetName());
			tempRegCode->ReplaceVar((*j)->GetResult()->GetName(), finalResult->GetName());
			code->AddTokens(tempRegCode->GetTokens());

			Ref<TreeNode> match = new TreeNode(*(*i)->GetMatch());
			match->SetSizeFlags(match->GetSizeFlags() & finalResult->GetSizeFlags());

			Ref<Match> newMatch =
			    new Match((*i)->GetFileName(), (*i)->GetLineNumber(), match, finalResult, temp, code);
			expanded.push_back(newMatch);
		}
	}

	for (size_t i = 0; i < expanded.size(); i++)
		expanded[i]->SetIndex(i);

	m_matches = expanded;
}


RegisterClassDef* ParserState::GetRegisterClass(const string& name) const
{
	map<string, Ref<RegisterClassDef>>::const_iterator i = m_regClasses.find(name);
	if (i == m_regClasses.end())
		return NULL;
	return i->second;
}


CodeBlock* ParserState::GetImmediateClass(const string& name) const
{
	map<string, Ref<CodeBlock>>::const_iterator i = m_immClasses.find(name);
	if (i == m_immClasses.end())
		return NULL;
	return i->second;
}


Encoding* ParserState::GetEncoding(const std::string& name) const
{
	map<string, Ref<Encoding>>::const_iterator i = m_encodings.find(name);
	if (i == m_encodings.end())
		return NULL;
	return i->second;
}
