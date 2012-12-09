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
	Ref<Function> m_startFunction;

	bool m_markovReady;
	std::map< uint16_t, std::map< std::string, size_t> > m_markovChain;

	size_t AddInstructionToMarkovChain(uint16_t& prev, uint8_t* data, size_t len);
	void PrepareMarkovInstructionsFromFile(const std::string& filename);
	void PrepareMarkovInstructionsFromBlocks(const std::vector<ILBlock*>& codeBlocks);
	void InsertMarkovInstructions(OutputBlock* block, size_t len);

	bool LayoutCode(std::vector<ILBlock*>& codeBlocks);

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

	const Settings& GetSettings() { return m_settings; }
	std::vector< Ref<Function> >& GetFunctions() { return m_functions; }
	std::map< std::string, Ref<Function> >& GetFunctionsByName() { return m_functionsByName; }
	std::vector< Ref<Variable> >& GetVariables() { return m_variables; }
	std::map< std::string, Ref<Variable> >& GetVariablesByName() { return m_variablesByName; }
};


#endif

