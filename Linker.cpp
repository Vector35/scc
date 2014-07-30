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

#ifdef WIN32
#define YY_NO_UNISTD_H
#endif
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
#include "MachOOutput.h"

using namespace std;
using namespace asmx86;


// Internal libraries
extern unsigned char Obj_x86_lib[];
extern unsigned int Obj_x86_lib_len;
extern unsigned char Obj_x64_lib[];
extern unsigned int Obj_x64_lib_len;
extern unsigned char Obj_quark_lib[];
extern unsigned int Obj_quark_lib_len;
extern unsigned char Obj_mips_lib[];
extern unsigned int Obj_mips_lib_len;
extern unsigned char Obj_mipsel_lib[];
extern unsigned int Obj_mipsel_lib_len;
extern unsigned char Obj_arm_lib[];
extern unsigned int Obj_arm_lib_len;
extern unsigned char Obj_armeb_lib[];
extern unsigned int Obj_armeb_lib_len;
extern unsigned char Obj_ppc_lib[];
extern unsigned int Obj_ppc_lib_len;
extern unsigned char Obj_ppcel_lib[];
extern unsigned int Obj_ppcel_lib_len;
extern unsigned char Obj_linux_x86_lib[];
extern unsigned int Obj_linux_x86_lib_len;
extern unsigned char Obj_linux_x64_lib[];
extern unsigned int Obj_linux_x64_lib_len;
extern unsigned char Obj_linux_quark_lib[];
extern unsigned int Obj_linux_quark_lib_len;
extern unsigned char Obj_linux_mips_lib[];
extern unsigned int Obj_linux_mips_lib_len;
extern unsigned char Obj_linux_mipsel_lib[];
extern unsigned int Obj_linux_mipsel_lib_len;
extern unsigned char Obj_linux_arm_lib[];
extern unsigned int Obj_linux_arm_lib_len;
extern unsigned char Obj_linux_armeb_lib[];
extern unsigned int Obj_linux_armeb_lib_len;
extern unsigned char Obj_linux_ppc_lib[];
extern unsigned int Obj_linux_ppc_lib_len;
extern unsigned char Obj_linux_ppcel_lib[];
extern unsigned int Obj_linux_ppcel_lib_len;
extern unsigned char Obj_freebsd_x86_lib[];
extern unsigned int Obj_freebsd_x86_lib_len;
extern unsigned char Obj_freebsd_x64_lib[];
extern unsigned int Obj_freebsd_x64_lib_len;
extern unsigned char Obj_freebsd_quark_lib[];
extern unsigned int Obj_freebsd_quark_lib_len;
extern unsigned char Obj_mac_x86_lib[];
extern unsigned int Obj_mac_x86_lib_len;
extern unsigned char Obj_mac_x64_lib[];
extern unsigned int Obj_mac_x64_lib_len;
extern unsigned char Obj_mac_quark_lib[];
extern unsigned int Obj_mac_quark_lib_len;
extern unsigned char Obj_windows_x86_lib[];
extern unsigned int Obj_windows_x86_lib_len;
extern unsigned char Obj_windows_x64_lib[];
extern unsigned int Obj_windows_x64_lib_len;
extern unsigned char Obj_windows_quark_lib[];
extern unsigned int Obj_windows_quark_lib_len;
extern unsigned char Obj_windows_arm_lib[];
extern unsigned int Obj_windows_arm_lib_len;


extern int Code_parse(ParserState* state);
extern void Code_set_lineno(int line, void* yyscanner);


extern Output* CreateQuarkCodeGen(const Settings& settings, Function* startFunc);
extern Output* CreateMipsCodeGen(const Settings& settings, Function* startFunc);
extern Output* CreateArmCodeGen(const Settings& settings, Function* startFunc);
extern Output* CreatePpcCodeGen(const Settings& settings, Function* startFunc);


Linker::Linker(const Settings& settings): m_settings(settings), m_precompiledPreprocess("precompiled headers", NULL, settings),
	m_precompileState(settings, "precompiled headers", NULL), m_initExpression(new Expr(EXPR_SEQUENCE))
{
	m_markovReady = false;
}


