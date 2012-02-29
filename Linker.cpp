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

#include <stdio.h>
#include <string.h>
#include "asmx86.h"
#include "Linker.h"
#include "CodeParser.h"
#include "CodeLexer.h"
#include "Optimize.h"
#include "OutputX86.h"
#include "OutputX64.h"
#include "ElfOutput.h"

using namespace std;


// Internal libraries
extern unsigned char Obj_x86_lib[];
extern unsigned int Obj_x86_lib_len;
extern unsigned char Obj_x64_lib[];
extern unsigned int Obj_x64_lib_len;
extern unsigned char Obj_linux_x86_lib[];
extern unsigned int Obj_linux_x86_lib_len;
extern unsigned char Obj_linux_x64_lib[];
extern unsigned int Obj_linux_x64_lib_len;
extern unsigned char Obj_freebsd_x86_lib[];
extern unsigned int Obj_freebsd_x86_lib_len;
extern unsigned char Obj_freebsd_x64_lib[];
extern unsigned int Obj_freebsd_x64_lib_len;
extern unsigned char Obj_mach_x86_lib[];
extern unsigned int Obj_mach_x86_lib_len;
extern unsigned char Obj_mach_x64_lib[];
extern unsigned int Obj_mach_x64_lib_len;
extern unsigned char Obj_windows_x86_lib[];
extern unsigned int Obj_windows_x86_lib_len;
extern unsigned char Obj_windows_x64_lib[];
extern unsigned int Obj_windows_x64_lib_len;

// Create weak symbols for empty libraries to be used during bootstrap process
unsigned char __attribute__((weak)) Obj_x86_lib[] = {};
unsigned int __attribute__((weak)) Obj_x86_lib_len = 0;
unsigned char __attribute__((weak)) Obj_x64_lib[] = {};
unsigned int __attribute__((weak)) Obj_x64_lib_len = 0;
unsigned char __attribute__((weak)) Obj_linux_x86_lib[] = {};
unsigned int __attribute__((weak)) Obj_linux_x86_lib_len = 0;
unsigned char __attribute__((weak)) Obj_linux_x64_lib[] = {};
unsigned int __attribute__((weak)) Obj_linux_x64_lib_len = 0;
unsigned char __attribute__((weak)) Obj_freebsd_x86_lib[] = {};
unsigned int __attribute__((weak)) Obj_freebsd_x86_lib_len = 0;
unsigned char __attribute__((weak)) Obj_freebsd_x64_lib[] = {};
unsigned int __attribute__((weak)) Obj_freebsd_x64_lib_len = 0;
unsigned char __attribute__((weak)) Obj_mach_x86_lib[] = {};
unsigned int __attribute__((weak)) Obj_mach_x86_lib_len = 0;
unsigned char __attribute__((weak)) Obj_mach_x64_lib[] = {};
unsigned int __attribute__((weak)) Obj_mach_x64_lib_len = 0;
unsigned char __attribute__((weak)) Obj_windows_x86_lib[] = {};
unsigned int __attribute__((weak)) Obj_windows_x86_lib_len = 0;
unsigned char __attribute__((weak)) Obj_windows_x64_lib[] = {};
unsigned int __attribute__((weak)) Obj_windows_x64_lib_len = 0;


extern int Code_parse(ParserState* state);
extern void Code_set_lineno(int line, void* yyscanner);


Linker::Linker(const Settings& settings): m_settings(settings), m_precompiledPreprocess("precompiled headers", NULL),
	m_precompileState("precompiled headers", NULL), m_initExpression(new Expr(EXPR_SEQUENCE))
{
}


Linker::~Linker()
{
}


