// Copyright (c) 2011-2012 Rusty Wagner
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

#ifndef __PREPROCESSSTATE_H__
#define __PREPROCESSSTATE_H__

#include "Token.h"
#include <map>
#include <stack>
#include <string>
#include <vector>


struct Location
{
	std::string fileName;
	int lineNumber;
};

struct Macro
{
	std::string name;
	bool hasParams;
	std::vector<std::string> params;
	std::vector<Ref<Token>> tokens;
	Location location;
};

struct MacroExpansion
{
	Macro macro;
	size_t parens;
	std::vector<std::vector<Ref<Token>>> params;
	std::vector<Ref<Token>> curParam;
};

class PreprocessState
{
	std::string m_fileName;
	void* m_scanner;
	void* m_lvalue;
	int m_errors;

	std::map<std::string, Macro> m_macros;

	bool m_expansionInProgress;
	MacroExpansion m_expansion;
	std::stack<MacroExpansion> m_expansionStack;

	std::stack<bool> m_ifStack;
	size_t m_ifFailCount;
	size_t m_startingIfStackSize;

	std::string m_output;
	bool m_locationRequest;

	bool m_paste;

 public:
	PreprocessState(const std::string& name, void* scanner);
	PreprocessState(PreprocessState& parent, const std::string& name, void* scanner);
	~PreprocessState();

	const std::string& GetFileName() { return m_fileName; }
	int GetLineNumber();
	Location GetLocation();

	const std::string& GetOutput() { return m_output; }
	void Append(const std::string& str);
	void Append(Token* token);
	void AppendLocation();
	void RequestLocation() { m_locationRequest = true; }

	void* GetScanner() { return m_scanner; }
	void* GetLValue() const { return m_lvalue; }
	void SetLValue(void* lvalue) { m_lvalue = lvalue; }

	void Error() { m_errors++; }
	bool HasErrors() const { return m_errors != 0; }

	void IncludeSource(const std::string& source);
	void IncludeFile(const std::string& name);

	void Define(const std::string& name, const std::vector<std::string>& params,
	    const std::vector<Ref<Token>>& tokens, bool hasParams);
	void Undefine(const std::string& name);
	bool IsDefined(const std::string& name);

	void BeginMacroExpansion(const std::string& name);
	void FinishMacroExpansion();
	void Finalize();

	void BeginIf(bool result);
	void Else();
	void EndIf();

	static bool PreprocessSource(const std::string& source, const std::string& fileName,
	    std::string& output, PreprocessState* parent = NULL);
};


#endif