Linker::~Linker()
{
}


size_t Linker::AddInstructionToMarkovChain(uint16_t& prev, uint8_t* data, size_t len)
{
	Instruction instr;
	if (m_settings.preferredBits == 32)
	{
		if (!Disassemble32(data, 0, len, &instr))
			return 1;
	}
	else
	{
		if (!Disassemble64(data, 0, len, &instr))
			return 1;
	}

	if (instr.length == 0)
		return 1;

	// Skip instructions that don't satisfy the blacklist
	bool ok = true;
	for (size_t j = 0; j < instr.length; j++)
	{
		for (vector<uint8_t>::iterator k = m_settings.blacklist.begin(); k != m_settings.blacklist.end(); k++)
		{
			if (data[j] == *k)
			{
				ok = false;
				break;
			}
		}

		if (!ok)
			break;
	}

	if (ok)
	{
		m_markovChain[prev][string((char*)data, instr.length)]++;

		// Insert all instructions into slot 0xffff (invalid for X86), this will be what is used when inserting
		// an instruction from a fresh state (or one which has no valid transitions)
		m_markovChain[0xffff][string((char*)data, instr.length)]++;

		if (instr.length == 1)
			prev = data[0];
		else
			prev = ((uint16_t)data[0]) | ((uint16_t)data[1] << 8);
	}

	return instr.length;
}


void Linker::PrepareMarkovInstructionsFromFile(const string& filename)
{
	// Markov chain generation only supported on x86 for now
	if (m_settings.architecture != ARCH_X86)
		return;

	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return;
	fseek(fp, 0, SEEK_END);
	size_t len = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	uint8_t* data = new uint8_t[len];
	if (fread(data, len, 1, fp) <= 0)
	{
		fclose(fp);
		return;
	}

	fclose(fp);

	uint16_t prev = 0x90;
	for (size_t i = 0; i < len; )
	{
		size_t maxLen = len - i;
		if (maxLen > 15)
			maxLen = 15;

		i += AddInstructionToMarkovChain(prev, &data[i], maxLen);
	}

	m_markovReady = true;
}