bool Linker::ImportLibrary(InputBlock* input)
{
	// Deserialize precompiled header state
	if (!m_precompiledPreprocess.Deserialize(input))
		return false;
	if (!m_precompileState.Deserialize(input))
		return false;

	// Deserialize functions
	size_t functionCount;
	if (!input->ReadNativeInteger(functionCount))
		return false;
	for (size_t i = 0; i < functionCount; i++)
	{
		Function* func = Function::Deserialize(input);
		if (!func)
			return false;
		m_functions.push_back(func);
	}

	// Deserialize variables
	size_t variableCount;
	if (!input->ReadNativeInteger(variableCount))
		return false;
	for (size_t i = 0; i < variableCount; i++)
	{
		Variable* var = Variable::Deserialize(input);
		if (!var)
			return false;
		m_variables.push_back(var);
	}

	// Deserialize function name map
	size_t functionMapCount;
	if (!input->ReadNativeInteger(functionMapCount))
		return false;
	for (size_t i = 0; i < functionMapCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Function* func = Function::Deserialize(input);
		if (!func)
			return false;

		m_functionsByName[name] = func;
	}

	// Deserialize variable name map
	size_t variableMapCount;
	if (!input->ReadNativeInteger(variableMapCount))
		return false;
	for (size_t i = 0; i < variableMapCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Variable* var = Variable::Deserialize(input);
		if (!var)
			return false;

		m_variablesByName[name] = var;
	}

	// Deserialize initialization expression
	m_initExpression = Expr::Deserialize(input);
	if (!m_initExpression)
		return false;

	return true;
}


bool Linker::ImportStandardLibrary()
{
	unsigned char* lib;
	unsigned int len;

	switch (m_settings.os)
	{
	case OS_LINUX:
		if (m_settings.preferredBits == 32)
		{
			lib = Obj_linux_x86_lib;
			len = Obj_linux_x86_lib_len;
		}
		else
		{
			lib = Obj_linux_x64_lib;
			len = Obj_linux_x64_lib_len;
		}
		break;
	case OS_FREEBSD:
		if (m_settings.preferredBits == 32)
		{
			lib = Obj_freebsd_x86_lib;
			len = Obj_freebsd_x86_lib_len;
		}
		else
		{
			lib = Obj_freebsd_x64_lib;
			len = Obj_freebsd_x64_lib_len;
		}
		break;
	case OS_MACH:
		if (m_settings.preferredBits == 32)
		{
			lib = Obj_mach_x86_lib;
			len = Obj_mach_x86_lib_len;
		}
		else
		{
			lib = Obj_mach_x64_lib;
			len = Obj_mach_x64_lib_len;
		}
		break;
	case OS_WINDOWS:
		if (m_settings.preferredBits == 32)
		{
			lib = Obj_windows_x86_lib;
			len = Obj_windows_x86_lib_len;
		}
		else
		{
			lib = Obj_windows_x64_lib;
			len = Obj_windows_x64_lib_len;
		}
		break;
	default:
		if (m_settings.preferredBits == 32)
		{
			lib = Obj_x86_lib;
			len = Obj_x86_lib_len;
		}
		else
		{
			lib = Obj_x64_lib;
			len = Obj_x64_lib_len;
		}
		break;
	}

	if (len != 0)
	{
		InputBlock input;
		input.code = lib;
		input.len = len;
		input.offset = 0;

		if (!ImportLibrary(&input))
			return false;
	}

	return true;
}


bool Linker::PrecompileHeader(const string& path)
{
	m_precompiledPreprocess.IncludeFile(path);
	if (m_precompiledPreprocess.HasErrors())
		return false;
	return true;
}


bool Linker::FinalizePrecompiledHeaders()
{
	yyscan_t scanner;
	Code_lex_init(&scanner);
	m_precompileState.SetScanner(scanner);

	YY_BUFFER_STATE buf = Code__scan_string(m_precompiledPreprocess.GetOutput().c_str(), scanner);
	Code__switch_to_buffer(buf, scanner);
	Code_set_lineno(1, scanner);

	bool ok = true;
	if (Code_parse(&m_precompileState) != 0)
		ok = false;
	if (m_precompileState.HasErrors())
		ok = false;

	Code_lex_destroy(scanner);
	return ok;
}


