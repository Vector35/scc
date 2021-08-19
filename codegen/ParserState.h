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

#include "CodeBlock.h"
#include "Encoding.h"
#include "Instruction.h"
#include "Match.h"
#include "PreprocessState.h"
#include "RegisterClass.h"
#include "TreeNode.h"
#include <inttypes.h>
#include <map>
#include <set>
#include <string>
#include <vector>


class ParserState
{
	std::string m_fileName, m_archName;
	int m_line;
	void* m_scanner;
	void* m_lvalue;
	int m_errors;
	Ref<RegisterClassDef> m_defaultRegClass;
	uint32_t m_archBits;

	std::map<std::string, Ref<RegisterClassDef>> m_regClasses;
	std::map<std::string, std::vector<std::string>> m_regSubclasses;
	std::map<std::string, Ref<CodeBlock>> m_immClasses;
	std::map<std::string, Ref<Encoding>> m_encodings;
	std::vector<Ref<CodeBlock>> m_funcs;
	std::vector<Ref<CodeBlock>> m_instrFuncs;
	std::vector<Ref<CodeBlock>> m_archFuncs;
	std::vector<Ref<CodeBlock>> m_staticFuncs;
	std::vector<Ref<CodeBlock>> m_vars;
	std::vector<Ref<Match>> m_matches;
	std::vector<Ref<Match>> m_tempRegMatches;
	std::vector<Ref<Instruction>> m_instrs;
	std::set<std::string> m_regClassNames;
	std::vector<Ref<CodeBlock>> m_callerSavedRegs;
	std::vector<Ref<CodeBlock>> m_calleeSavedRegs;
	std::map<std::string, Ref<CodeBlock>> m_specialRegs;
	std::vector<std::string> m_includes;

 public:
	ParserState(const std::string& name, void* scanner);
	~ParserState();

	const std::string& GetArchName() const { return m_archName; }
	void SetArchName(const std::string& name) { m_archName = name; }

	uint32_t GetArchBits() const { return m_archBits; }
	void SetArchBits(uint32_t bits) { m_archBits = bits; }

	const std::string& GetFileName() const { return m_fileName; }
	int GetLineNumber() const { return m_line; }
	Location GetLocation();
	void SetLocation(const std::string& name, int line)
	{
		m_fileName = name;
		m_line = line;
	}

	void* GetScanner() { return m_scanner; }
	void SetScanner(void* scanner) { m_scanner = scanner; }
	void* GetLValue() const { return m_lvalue; }
	void SetLValue(void* lvalue) { m_lvalue = lvalue; }

	void Error() { m_errors++; }
	bool HasErrors() const { return m_errors != 0; }

	void DefineRegisterClass(
	    uint32_t sizeFlags, const std::string& name, CodeBlock* fixed, bool isDefault);
	void DefineLargeRegisterClass(uint32_t sizeFlags, const std::string& name,
	    const std::string& lowRegClass, const std::string& highRegClass);
	void DefineTempRegisterClass(
	    uint32_t sizeFlags, const std::string& name, const std::string& symRegClass);
	void DefineRegisterSubclass(const std::string& name, const std::string& base);

	void DefineImmediateClass(const std::string& name, CodeBlock* code);

	void DefineFunction(CodeBlock* func);
	void DefineInstrFunction(CodeBlock* func);
	void DefineArchFunction(CodeBlock* func);
	void DefineStaticFunction(CodeBlock* func);
	void DefineVariable(CodeBlock* var);

	void DefineMatch(const std::string& file, int line, TreeNode* match, TreeNode* result,
	    TreeNode* temp, CodeBlock* code);

	void DefineEncoding(const std::string& name, Encoding* encoding);
	void DefineInstruction(Instruction* instr);

	void AddCallerSavedRegs(const std::vector<Ref<CodeBlock>>& regs);
	void AddCalleeSavedRegs(const std::vector<Ref<CodeBlock>>& regs);
	void DefineSpecialReg(const std::string& name, CodeBlock* reg);

	void AddInclude(const std::string& name) { m_includes.push_back(name); }

	void ExpandTempRegisterClasses();

	RegisterClassDef* GetRegisterClass(const std::string& name) const;
	RegisterClassDef* GetDefaultRegisterClass() const { return m_defaultRegClass; }
	bool IsRegisterClass(const std::string& name) const { return GetRegisterClass(name) != NULL; }
	CodeBlock* GetImmediateClass(const std::string& name) const;
	bool IsImmediateClass(const std::string& name) const { return GetImmediateClass(name) != NULL; }
	Encoding* GetEncoding(const std::string& name) const;
	const std::map<std::string, Ref<RegisterClassDef>>& GetRegisterClasses() const
	{
		return m_regClasses;
	}
	const std::map<std::string, std::vector<std::string>>& GetRegisterSubclasses() const
	{
		return m_regSubclasses;
	}
	const std::map<std::string, Ref<CodeBlock>>& GetImmediateClasses() const { return m_immClasses; }
	const std::map<std::string, Ref<Encoding>>& GetEncodings() const { return m_encodings; }
	const std::vector<Ref<CodeBlock>>& GetFunctions() const { return m_funcs; }
	const std::vector<Ref<CodeBlock>>& GetInstrFunctions() const { return m_instrFuncs; }
	const std::vector<Ref<CodeBlock>>& GetArchFunctions() const { return m_archFuncs; }
	const std::vector<Ref<CodeBlock>>& GetStaticFunctions() const { return m_staticFuncs; }
	const std::vector<Ref<CodeBlock>>& GetVariables() const { return m_vars; }
	const std::vector<Ref<Match>>& GetMatches() const { return m_matches; }
	const std::vector<Ref<Instruction>>& GetInstructions() const { return m_instrs; }
	const std::vector<std::string>& GetIncludes() const { return m_includes; }
	const std::set<std::string>& GetRegisterClassNames() const { return m_regClassNames; }
	const std::vector<Ref<CodeBlock>>& GetCallerSavedRegs() const { return m_callerSavedRegs; }
	const std::vector<Ref<CodeBlock>>& GetCalleeSavedRegs() const { return m_calleeSavedRegs; }
	const std::map<std::string, Ref<CodeBlock>>& GetSpecialRegs() const { return m_specialRegs; }
};


#endif