void Linker::PrepareMarkovInstructionsFromBlocks(const vector<ILBlock*>& codeBlocks)
{
	// Markov chain generation only supported on x86 for now
	if (m_settings.architecture != ARCH_X86)
		return;

	uint16_t prev = 0x90;
	for (vector<ILBlock*>::const_iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
	{
		for (size_t j = 0; j < (*i)->GetOutputBlock()->len; j++)
		{
			size_t maxLen = (*i)->GetOutputBlock()->len - j;
			if (maxLen > 15)
				maxLen = 15;

			j += AddInstructionToMarkovChain(prev, &((uint8_t*)(*i)->GetOutputBlock()->code)[j], maxLen);
		}
	}

	m_markovReady = true;
}


void Linker::InsertMarkovInstructions(OutputBlock* block, size_t len)
{
	uint16_t prev = 0xffff;
	while (len > 0)
	{
		size_t total = 0;
		for (map<string, size_t>::iterator i = m_markovChain[prev].begin(); i != m_markovChain[prev].end(); i++)
			total += i->second;

		if (total == 0)
		{
			if (prev == 0xffff)
			{
				// No valid starting instructions, just generate random data
				vector<uint8_t> available;
				for (size_t i = 0; i < 256; i++)
				{
					bool ok = true;
					for (vector<uint8_t>::iterator j = m_settings.blacklist.begin(); j != m_settings.blacklist.end(); j++)
					{
						if (i == *j)
						{
							ok = false;
							break;
						}
					}

					if (ok)
						available.push_back((uint8_t)i);
				}

				for (size_t i = 0; i < len; i++)
				{
					uint8_t choice = available[rand() % available.size()];
					*(uint8_t*)block->PrepareWrite(1) = choice;
					block->FinishWrite(1);
				}

				return;
			}

			// No valid transition states, start from the top
			prev = 0xffff;
			continue;
		}

		// Pick a random instruction, ensuring that the weighting is correct
		size_t cur = 0;
		size_t pick = rand() % total;
		string instruction;
		for (map<string, size_t>::iterator i = m_markovChain[prev].begin(); i != m_markovChain[prev].end(); i++)
		{
			cur += i->second;
			if (cur > pick)
			{
				instruction = i->first;
				break;
			}
		}

		if (instruction.size() == 1)
			prev = instruction[0];
		else
			prev = ((uint16_t)instruction[0]) | ((uint16_t)instruction[1] << 8);

		memcpy(block->PrepareWrite(instruction.size()), instruction.c_str(), instruction.size());
		block->FinishWrite(instruction.size());

		if (instruction.size() > len)
			break;
		len -= instruction.size();
	}
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
	unsigned char* lib = NULL;
	unsigned int len = 0;

	if (m_settings.architecture == ARCH_X86)
	{
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
		case OS_MAC:
			if (m_settings.preferredBits == 32)
			{
				lib = Obj_mac_x86_lib;
				len = Obj_mac_x86_lib_len;
			}
			else
			{
				lib = Obj_mac_x64_lib;
				len = Obj_mac_x64_lib_len;
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
	}
	else if (m_settings.architecture == ARCH_QUARK)
	{
		switch (m_settings.os)
		{
		case OS_LINUX:
			lib = Obj_linux_quark_lib;
			len = Obj_linux_quark_lib_len;
			break;
		case OS_FREEBSD:
			lib = Obj_freebsd_quark_lib;
			len = Obj_freebsd_quark_lib_len;
			break;
		case OS_MAC:
			lib = Obj_mac_quark_lib;
			len = Obj_mac_quark_lib_len;
			break;
		case OS_WINDOWS:
			lib = Obj_windows_quark_lib;
			len = Obj_windows_quark_lib_len;
			break;
		default:
			lib = Obj_quark_lib;
			len = Obj_quark_lib_len;
			break;
		}
	}
	else if (m_settings.architecture == ARCH_MIPS)
	{
		if (m_settings.bigEndian)
		{
			switch (m_settings.os)
			{
			case OS_LINUX:
				lib = Obj_linux_mips_lib;
				len = Obj_linux_mips_lib_len;
				break;
			default:
				lib = Obj_mips_lib;
				len = Obj_mips_lib_len;
				break;
			}
		}
		else
		{
			switch (m_settings.os)
			{
			case OS_LINUX:
				lib = Obj_linux_mipsel_lib;
				len = Obj_linux_mipsel_lib_len;
				break;
			default:
				lib = Obj_mipsel_lib;
				len = Obj_mipsel_lib_len;
				break;
			}
		}
	}
	else if (m_settings.architecture == ARCH_ARM)
	{
		if (m_settings.bigEndian)
		{
			switch (m_settings.os)
			{
			case OS_LINUX:
				lib = Obj_linux_armeb_lib;
				len = Obj_linux_armeb_lib_len;
				break;
			default:
				lib = Obj_armeb_lib;
				len = Obj_armeb_lib_len;
				break;
			}
		}
		else
		{
			switch (m_settings.os)
			{
			case OS_LINUX:
				lib = Obj_linux_arm_lib;
				len = Obj_linux_arm_lib_len;
				break;
			case OS_WINDOWS:
				lib = Obj_windows_arm_lib;
				len = Obj_windows_arm_lib_len;
				break;
			default:
				lib = Obj_arm_lib;
				len = Obj_arm_lib_len;
				break;
			}
		}
	}
	else if (m_settings.architecture == ARCH_PPC)
	{
		if (m_settings.bigEndian)
		{
			switch (m_settings.os)
			{
			case OS_LINUX:
				lib = Obj_linux_ppc_lib;
				len = Obj_linux_ppc_lib_len;
				break;
			default:
				lib = Obj_ppc_lib;
				len = Obj_ppc_lib_len;
				break;
			}
		}
		else
		{
			switch (m_settings.os)
			{
			case OS_LINUX:
				lib = Obj_linux_ppcel_lib;
				len = Obj_linux_ppcel_lib_len;
				break;
			default:
				lib = Obj_ppcel_lib;
				len = Obj_ppcel_lib_len;
				break;
			}
		}
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


bool Linker::PrecompileSource(const string& source)
{
	m_precompiledPreprocess.IncludeSource(source);
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
	if (!PreprocessState::PreprocessSource(m_settings, source, filename, preprocessed, &m_precompiledPreprocess))
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
		i->second->GetBody()->ComputeType(&parser, i->second);
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
							prev->GetLocation().fileName.c_str(),
							prev->GetLocation().lineNumber,
							prev->GetName().c_str());
						fprintf(stderr, "%s:%d: prototype definition of '%s'\n",
							i->second->GetLocation().fileName.c_str(),
							i->second->GetLocation().lineNumber,
							i->second->GetName().c_str());
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
		return false;

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

	m_startFunction = new Function(startInfo, false);
	m_startFunction->SetVariables(paramVars);
	m_functions.insert(m_functions.begin(), m_startFunction);
	m_functionsByName["_start"] = m_startFunction;

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
	m_startFunction->SetBody(startBody);

	// First, propogate type information
	ParserState startState(m_settings, "_start", NULL);
	m_startFunction->SetBody(m_startFunction->GetBody()->Simplify(&startState));
	m_startFunction->GetBody()->ComputeType(&startState, m_startFunction);
	m_startFunction->SetBody(m_startFunction->GetBody()->Simplify(&startState));
	if (startState.HasErrors())
		return false;

	// Generate IL
	m_startFunction->GenerateIL(&startState);
	m_startFunction->ReportUndefinedLabels(&startState);
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

	// Make string constants into global const character arrays
	map< string, Ref<Variable> > stringMap;
	for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			(*j)->ConvertStringsToVariables(stringMap);
	}

	for (map< string, Ref<Variable> >::iterator i = stringMap.begin(); i != stringMap.end(); i++)
		m_variables.push_back(i->second);

#ifndef WIN32
	if (m_settings.internalDebug)
	{
		fprintf(stderr, "Functions:\n");
		for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
			(*i)->Print();
	}
#endif

	return true;
}


bool Linker::LayoutCode(vector<ILBlock*>& codeBlocks)
{
	// Check relocations and ensure that everything is within bounds, and expand any references that are not
	while (true)
	{
		// Lay out address space for code
		uint64_t addr = m_settings.base;
		for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
		{
			(*i)->SetAddress(addr);
			addr += (*i)->GetOutputBlock()->len;
		}
		if ((m_settings.alignment > 1) && ((addr % m_settings.alignment) != 0))
			addr += m_settings.alignment - (addr % m_settings.alignment);

		m_settings.dataSectionBase = addr;
		if (m_settings.format == FORMAT_ELF)
			m_settings.dataSectionBase = AdjustDataSectionBaseForElfFile(m_settings.dataSectionBase);
		else if (m_settings.format == FORMAT_MACHO)
			m_settings.dataSectionBase = AdjustDataSectionBaseForMachOFile(m_settings.dataSectionBase);

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
			if (!i->reloc->overflow)
			{
				fprintf(stderr, "error: relocation out of range\n");
				return false;
			}
			i->reloc->overflow(i->block, *i->reloc);
		}
	}

	return true;
}


bool Linker::OutputCode(OutputBlock* finalBinary)
{
	// Create output classes for the requested architecture
	map<SubarchitectureType, Output*> out;
	if (m_settings.architecture == ARCH_X86)
	{
		out[SUBARCH_X86] = new OutputX86(m_settings, m_startFunction);
		out[SUBARCH_X64] = new OutputX64(m_settings, m_startFunction);
		if (m_settings.preferredBits == 32)
			out[SUBARCH_DEFAULT] = out[SUBARCH_X86];
		else
			out[SUBARCH_DEFAULT] = out[SUBARCH_X64];
	}
	else if (m_settings.architecture == ARCH_QUARK)
	{
		out[SUBARCH_DEFAULT] = CreateQuarkCodeGen(m_settings, m_startFunction);
	}
	else if (m_settings.architecture == ARCH_MIPS)
	{
		out[SUBARCH_DEFAULT] = CreateMipsCodeGen(m_settings, m_startFunction);
	}
	else if (m_settings.architecture == ARCH_ARM)
	{
		out[SUBARCH_DEFAULT] = CreateArmCodeGen(m_settings, m_startFunction);
	}
	else if (m_settings.architecture == ARCH_PPC)
	{
		out[SUBARCH_DEFAULT] = CreatePpcCodeGen(m_settings, m_startFunction);
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
	dataSection.bigEndian = m_settings.bigEndian;

	// Lay out address space for data
	uint64_t addr = 0;
	for (vector< Ref<Variable> >::iterator i = m_variables.begin(); i != m_variables.end(); i++)
	{
		if (addr & ((*i)->GetType()->GetAlignment() - 1))
		{
			size_t padding = (size_t)((*i)->GetType()->GetAlignment() - (addr & ((*i)->GetType()->GetAlignment() - 1)));
			uint8_t zero = 0;
			addr += padding;
			for (size_t j = 0; j < padding; j++)
				dataSection.Write(&zero, 1);
		}

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

	if ((m_settings.alignment > 1) && ((addr % m_settings.alignment) != 0))
	{
		// Pad data section with random bytes (respecting blacklist)
		size_t alignSize = (size_t)(m_settings.alignment - (addr % m_settings.alignment));
		addr += alignSize;

		if (m_settings.polymorph || (m_settings.blacklist.size() > 0))
		{
			vector<uint8_t> available;
			for (size_t i = 0; i < 256; i++)
			{
				bool ok = true;
				for (vector<uint8_t>::iterator j = m_settings.blacklist.begin(); j != m_settings.blacklist.end(); j++)
				{
					if (i == *j)
					{
						ok = false;
						break;
					}
				}

				if (ok)
					available.push_back((uint8_t)i);
			}

			for (size_t i = 0; i < alignSize; i++)
			{
				uint8_t choice = available[rand() % available.size()];
				*(uint8_t*)dataSection.PrepareWrite(1) = choice;
				dataSection.FinishWrite(1);
			}
		}
		else
		{
			uint8_t zero = 0;
			for (size_t i = 0; i < alignSize; i++)
				dataSection.Write(&zero, 1);
		}
	}

	if (m_variablesByName.find("__end") != m_variablesByName.end())
		m_variablesByName["__end"]->SetDataSectionOffset(addr);

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
		{
			fprintf(stderr, "error: code generation failed for function '%s'\n",
				(*i)->GetName().c_str());
			return false;
		}
	}

	// Perform address space layout of code section, also handling overflows in relocations
	if (!LayoutCode(codeBlocks))
		return false;

	if (m_settings.pad)
	{
		if ((!m_markovReady) && m_settings.markovChains && (m_settings.markovFile.size() != 0))
		{
			// Initialize the markov chain instruction generator with file contents
			PrepareMarkovInstructionsFromFile(m_settings.markovFile);
		}
		else if ((!m_markovReady) && m_settings.markovChains)
		{
			// Initialize the markov chain instruction generator with existing code blocks
			PrepareMarkovInstructionsFromBlocks(codeBlocks);
		}

		// Padding is enabled, insert random code in between blocks to get the code closer to the target size
		while (true)
		{
			size_t totalSize = dataSection.len;
			for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
				totalSize += (*i)->GetOutputBlock()->len;

			ssize_t remaining = m_settings.maxLength - totalSize;
			if (remaining < 0)
			{
				// Oops, added too many bytes in a previous loop, need to remove some of the random bytes
				for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
				{
					if ((*i)->GetOutputBlock()->randomLen > 0)
					{
						remaining += (*i)->GetOutputBlock()->randomLen;
						(*i)->GetOutputBlock()->len -= (*i)->GetOutputBlock()->randomLen;
						(*i)->GetOutputBlock()->randomLen = 0;
					}

					if (remaining >= 0)
						break;
				}
				break;
			}

			// Don't try to add more bytes if there isn't much room left
			if (remaining < 32)
				break;

			remaining /= 2;
			while (remaining > 0)
			{
				ssize_t insertSize = rand() & 31;
				if (insertSize > remaining)
					insertSize = remaining;

				OutputBlock* block = codeBlocks[rand() % codeBlocks.size()]->GetOutputBlock();

				if (m_markovReady)
				{
					// Insert random valid instructions
					InsertMarkovInstructions(block, insertSize);
				}
				else
				{
					// Pad block with random bytes (respecting blacklist)
					vector<uint8_t> available;
					for (size_t i = 0; i < 256; i++)
					{
						bool ok = true;
						for (vector<uint8_t>::iterator j = m_settings.blacklist.begin(); j != m_settings.blacklist.end(); j++)
						{
							if (i == *j)
							{
								ok = false;
								break;
							}
						}

						if (ok)
							available.push_back((uint8_t)i);
					}

					for (ssize_t i = 0; i < insertSize; i++)
					{
						uint8_t choice = available[rand() % available.size()];
						*(uint8_t*)block->PrepareWrite(1) = choice;
						block->FinishWrite(1);
					}
				}

				block->randomLen += insertSize;
				remaining -= insertSize;
			}

			// Need to update layout of code section, and resolve any new overflows in relocations
			if (!LayoutCode(codeBlocks))
				return false;
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
	codeSection.bigEndian = m_settings.bigEndian;

	for (vector<ILBlock*>::iterator i = codeBlocks.begin(); i != codeBlocks.end(); i++)
	{
		OutputBlock* block = (*i)->GetOutputBlock();
		memcpy(codeSection.PrepareWrite(block->len), block->code, block->len);
		codeSection.FinishWrite(block->len);
	}

	if (m_settings.sizeInfo)
	{
		for (vector< Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
		{
			size_t size = 0;
			for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
				size += (*j)->GetOutputBlock()->len;

			fprintf(stderr, "%-32s  %d bytes\n", (*i)->GetName().c_str(), (int)size);
		}

		fprintf(stderr, "%-32s  %d bytes\n", "data section", (int)dataSection.len);
	}

	if ((m_settings.alignment > 1) && ((codeSection.len % m_settings.alignment) != 0))
	{
		// Pad code section with random bytes (respecting blacklist)
		size_t alignSize = (size_t)(m_settings.alignment - (codeSection.len % m_settings.alignment));

		if (m_settings.polymorph || (m_settings.blacklist.size() > 0))
		{
			vector<uint8_t> available;
			for (size_t i = 0; i < 256; i++)
			{
				bool ok = true;
				for (vector<uint8_t>::iterator j = m_settings.blacklist.begin(); j != m_settings.blacklist.end(); j++)
				{
					if (i == *j)
					{
						ok = false;
						break;
					}
				}

				if (ok)
					available.push_back((uint8_t)i);
			}

			for (size_t i = 0; i < alignSize; i++)
			{
				uint8_t choice = available[rand() % available.size()];
				*(uint8_t*)codeSection.PrepareWrite(1) = choice;
				codeSection.FinishWrite(1);
			}
		}
		else
		{
			uint8_t zero = 0;
			for (size_t i = 0; i < alignSize; i++)
				codeSection.Write(&zero, 1);
		}
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
	case FORMAT_MACHO:
		if (!GenerateMachOFile(finalBinary, m_settings, &codeSection, &dataSection))
		{
			fprintf(stderr, "error: failed to output Mach-O format\n");
			return false;
		}
		break;
	default:
		fprintf(stderr, "error: unimplemented output format\n");
		return false;
	}

	// Verify blacklist constraints
	if (m_settings.blacklist.size() != 0)
	{
		for (size_t i = 0; i < finalBinary->len; i++)
		{
			bool valid = true;
			uint8_t errorByte = 0;
			for (vector<uint8_t>::iterator j = m_settings.blacklist.begin(); j != m_settings.blacklist.end(); j++)
			{
				if (((uint8_t*)finalBinary->code)[i] == *j)
				{
					errorByte = *j;
					valid = false;
					break;
				}
			}

			if (!valid)
			{
				fprintf(stderr, "error: unable to satisfy constraints (output contains 0x%.2x)\n", errorByte);
				return false;
			}
		}
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

