#ifndef __LINKER_H__
#define __LINKER_H__

#include "Function.h"
#include "ParserState.h"
#include "PreprocessState.h"
#include "Settings.h"
#include "Variable.h"


struct ImportTable
{
	std::string module;
	Ref<Variable> table;
	std::vector<Ref<Function>> functions;
	std::vector<size_t> nameOffsets;
};

class Linker
{
	Settings m_settings;
	PreprocessState m_precompiledPreprocess;
	ParserState m_precompileState;
	std::vector<Ref<Function>> m_functions;
	std::map<std::string, Ref<Function>> m_functionsByName;
	std::vector<Ref<Variable>> m_variables;
	std::map<std::string, Ref<Variable>> m_variablesByName;
	Ref<Expr> m_initExpression;
	Ref<Function> m_startFunction;
	std::map<std::string, ImportTable> m_importTables;

	bool m_markovReady;
	std::map<uint16_t, std::map<std::string, size_t>> m_markovChain;

	size_t AddInstructionToMarkovChain(uint16_t& prev, uint8_t* data, size_t len);
	void PrepareMarkovInstructionsFromFile(const std::string& filename);
	void PrepareMarkovInstructionsFromBlocks(const std::vector<ILBlock*>& codeBlocks);
	void InsertMarkovInstructions(OutputBlock* block, size_t len);

	bool LayoutCode(std::vector<ILBlock*>& codeBlocks);

	uint32_t GetCaseInsensitiveNameHash(const std::string& name);
	uint32_t GetNameHash(const std::string& name);

 public:
	Linker(const Settings& settings);
	~Linker();

	bool ImportLibrary(InputBlock* input);
	bool ImportStandardLibrary();

	bool PrecompileHeader(const std::string& path);
	bool PrecompileSource(const std::string& source);
	bool CompileSource(const std::string& source, const std::string& filename);

	bool FinalizePrecompiledHeaders();
	bool FinalizeLink();

	bool OutputLibrary(OutputBlock* output);
	bool OutputCode(OutputBlock* finalBinary);
	bool WriteMapFile(const std::string& filename);

	const Settings& GetSettings() { return m_settings; }
	std::vector<Ref<Function>>& GetFunctions() { return m_functions; }
	std::map<std::string, Ref<Function>>& GetFunctionsByName() { return m_functionsByName; }
	std::vector<Ref<Variable>>& GetVariables() { return m_variables; }
	std::map<std::string, Ref<Variable>>& GetVariablesByName() { return m_variablesByName; }
};


#endif
