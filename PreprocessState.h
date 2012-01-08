#ifndef __PREPROCESSSTATE_H__
#define __PREPROCESSSTATE_H__

#include <string>
#include <map>
#include <vector>
#include <stack>
#include "Expr.h"
#include "Struct.h"
#include "Token.h"


struct Macro
{
	std::string name;
	std::vector<std::string> params;
	std::vector< Ref<Token> > tokens;
};

struct MacroExpansion
{
	Macro macro;
	size_t parens;
	std::vector< std::vector< Ref<Token> > > params;
	std::vector< Ref<Token> > curParam;
};

class PreprocessState
{
	std::string m_fileName;
	void* m_scanner;
	void* m_lvalue;
	int m_errors;

	std::map< std::string, Macro > m_macros;

	bool m_expansionInProgress;
	MacroExpansion m_expansion;
	std::stack<MacroExpansion> m_expansionStack;

	std::stack<bool> m_ifStack;
	size_t m_ifFailCount;

	std::string m_output;
	bool m_locationRequest;

public:
	PreprocessState(const std::string& name, void* scanner);
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

	void IncludeFile(const std::string& name);

	void Define(const std::string& name, const std::vector<std::string>& params, const std::vector< Ref<Token> >& tokens);
	void Undefine(const std::string& name);
	bool IsDefined(const std::string& name);

	void BeginMacroExpansion(const std::string& name);
	void FinishMacroExpansion();
	void Finalize();

	void BeginIf(bool result);
	void Else();
	void EndIf();

	static bool PreprocessSource(const std::string& source, const std::string& fileName, std::string& output);
};


#endif

