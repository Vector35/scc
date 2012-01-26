#ifndef __LINKER_H__
#define __LINKER_H__

#include "Function.h"
#include "Variable.h"
#include "Settings.h"
#include "PreprocessState.h"
#include "ParserState.h"


class Linker
{
	Settings m_settings;
	PreprocessState m_precompiledPreprocess;
	ParserState m_precompileState;
	std::vector< Ref<Function> > m_functions;
	std::map< std::string, Ref<Function> > m_functionsByName;
	std::vector< Ref<Variable> > m_variables;
	std::map< std::string, Ref<Variable> > m_variablesByName;
	Ref<Expr> m_initExpression;

public:
	Linker(const Settings& settings);
	~Linker();

	bool ImportLibrary(InputBlock* input);
	bool ImportStandardLibrary();

	bool PrecompileHeader(const std::string& path);
	bool CompileSource(const std::string& source, const std::string& filename);

	bool FinalizePrecompiledHeaders();
	bool FinalizeLink();

	bool OutputLibrary(OutputBlock* output);
	bool OutputCode(OutputBlock* finalBinary);
	bool WriteMapFile(const std::string& filename);
};


#endif