bool Linker::CompileSource(const std::string& source, const std::string& filename)
{
	string preprocessed;
	if (!PreprocessState::PreprocessSource(source, filename, preprocessed, &m_precompiledPreprocess))
		return false;

	yyscan_t scanner;
	Code_lex_init(&scanner);
	ParserState parser(&m_precompileState, filename.c_str(), scanner);

	YY_BUFFER_STATE buf = Code__scan_string(preprocessed.c_str(), scanner);
	Code__switch_to_buffer(buf, scanner);
	Code_set_lineno(1, scanner);

	bool ok = true;
	if (Code_parse(&parser) != 0)
		ok = false;
	if (parser.HasErrors())
		ok = false;

	Code_lex_destroy(scanner);

	if (!ok)
		return false;

	// First, propogate type information
	parser.SetInitExpression(parser.GetInitExpression()->Simplify(&parser));
	parser.GetInitExpression()->ComputeType(&parser, NULL);
	for (map< string, Ref<Function> >::const_iterator i = parser.GetFunctions().begin();
		i != parser.GetFunctions().end(); i++)
	{
		if (!i->second->IsFullyDefined())
			continue;
		i->second->SetBody(i->second->GetBody()->Simplify(&parser));
		i->second->GetBody()->ComputeType(&parser, i->second);
		i->second->SetBody(i->second->GetBody()->Simplify(&parser));
	}
	if (parser.HasErrors())
		return false;

	// Generate IL
	for (map< string, Ref<Function> >::const_iterator i = parser.GetFunctions().begin();
		i != parser.GetFunctions().end(); i++)
	{
		if (!i->second->IsFullyDefined())
			continue;
		i->second->GenerateIL(&parser);
		i->second->ReportUndefinedLabels(&parser);
	}
	if (parser.HasErrors())
		return false;

	// Link functions to other files
	for (map< string, Ref<Function> >::const_iterator i = parser.GetFunctions().begin();
		i != parser.GetFunctions().end(); i++)
	{
		if (i->second->IsFullyDefined())
		{
			// Function is defined in this file
			if (i->second->IsLocalScope())
			{
				// Function is in local scope, add it to list but not to name table
				m_functions.push_back(i->second);
			}
			else
			{
				// Funciton is in global scope
				if (m_functionsByName.find(i->second->GetName()) != m_functionsByName.end())
				{
					// Function by this name already defined in another file
					Function* prev = m_functionsByName[i->second->GetName()];
					if (prev->IsFullyDefined())
					{
						// Both functions have a body, this is an error
						parser.Error();
						fprintf(stderr, "%s:%d: error: duplicate function '%s' during link\n",
							i->second->GetLocation().fileName.c_str(),
							i->second->GetLocation().lineNumber, i->second->GetName().c_str());
						fprintf(stderr, "%s:%d: previous definition of '%s'\n",
							prev->GetLocation().fileName.c_str(), prev->GetLocation().lineNumber,
							prev->GetName().c_str());
					}
					else
					{
						// Other function was a prototype, check for compatibility
						vector< pair< Ref<Type>, string> > params;
						for (vector<FunctionParameter>::const_iterator j =
							prev->GetParameters().begin(); j != prev->GetParameters().end(); j++)
							params.push_back(pair< Ref<Type>, string>(j->type, j->name));
						if (!i->second->IsCompatible(prev->GetReturnValue(),
							prev->GetCallingConvention(), params, prev->HasVariableArguments()))
						{
							parser.Error();
							fprintf(stderr, "%s:%d: error: function '%s' incompatible with prototype\n",
								i->second->GetLocation().fileName.c_str(),
								i->second->GetLocation().lineNumber,
								i->second->GetName().c_str());
							fprintf(stderr, "%s:%d: prototype definition of '%s'\n",
								prev->GetLocation().fileName.c_str(),
								prev->GetLocation().lineNumber,
								prev->GetName().c_str());
						}

						// Replace old references with the fully defined one
						for (vector< Ref<Function> >::iterator j = m_functions.begin();
							j != m_functions.end(); j++)
							(*j)->ReplaceFunction(prev, i->second);
						m_initExpression->ReplaceFunction(prev, i->second);
					}
				}

				m_functions.push_back(i->second);
				m_functionsByName[i->second->GetName()] = i->second;
			}
		}
		else
		{
			// Function is a prototype only, ignore local scope
			if (!i->second->IsLocalScope())
			{
				if (m_functionsByName.find(i->second->GetName()) != m_functionsByName.end())
				{
					// Function by this name already defined in another file
					Function* prev = m_functionsByName[i->second->GetName()];

					// Check for compatibility
					vector< pair< Ref<Type>, string> > params;
					for (vector<FunctionParameter>::const_iterator j =
						prev->GetParameters().begin(); j != prev->GetParameters().end(); j++)
						params.push_back(pair< Ref<Type>, string>(j->type, j->name));
					if (!i->second->IsCompatible(prev->GetReturnValue(),
						prev->GetCallingConvention(), params, prev->HasVariableArguments()))
					{
						parser.Error();
						fprintf(stderr, "%s:%d: error: function '%s' incompatible with prototype\n",
							i->second->GetLocation().fileName.c_str(),
							i->second->GetLocation().lineNumber,
							i->second->GetName().c_str());
						fprintf(stderr, "%s:%d: prototype definition of '%s'\n",
							prev->GetLocation().fileName.c_str(),
							prev->GetLocation().lineNumber,
							prev->GetName().c_str());
					}

					// Replace references with existing definition
					for (map< string, Ref<Function> >::const_iterator j =
						parser.GetFunctions().begin(); j != parser.GetFunctions().end(); j++)
						j->second->ReplaceFunction(i->second, prev);
					parser.GetInitExpression()->ReplaceFunction(i->second, prev);
				}
				else
				{
					// New prototype, add to list of functions
					m_functions.push_back(i->second);
					m_functionsByName[i->second->GetName()] = i->second;
				}
			}
		}
	}

	if (parser.HasErrors())
		return 1;

	// Add initialization expression to global expression
	m_initExpression->AddChild(parser.GetInitExpression());

	// Link variables to other files
	for (vector< Ref<Variable> >::const_iterator i = parser.GetGlobalScope()->GetVariables().begin();
		i != parser.GetGlobalScope()->GetVariables().end(); i++)
	{
		if ((*i)->IsExternal())
		{
			// Variable is external
			if (m_variablesByName.find((*i)->GetName()) != m_variablesByName.end())
			{
				// Variable is defined in another file
				Variable* prev = m_variablesByName[(*i)->GetName()];

				// Check for compatibility
				if ((*prev->GetType()) != (*(*i)->GetType()))
				{
					parser.Error();
					fprintf(stderr, "%s:%d: error: variable '%s' incompatible with previous definition\n",
						(*i)->GetLocation().fileName.c_str(),
						(*i)->GetLocation().lineNumber,
						(*i)->GetName().c_str());
					fprintf(stderr, "%s:%d: previous definition of '%s'\n",
						prev->GetLocation().fileName.c_str(),
						prev->GetLocation().lineNumber,
						prev->GetName().c_str());
				}

				if (!prev->IsExternal())
				{
					// Previous definition is complete, replace references with the correct definition
					for (vector< Ref<Function> >::iterator j = m_functions.begin(); j != m_functions.end(); j++)
						(*j)->ReplaceVariable(*i, prev);
					m_initExpression->ReplaceVariable(*i, prev);
				}
			}
			else
			{
				// New definition
				m_variables.push_back(*i);
				m_variablesByName[(*i)->GetName()] = *i;
			}
		}
		else if ((*i)->IsLocalScope())
		{
			// Variable is local to the file, add to list but do not bind in name table
			m_variables.push_back(*i);
		}
		else
		{
			// Variable is global
			if (m_variablesByName.find((*i)->GetName()) != m_variablesByName.end())
			{
				// Variable is defined in another file
				Variable* prev = m_variablesByName[(*i)->GetName()];

				// Check for compatibility and duplicates
				if (!prev->IsExternal())
				{
					parser.Error();
					fprintf(stderr, "%s:%d: error: duplicate variable '%s' during link\n",
						(*i)->GetLocation().fileName.c_str(),
						(*i)->GetLocation().lineNumber,
						(*i)->GetName().c_str());
					fprintf(stderr, "%s:%d: previous definition of '%s'\n",
						prev->GetLocation().fileName.c_str(),
						prev->GetLocation().lineNumber,
						prev->GetName().c_str());
				}
				else if ((*prev->GetType()) != (*(*i)->GetType()))
				{
					parser.Error();
					fprintf(stderr, "%s:%d: error: variable '%s' incompatible with previous definition\n",
						(*i)->GetLocation().fileName.c_str(),
						(*i)->GetLocation().lineNumber,
						(*i)->GetName().c_str());
					fprintf(stderr, "%s:%d: previous definition of '%s'\n",
						prev->GetLocation().fileName.c_str(),
						prev->GetLocation().lineNumber,
						prev->GetName().c_str());
				}

				// Replace old external references with this definition
				for (vector< Ref<Function> >::iterator j = m_functions.begin();
					j != m_functions.end(); j++)
					(*j)->ReplaceVariable(prev, *i);
				m_initExpression->ReplaceVariable(prev, *i);
			}

			// Add definition to variable list
			m_variables.push_back(*i);
			m_variablesByName[(*i)->GetName()] = *i;
		}
	}

	if (parser.HasErrors())
		return false;

	return true;
}


