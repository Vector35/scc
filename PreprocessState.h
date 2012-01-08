#ifndef __PREPROCESSSTATE_H__
#define __PREPROCESSSTATE_H__

#include <string>
#include <map>
#include <vector>
#include "Expr.h"
#include "Struct.h"


class PreprocessState
{
	std::string m_fileName;
	void* m_scanner;
	void* m_lvalue;
	int m_errors;

	std::string m_output;

public:
	PreprocessState(const std::string& name, void* scanner);
	~PreprocessState();

	const std::string& GetFileName() { return m_fileName; }
	int GetLineNumber();
	Location GetLocation();

	const std::string& GetOutput() { return m_output; }
	void Append(const std::string& text) { m_output += text; }
	void AppendLocation();

	void* GetScanner() { return m_scanner; }
	void* GetLValue() const { return m_lvalue; }
	void SetLValue(void* lvalue) { m_lvalue = lvalue; }

	void Error() { m_errors++; }
	bool HasErrors() const { return m_errors != 0; }

	static bool PreprocessSource(const std::string& source, const std::string& fileName, std::string& output);
};


#endif

