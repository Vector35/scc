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

#ifndef __PARSERSTATE_H__
#define __PARSERSTATE_H__

#include <string>
#include <map>
#include <vector>
#include <map>
#include <inttypes.h>
#include "PreprocessState.h"
#include "CodeBlock.h"
#include "TreeNode.h"
#include "RegisterClass.h"
#include "Match.h"


class ParserState
{
	std::string m_fileName, m_archName;
	int m_line;
	void* m_scanner;
	void* m_lvalue;
	int m_errors;
	Ref<RegisterClass> m_defaultRegClass;

	std::map< std::string, Ref<RegisterClass> > m_regClasses;
	std::map< std::string, Ref<CodeBlock> > m_immClasses;
	std::vector< Ref<CodeBlock> > m_funcs;
	std::vector< Ref<CodeBlock> > m_vars;
	std::vector< Ref<Match> > m_matches;
	std::vector< Ref<Match> > m_tempRegMatches;
	std::vector<std::string> m_includes;

public:
	ParserState(const std::string& name, void* scanner);
	~ParserState();

	const std::string& GetArchName() const { return m_archName; }
	void SetArchName(const std::string& name) { m_archName = name; }

	const std::string& GetFileName() const { return m_fileName; }
	int GetLineNumber() const { return m_line; }
	Location GetLocation();
	void SetLocation(const std::string& name, int line) { m_fileName = name; m_line = line; }

	void* GetScanner() { return m_scanner; }
	void SetScanner(void* scanner) { m_scanner = scanner; }
	void* GetLValue() const { return m_lvalue; }
	void SetLValue(void* lvalue) { m_lvalue = lvalue; }

	void Error() { m_errors++; }
	bool HasErrors() const { return m_errors != 0; }

	void DefineRegisterClass(RegisterType type, const std::string& name, const std::string& symRegClass, bool isDefault);
	void DefineLargeRegisterClass(RegisterType type, const std::string& name,
		const std::string& lowRegClass, const std::string& highRegClass);
	void DefineTempRegisterClass(const std::string& name, const std::string& symRegClass);

	void DefineImmediateClass(const std::string& name, CodeBlock* code);

	void DefineFunction(CodeBlock* func);
	void DefineVariable(CodeBlock* var);

	void DefineMatch(const std::string& file, int line, TreeNode* match, TreeNode* result, TreeNode* temp, CodeBlock* code);

	void AddInclude(const std::string& name) { m_includes.push_back(name); }

	void ExpandTempRegisterClasses();

	RegisterClass* GetRegisterClass(const std::string& name) const;
	RegisterClass* GetDefaultRegisterClass() const { return m_defaultRegClass; }
	bool IsRegisterClass(const std::string& name) const { return GetRegisterClass(name) != NULL; }
	CodeBlock* GetImmediateClass(const std::string& name) const;
	bool IsImmediateClass(const std::string& name) const { return GetImmediateClass(name) != NULL; }
	const std::map< std::string, Ref<RegisterClass> >& GetRegisterClasses() const { return m_regClasses; }
	const std::map< std::string, Ref<CodeBlock> >& GetImmediateClasses() const { return m_immClasses; }
	const std::vector< Ref<CodeBlock> >& GetFunctions() const { return m_funcs; }
	const std::vector< Ref<CodeBlock> >& GetVariables() const { return m_vars; }
	const std::vector< Ref<Match> >& GetMatches() const { return m_matches; }
	const std::vector<std::string>& GetIncludes() const { return m_includes; }
};


#endif