bool Linker::OutputLibrary(OutputBlock* output)
{
	// Serialize precompiled header state
	m_precompiledPreprocess.Serialize(output);
	m_precompileState.Serialize(output);

	// Serialize function objects
	output->WriteInteger(m_functions.size());
	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
		(*i)->Serialize(output);

	// Serialize variable objects
	output->WriteInteger(m_variables.size());
	for (vector< Ref<Variable> >::iterator i = m_variables.begin(); i != m_variables.end(); i++)
		(*i)->Serialize(output);

	// Serialize function name map
	output->WriteInteger(m_functionsByName.size());
	for (map< string, Ref<Function> >::iterator i = m_functionsByName.begin(); i != m_functionsByName.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	// Serialize variable name map
	output->WriteInteger(m_variablesByName.size());
	for (map< string, Ref<Variable> >::iterator i = m_variablesByName.begin(); i != m_variablesByName.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	// Serialize initialization expression
	m_initExpression->Serialize(output);
	return true;
}


bool Linker::FinalizeLink()
{
	// Link complete, remove prototype functions from linked set
	for (size_t i = 0; i < m_functions.size(); i++)
	{
		if (!m_functions[i]->IsFullyDefined())
		{
			m_functions.erase(m_functions.begin() + i);
			i--;
		}
	}

	// Remove extern variables from linked set
	for (size_t i = 0; i < m_variables.size(); i++)
	{
		if (m_variables[i]->IsExternal())
		{
			m_variables.erase(m_variables.begin() + i);
			i--;
		}
	}

	// Find main function
	map< string, Ref<Function> >::iterator mainFuncRef = m_functionsByName.find("main");
	if (mainFuncRef == m_functionsByName.end())
	{
		fprintf(stderr, "error: function 'main' is undefined\n");
		return false;
	}
	Ref<Function> mainFunc = mainFuncRef->second;

	if (mainFunc->HasVariableArguments())
	{
		fprintf(stderr, "error: function 'main' can not have variable arguments\n");
		return false;
	}

	// Find exit function
	map< string, Ref<Function> >::iterator exitFuncRef = m_functionsByName.find("exit");
	if (exitFuncRef == m_functionsByName.end())
	{
		fprintf(stderr, "error: function 'exit' is undefined\n");
		return false;
	}
	Ref<Function> exitFunc = exitFuncRef->second;

	// Generate _start function
	map< string, Ref<Function> >::iterator entryFuncRef = m_functionsByName.find("_start");
	if (entryFuncRef != m_functionsByName.end())
	{
		fprintf(stderr, "error: cannot override internal function '_start'\n");
		return false;
	}

	FunctionInfo startInfo;
	startInfo.returnValue = mainFunc->GetReturnValue();
	startInfo.callingConvention = mainFunc->GetCallingConvention();
	startInfo.name = "_start";
	startInfo.subarch = SUBARCH_DEFAULT;
	startInfo.noReturn = !m_settings.allowReturn;
	startInfo.location = mainFunc->GetLocation();

	// Set up _start parameters to mirror main
	vector< Ref<Variable> > paramVars;
	for (vector<FunctionParameter>::const_iterator i = mainFunc->GetParameters().begin();
		i != mainFunc->GetParameters().end(); i++)
	{
		string name = i->name;
		if (name.c_str() == 0)
		{
			char str[32];
			sprintf(str, "$%d", (int)paramVars.size());
			name = str;
		}

		startInfo.params.push_back(pair< Ref<Type>, string >(i->type, name));

		Variable* var = new Variable(paramVars.size(), i->type, name);
		paramVars.push_back(var);
	}

	Ref<Function> startFunction = new Function(startInfo, false);
	m_functions.insert(m_functions.begin(), startFunction);
	m_functionsByName["_start"] = startFunction;

	Ref<Expr> startBody = new Expr(EXPR_SEQUENCE);

	// If using encoded pointers, choose the key now
	if (m_settings.encodePointers)
	{
		m_settings.encodePointerKey = new Variable(VAR_GLOBAL, Type::IntType(GetTargetPointerSize(), false), "@pointer_key");
		m_variables.push_back(m_settings.encodePointerKey);
		m_variablesByName["@pointer_key"] = m_settings.encodePointerKey;

		Ref<Expr> keyExpr = Expr::VariableExpr(mainFunc->GetLocation(), m_settings.encodePointerKey);
		Ref<Expr> valueExpr = new Expr(mainFunc->GetLocation(), (GetTargetPointerSize() == 4) ? EXPR_RDTSC_LOW : EXPR_RDTSC);
		startBody->AddChild(Expr::BinaryExpr(mainFunc->GetLocation(), EXPR_ASSIGN, keyExpr, valueExpr));
	}

	// Add global variable initialization expression
	startBody->AddChild(m_initExpression);

	Ref<Expr> mainExpr = Expr::FunctionExpr(mainFunc->GetLocation(), mainFunc);
	Ref<Expr> exitExpr = Expr::FunctionExpr(mainFunc->GetLocation(), exitFunc);

	// Generate call to main
	vector< Ref<Expr> > params;
	for (size_t i = 0; i < mainFunc->GetParameters().size(); i++)
		params.push_back(Expr::VariableExpr(mainFunc->GetLocation(), paramVars[i]));
	Ref<Expr> callExpr = Expr::CallExpr(mainFunc->GetLocation(), mainExpr, params);

	// Handle result of main
	if (m_settings.allowReturn)
	{
		if (mainFunc->GetReturnValue()->GetClass() == TYPE_VOID)
			startBody->AddChild(callExpr);
		else
			startBody->AddChild(Expr::UnaryExpr(mainFunc->GetLocation(), EXPR_RETURN, callExpr));
	}
	else if (mainFunc->GetReturnValue()->GetClass() == TYPE_VOID)
	{
		startBody->AddChild(callExpr);

		vector< Ref<Expr> > exitParams;
		exitParams.push_back(new Expr(mainFunc->GetLocation(), EXPR_UNDEFINED));
		startBody->AddChild(Expr::CallExpr(mainFunc->GetLocation(), exitExpr, exitParams));
	}
	else
	{
		vector< Ref<Expr> > exitParams;
		exitParams.push_back(callExpr);
		startBody->AddChild(Expr::CallExpr(mainFunc->GetLocation(), exitExpr, exitParams));
	}

	// Generate code for _start
	startFunction->SetBody(startBody);

	// First, propogate type information
	ParserState startState("_start", NULL);
	startFunction->SetBody(startFunction->GetBody()->Simplify(&startState));
	startFunction->GetBody()->ComputeType(&startState, startFunction);
	startFunction->SetBody(startFunction->GetBody()->Simplify(&startState));
	if (startState.HasErrors())
		return false;

	// Generate IL
	startFunction->GenerateIL(&startState);
	startFunction->ReportUndefinedLabels(&startState);
	if (startState.HasErrors())
		return false;

	// Remove all functions that aren't referenced
	Optimize optimize(this);
	optimize.RemoveUnreferencedSymbols();

	// Generate errors for undefined references
	size_t errors = 0;
	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
		(*i)->CheckForUndefinedReferences(errors);
	if (errors > 0)
		return false;

	// Perform analysis on the code and optimize using settings.  Be sure to reevaluate global
	// optimiziations if functions have changed.
	// IMPORTANT: The call to the optimizer must be made, even if optimization is disabled, so that
	// control and data flow analysis is performed (needed for code generation).  No actual optimization
	// will be done if optimization is disabled.
	bool changed = true;
	while (changed)
	{
		changed = false;
		optimize.PerformGlobalOptimizations();
		for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
		{
			if (optimize.OptimizeFunction(*i))
				changed = true;
		}
	}

#ifndef WIN32
	if (m_settings.internalDebug)
	{
		fprintf(stderr, "Functions:\n");
		for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
			(*i)->Print();
	}
#endif

	// Make string constants into global const character arrays
	map< string, Ref<Variable> > stringMap;
	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			(*j)->ConvertStringsToVariables(stringMap);
	}

	for (map< string, Ref<Variable> >::iterator i = stringMap.begin(); i != stringMap.end(); i++)
		m_variables.push_back(i->second);

	return true;
}


bool Linker::OutputCode(OutputBlock* finalBinary)
{
	// Create output classes for the requested architecture
	map<SubarchitectureType, Output*> out;
	if (m_settings.architecture == ARCH_X86)
	{
		out[SUBARCH_X86] = new OutputX86(m_settings);
		out[SUBARCH_X64] = new OutputX64(m_settings);
		if (m_settings.preferredBits == 32)
			out[SUBARCH_DEFAULT] = out[SUBARCH_X86];
		else
			out[SUBARCH_DEFAULT] = out[SUBARCH_X64];
	}
	else
	{
		fprintf(stderr, "error: invalid architecture\n");
		return false;
	}

	// Generate data section
	OutputBlock dataSection;
	dataSection.code = NULL;
	dataSection.len = 0;
	dataSection.maxLen = 0;

	// Lay out address space for data
	uint64_t addr = 0;
	for (vector< Ref<Variable> >::iterator i = m_variables.begin(); i != m_variables.end(); i++)
	{
		if (addr & ((*i)->GetType()->GetAlignment() - 1))
			addr += (*i)->GetType()->GetAlignment() - (addr & ((*i)->GetType()->GetAlignment() - 1));

		(*i)->SetDataSectionOffset(addr);

		dataSection.Write((*i)->GetData().code, (*i)->GetData().len);
		if ((*i)->GetData().len < (*i)->GetType()->GetWidth())
		{
			uint8_t zero = 0;
			for (size_t j = (*i)->GetData().len; j < (*i)->GetType()->GetWidth(); j++)
				dataSection.Write(&zero, 1);
		}

		addr += (*i)->GetType()->GetWidth();
	}

	// Generate list of IL blocks
	vector<ILBlock*> codeBlocks;
	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			codeBlocks.push_back(*j);
	}

	if (m_settings.polymorph)
	{
		// Polymorph enabled, randomize block ordering
		vector<ILBlock*> remaining = codeBlocks;

		// Ensure starting block is always at start (it is the entry point)
		codeBlocks.clear();
		codeBlocks.push_back(remaining[0]);
		remaining.erase(remaining.begin());

		while (remaining.size() > 0)
		{
			size_t choice = rand() % remaining.size();
			codeBlocks.push_back(remaining[choice]);
			remaining.erase(remaining.begin() + choice);
		}
	}

	// Ensure IL blocks have global indexes
	size_t globalBlockIndex = 0;
	for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
		(*i)->SetGlobalIndex(globalBlockIndex++);

	if (m_settings.mixedMode)
	{
		// Mixed mode enabled, choose random subarchitecture for any function that does not
		// have a subarchitecture explicitly defined.  Be sure to skip the _start function,
		// which has to be the default subarchitecture.
		for (vector< Ref<Function> >::iterator i = m_functions.begin() + 1; i != m_functions.end(); i++)
		{
			if ((*i)->GetSubarchitecture() != SUBARCH_DEFAULT)
				continue;

			if (m_settings.architecture == ARCH_X86)
			{
				if (rand() & 1)
					(*i)->SetSubarchitecture(SUBARCH_X86);
				else
					(*i)->SetSubarchitecture(SUBARCH_X64);
			}
		}
	}

	// Generate code for each block
	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
	{
		map<SubarchitectureType, Output*>::iterator j = out.find((*i)->GetSubarchitecture());
		if (j == out.end())
		{
			fprintf(stderr, "error: invalid subarchitecture in function '%s'\n",
				(*i)->GetName().c_str());
			return false;
		}

		if (!j->second->GenerateCode(*i))
			return false;
	}

	// Check relocations and ensure that everything is within bounds, and expand any references that are not
	while (true)
	{
		// Lay out address space for code
		addr = m_settings.base;
		for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
		{
			(*i)->SetAddress(addr);
			addr += (*i)->GetOutputBlock()->len;
		}

		m_settings.dataSectionBase = addr;
		if (m_settings.format == FORMAT_ELF)
			m_settings.dataSectionBase = AdjustDataSectionBaseForElfFile(m_settings.dataSectionBase);

		// Check relocations and gather the overflow list
		vector<RelocationReference> overflows;
		for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
		{
			if (!(*i)->CheckRelocations(m_settings.base, m_settings.dataSectionBase, overflows))
				return false;
		}

		if (overflows.size() == 0)
		{
			// All relocations are within limits, ready to finalize
			break;
		}

		// There are relocations that do not fit within the size allocated, need to call the overflow handlers
		for (vector<RelocationReference>::iterator i = overflows.begin(); i != overflows.end(); i++)
		{
			i->reloc->overflow(i->block, i->reloc->instruction, i->reloc->offset);

			if (i->reloc->type == CODE_RELOC_RELATIVE_8)
				i->reloc->type = CODE_RELOC_RELATIVE_32;
			else if (i->reloc->type == CODE_RELOC_BASE_RELATIVE_8)
				i->reloc->type = CODE_RELOC_BASE_RELATIVE_32;
			else if (i->reloc->type == DATA_RELOC_RELATIVE_8)
				i->reloc->type = DATA_RELOC_RELATIVE_32;
			else if (i->reloc->type == DATA_RELOC_BASE_RELATIVE_8)
				i->reloc->type = DATA_RELOC_BASE_RELATIVE_32;
		}
	}

	// Resolve relocations
	for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
	{
		if (!(*i)->ResolveRelocations(m_settings.base, m_settings.dataSectionBase))
			return false;
	}

	// Generate code section
	OutputBlock codeSection;
	codeSection.code = NULL;
	codeSection.len = 0;
	codeSection.maxLen = 0;

	for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
	{
		OutputBlock* block = (*i)->GetOutputBlock();
		memcpy(codeSection.PrepareWrite(block->len), block->code, block->len);
		codeSection.FinishWrite(block->len);
	}

	// Generate final binary
	switch (m_settings.format)
	{
	case FORMAT_BIN:
		memcpy(finalBinary->PrepareWrite(codeSection.len), codeSection.code, codeSection.len);
		finalBinary->FinishWrite(codeSection.len);
		memcpy(finalBinary->PrepareWrite(dataSection.len), dataSection.code, dataSection.len);
		finalBinary->FinishWrite(dataSection.len);
		break;
	case FORMAT_ELF:
		if (!GenerateElfFile(finalBinary, m_settings, &codeSection, &dataSection))
		{
			fprintf(stderr, "error: failed to output ELF format\n");
			return false;
		}
		break;
	default:
		fprintf(stderr, "error: unimplemented output format\n");
		return false;
	}

	return true;
}


bool Linker::WriteMapFile(const string& filename)
{
	FILE* outFP = fopen(filename.c_str(), "w");
	if (!outFP)
	{
		fprintf(stderr, "error: unable to open map file\n");
		return false;
	}

	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
	{
		if ((*i)->GetName().size() == 0)
			continue;
#ifdef WIN32
		fprintf(outFP, "%I64x %s\n", (unsigned long long)(*i)->GetIL()[0]->GetAddress(), (*i)->GetName().c_str());
#else
		fprintf(outFP, "%llx %s\n", (unsigned long long)(*i)->GetIL()[0]->GetAddress(), (*i)->GetName().c_str());
#endif
	}

	for (vector< Ref<Variable> >::iterator i = m_variables.begin(); i != m_variables.end(); i++)
	{
		if ((*i)->GetName().size() == 0)
			continue;
		if ((*i)->GetName()[0] == '$')
			continue;
#ifdef WIN32
		fprintf(outFP, "%I64x %s\n", (unsigned long long)(m_settings.dataSectionBase +
			(*i)->GetDataSectionOffset()), (*i)->GetName().c_str());
#else
		fprintf(outFP, "%llx %s\n", (unsigned long long)(m_settings.dataSectionBase +
			(*i)->GetDataSectionOffset()), (*i)->GetName().c_str());
#endif
	}

	fclose(outFP);
	return true;
}

