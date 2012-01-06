#include "asmx86.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <sys/mman.h>
#include "ParserState.h"
#include "CodeParser.h"
#include "CodeLexer.h"
#include "OutputX86.h"
#include "OutputX64.h"

using namespace std;


enum Architecture
{
	ARCH_X86
};

enum OperatingSystem
{
	OS_NONE,
	OS_LINUX,
	OS_FREEBSD,
	OS_MACH,
	OS_WINDOWS
};

enum OutputFormat
{
	FORMAT_BIN,
	FORMAT_ELF,
	FORMAT_PE
};

enum OptimizationLevel
{
	OPTIMIZE_DISABLE,
	OPTIMIZE_SIZE,
	OPTIMIZE_NORMAL
};


extern int Code_parse(ParserState* state);
extern void Code_set_lineno(int line, void* yyscanner);


void Usage()
{
	fprintf(stderr, "scc [options] <input files> [...]\n\n");
	fprintf(stderr, "Shellcode Compiler\n");
	fprintf(stderr, "Copyright (C) 2011-2012 Rusty Wagner\n");
	fprintf(stderr, "EARLY PREVIEW RELEASE - NOT ALL OPTIONS ARE IMPLEMENTED\n\n");
	fprintf(stderr, "This software is released under the terms of the MIT open source license.\n\n");
	fprintf(stderr, "This compiler accepts a subset of C99 syntax, with extensions for creating a standalone\n");
	fprintf(stderr, "environment for writing shellcode.  Many standard system calls and C library functions\n");
	fprintf(stderr, "are automatically available without the need for include files.\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "    --arch <value>                    Specify processor architecture\n");
	fprintf(stderr, "                                      Can be: x86 (default), x64\n");
	fprintf(stderr, "    --allow-return                    Allow return from shellcode (default is to exit)\n");
	fprintf(stderr, "    --assume-safe-stack               Assume the stack pointer is not near the code\n");
	fprintf(stderr, "    --base <expr>                     Set base address of output (can be a runtime computed\n");
	fprintf(stderr, "                                      expression, such as \"[eax+8]-12\")\n");
	fprintf(stderr, "    --blacklist <byte>                Blacklist the given byte value\n");
	fprintf(stderr, "    --concat                          Jump to end of output on return for concatenating code\n");
	fprintf(stderr, "    --decoder <source>                Use decoder to decode shellcode before executing\n");
	fprintf(stderr, "    --encode-pointers                 All code pointers are encoded with a random canary\n");
	fprintf(stderr, "    --encoder <source>                Use encoder to encode shellcode\n");
	fprintf(stderr, "    --exec                            Execute shellcode after generation (does not write\n");
	fprintf(stderr, "                                      output to a file)\n");
	fprintf(stderr, "    --format <value>, -f <value>      Specify output format\n");
	fprintf(stderr, "                                      Can be: bin (default), lib, elf, pe\n");
	fprintf(stderr, "    --internal-debug                  Enable internal debugging output\n");
	fprintf(stderr, "    -L <lib>                          Include pre-built library\n");
	fprintf(stderr, "    -m32, -m64                        Specify target address size\n");
	fprintf(stderr, "    --max-length <value>              Do not let output size exceed given number of bytes\n");
	fprintf(stderr, "    -o <filename>                     Set output filename (default is hex dump to stdout)\n");
	fprintf(stderr, "    -O0                               Do not run the optimizer\n");
	fprintf(stderr, "    -Os                               Try to generate the smallest code possible\n");
	fprintf(stderr, "    --pad                             Pad output to be exactly the maximum length\n");
	fprintf(stderr, "    --platform <value>                Specify operating system\n");
	fprintf(stderr, "                                      Can be: linux (default), freebsd, mach, windows, none\n");
	fprintf(stderr, "    --polymorph                       Generate different code on each run (disabled with -Os)\n");
	fprintf(stderr, "    --preserve <reg>                  Preserve the value of the given register\n");
	fprintf(stderr, "    --seed <value>                    Specify random seed (to reproduce --polymorph runs)\n");
	fprintf(stderr, "    --shared                          Generate shared library instead of executable\n");
	fprintf(stderr, "    --stdin                           Read source code from stdin\n");
	fprintf(stderr, "    --stdout                          Send generated code to stdout for pipelines\n\n");
	fprintf(stderr, "Useful extensions:\n");
	fprintf(stderr, "    __noreturn                        Specifies that a function cannot return\n");
	fprintf(stderr, "                                      Example: void exit(int value) __noreturn;\n");
	fprintf(stderr, "    __syscall(num, ...)               Executes a system call on the target platform\n");
	fprintf(stderr, "    __undefined                       Gives undefined results, usually omitting code\n");
	fprintf(stderr, "                                      Example: exit(__undefined);\n");
	fprintf(stderr, "    __initial_<reg>                   Value of register at start of program\n");
	fprintf(stderr, "                                      Example: int socketDescriptor = __initial_ebx;\n\n");
}


int main(int argc, char* argv[])
{
	vector<string> sourceFiles;
	vector<string> libraries;
	vector<uint8_t> blacklist;
	vector<string> preservedRegs;
	string outputFile = "";
	bool hexOutput = true;
	Architecture architecture = ARCH_X86;
	OperatingSystem os = OS_LINUX;
	OutputFormat format = FORMAT_BIN;
	OptimizationLevel optimization = OPTIMIZE_NORMAL;
	uint32_t preferredBits = 32;
	bool architectureIsExplicit = false;
	bool osIsExplicit = false;
	bool allowReturn = false;
	bool assumeSafeStack = false;
	bool concat = false;
	string decoder, encoder;
	bool encodePointers = false;
	bool execute = false;
	bool internalDebug = false;
	uint32_t maxLength = 0;
	bool pad = false;
	bool polymorph = false;
	bool useSpecificSeed = false;
	uint32_t seed = 0;
	bool sharedLibrary = false;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--arch"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			if ((!strcmp(argv[i], "x86")) || (!strcmp(argv[i], "i386")))
			{
				architecture = ARCH_X86;
				preferredBits = 32;
			}
			else if ((!strcmp(argv[i], "x64")) || (!strcmp(argv[i], "x86_64")) || (!strcmp(argv[i], "amd64")))
			{
				architecture = ARCH_X86;
				preferredBits = 64;
			}
			else
			{
				fprintf(stderr, "error: unsupported architecture '%s'\n", argv[i]);
			}

			architectureIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "-m32"))
		{
			preferredBits = 32;
			architectureIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "-m64"))
		{
			preferredBits = 64;
			architectureIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "--allow-return"))
		{
			allowReturn = true;
			continue;
		}
		else if (!strcmp(argv[i], "--assume-safe-stack"))
		{
			assumeSafeStack = true;
			continue;
		}
		else if (!strcmp(argv[i], "--concat"))
		{
			concat = true;
			continue;
		}
		else if (!strcmp(argv[i], "--blacklist"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			blacklist.push_back((uint8_t)strtoul(argv[i], NULL, 0));
			continue;
		}
		else if (!strcmp(argv[i], "--decoder"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			decoder = argv[i];
			continue;
		}
		else if (!strcmp(argv[i], "--encoder"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			encoder = argv[i];
			continue;
		}
		else if (!strcmp(argv[i], "--encode-pointers"))
		{
			encodePointers = true;
			continue;
		}
		else if (!strcmp(argv[i], "--exec"))
		{
#ifdef __x86_64
			if (!architectureIsExplicit)
				preferredBits = 64;
#endif

			if (!osIsExplicit)
			{
				// Use current OS
#ifdef __APPLE__
				os = OS_MACH;
#elif defined(WIN32)
				os = OS_WINDOWS;
#elif defined(linux)
				os = OS_LINUX;
#else
				os = OS_FREEBSD;
#endif
			}

			execute = true;
			continue;
		}
		else if ((!strcmp(argv[i], "--format")) || (!strcmp(argv[i], "-f")))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			if (!strcmp(argv[i], "bin"))
				format = FORMAT_BIN;
			else if (!strcmp(argv[i], "elf"))
				format = FORMAT_ELF;
			else if (!strcmp(argv[i], "pe"))
				format = FORMAT_PE;
			else
			{
				fprintf(stderr, "error: unsupported format '%s'\n", argv[i]);
			}

			continue;
		}
		else if (!strcmp(argv[i], "--help"))
		{
			Usage();
			return 0;
		}
		else if (!strcmp(argv[i], "--internal-debug"))
		{
			internalDebug = true;
			continue;
		}
		else if (!strcmp(argv[i], "-L"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			libraries.push_back(argv[i]);
			continue;
		}
		else if (!strcmp(argv[i], "--max-length"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			maxLength = strtoul(argv[i], NULL, 0);
			continue;
		}
		else if (!strcmp(argv[i], "--pad"))
		{
			pad = true;
			continue;
		}
		else if (!strcmp(argv[i] ,"-o"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			outputFile = argv[i];
			hexOutput = false;
			continue;
		}
		else if (!strcmp(argv[i], "-O0"))
		{
			optimization = OPTIMIZE_DISABLE;
			continue;
		}
		else if (!strcmp(argv[i], "-Os"))
		{
			optimization = OPTIMIZE_SIZE;
			continue;
		}
		else if (!strcmp(argv[i], "--platform"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			if (!strcmp(argv[i], "linux"))
				os = OS_LINUX;
			else if (!strcmp(argv[i], "freebsd"))
				os = OS_FREEBSD;
			else if ((!strcmp(argv[i], "mach")) || (!strcmp(argv[i], "macos")) || (!strcmp(argv[i], "macosx")))
				os = OS_MACH;
			else if ((!strcmp(argv[i], "win32")) || (!strcmp(argv[i], "windows")))
				os = OS_WINDOWS;
			else
			{
				fprintf(stderr, "error: unsupported platform '%s'\n", argv[i]);
			}

			osIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "--polymorph"))
		{
			polymorph = true;
			continue;
		}
		else if (!strcmp(argv[i], "--preserve"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			preservedRegs.push_back(argv[i]);
			continue;
		}
		else if (!strcmp(argv[i], "--seed"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			useSpecificSeed = true;
			seed = (uint32_t)strtoul(argv[i], NULL, 0);
			continue;
		}
		else if (!strcmp(argv[i], "--shared"))
		{
			sharedLibrary = true;
			continue;
		}
		else if (!strcmp(argv[i], "--stdin"))
		{
			sourceFiles.push_back("");
			continue;
		}
		else if (!strcmp(argv[i], "--stdout"))
		{
			outputFile = "";
			hexOutput = false;
			continue;
		}
		else if (argv[i][0] == '-')
		{
			fprintf(stderr, "error: unrecognized option '%s'\n", argv[i]);
			return 1;
		}

		sourceFiles.push_back(argv[i]);
	}

	if (sourceFiles.size() == 0)
	{
		fprintf(stderr, "no input files\n");
		return 1;
	}

	// Initialize random seed if one is needed
	if (polymorph || pad)
	{
		if (!useSpecificSeed)
		{
			FILE* fp = fopen("/dev/urandom", "rb");
			fread(&seed, sizeof(seed), 1, fp);
			fclose(fp);
			printf("Seed is %u\n", seed);
		}

		srand(seed);
	}

	// Warn about incompatible options
	if ((optimization == OPTIMIZE_SIZE) && polymorph)
	{
		fprintf(stderr, "warning: polymorphic code generation not compatible with size optimization\n");
		polymorph = false;
	}

	if (sharedLibrary && (format == FORMAT_BIN))
	{
		fprintf(stderr, "warning: trying to generate shared library in raw binary output mode\n");
		sharedLibrary = false;
	}

	if ((maxLength != 0) && (format != FORMAT_BIN))
	{
		fprintf(stderr, "warning: maximum size only supported in raw binary output mode\n");
		maxLength = 0;
	}

	if ((blacklist.size() > 0) && (format != FORMAT_BIN))
	{
		fprintf(stderr, "warning: blacklist only supported in raw binary output mode\n");
		blacklist.clear();
	}

	if ((encoder.size() > 0) && (decoder.size() == 0))
	{
		fprintf(stderr, "error: decoder must be specified if an encoder is provided\n");
		return 1;
	}

	// Normal executables always have a normal stack
	if (format != FORMAT_BIN)
		assumeSafeStack = true;

	// Set pointer size
	if (preferredBits == 32)
		SetTargetPointerSize(4);
	else
		SetTargetPointerSize(8);

	// Start parsing source files
	vector< Ref<Function> > functions;
	map< string, Ref<Function> > functionsByName;
	vector< Ref<Variable> > variables;
	map< string, Ref<Variable> > variablesByName;
	Ref<Expr> initExpression = new Expr(EXPR_SEQUENCE);

	for (vector<string>::iterator i = sourceFiles.begin(); i != sourceFiles.end(); i++)
	{
		char* data;
		long size;
		if (i->size() == 0)
		{
			// Read source over stdin
			long max = 512;
			size = 0;
			data = new char[max + 1];
			while (!feof(stdin))
			{
				char ch;
				if (!fread(&ch, 1, 1, stdin))
					break;

				if (size >= max)
				{
					max = (size + 1) * 2;
					char* newData = new char[max + 1];
					memcpy(newData, data, size);
					if (data)
						delete[] data;
					data = newData;
				}

				data[size++] = ch;
			}

			data[size] = 0;
		}
		else
		{
			FILE* fp = fopen(i->c_str(), "r");
			if (!fp)
			{
				fprintf(stderr, "%s: error: file not found\n", i->c_str());
				return 1;
			}

			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			data = new char[size + 1];
			fread(data, 1, size, fp);
			data[size] = 0;
			fclose(fp);

			fprintf(stderr, "Parsing %s...\n", i->c_str());
		}

		yyscan_t scanner;
		Code_lex_init(&scanner);
		ParserState parser((i->size() == 0) ? "stdin" : i->c_str(), scanner);

		YY_BUFFER_STATE buf = Code__scan_string(data, scanner);
		Code__switch_to_buffer(buf, scanner);
		Code_set_lineno(1, scanner);

		bool ok = true;
		if (Code_parse(&parser) != 0)
			ok = false;
		if (parser.HasErrors())
			ok = false;

		Code_lex_destroy(scanner);
		delete[] data;

		if (!ok)
			return 1;

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
			return 1;

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
			return 1;

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
					functions.push_back(i->second);
				}
				else
				{
					// Funciton is in global scope
					if (functionsByName.find(i->second->GetName()) != functionsByName.end())
					{
						// Function by this name already defined in another file
						Function* prev = functionsByName[i->second->GetName()];
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
								prev->GetCallingConvention(), params))
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
							for (vector< Ref<Function> >::iterator j = functions.begin();
								j != functions.end(); j++)
								(*j)->ReplaceFunction(prev, i->second);
							initExpression->ReplaceFunction(prev, i->second);
						}
					}

					functions.push_back(i->second);
					functionsByName[i->second->GetName()] = i->second;
				}
			}
			else
			{
				// Function is a prototype only, ignore local scope
				if (!i->second->IsLocalScope())
				{
					if (functionsByName.find(i->second->GetName()) != functionsByName.end())
					{
						// Function by this name already defined in another file
						Function* prev = functionsByName[i->second->GetName()];

						// Check for compatibility
						vector< pair< Ref<Type>, string> > params;
						for (vector<FunctionParameter>::const_iterator j =
							prev->GetParameters().begin(); j != prev->GetParameters().end(); j++)
							params.push_back(pair< Ref<Type>, string>(j->type, j->name));
						if (!i->second->IsCompatible(prev->GetReturnValue(),
							prev->GetCallingConvention(), params))
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

						if (prev->IsFullyDefined())
						{
							// Function has already been fully defined, replace references
							// with the full function
							for (map< string, Ref<Function> >::const_iterator j =
								parser.GetFunctions().begin(); j != parser.GetFunctions().end(); j++)
								j->second->ReplaceFunction(i->second, prev);
							parser.GetInitExpression()->ReplaceFunction(i->second, prev);
						}
					}
					else
					{
						// New prototype, add to list of functions
						functions.push_back(i->second);
						functionsByName[i->second->GetName()] = i->second;
					}
				}
			}
		}

		if (parser.HasErrors())
			return 1;

		// Add initialization expression to global expression
		initExpression->AddChild(parser.GetInitExpression());

		// Link variables to other files
		for (vector< Ref<Variable> >::const_iterator i = parser.GetGlobalScope()->GetVariables().begin();
			i != parser.GetGlobalScope()->GetVariables().end(); i++)
		{
			if ((*i)->IsExternal())
			{
				// Variable is external
				if (variablesByName.find((*i)->GetName()) != variablesByName.end())
				{
					// Variable is defined in another file
					Variable* prev = variablesByName[(*i)->GetName()];

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
						for (vector< Ref<Function> >::iterator j = functions.begin();
							j != functions.end(); j++)
							(*j)->ReplaceVariable(*i, prev);
						initExpression->ReplaceVariable(*i, prev);
					}
				}
				else
				{
					// New definition
					variables.push_back(*i);
					variablesByName[(*i)->GetName()] = *i;
				}
			}
			else if ((*i)->IsLocalScope())
			{
				// Variable is local to the file, add to list but do not bind in name table
				variables.push_back(*i);
			}
			else
			{
				// Variable is global
				if (variablesByName.find((*i)->GetName()) != variablesByName.end())
				{
					// Variable is defined in another file
					Variable* prev = variablesByName[(*i)->GetName()];

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
					for (vector< Ref<Function> >::iterator j = functions.begin();
						j != functions.end(); j++)
						(*j)->ReplaceVariable(prev, *i);
					initExpression->ReplaceVariable(prev, *i);
				}

				// Add definition to variable list
				variables.push_back(*i);
				variablesByName[(*i)->GetName()] = *i;
			}
		}

		if (parser.HasErrors())
			return 1;
	}

	// Link complete, remove prototype functions from linked set
	for (size_t i = 0; i < functions.size(); i++)
	{
		if (!functions[i]->IsFullyDefined())
		{
			functions.erase(functions.begin() + i);
			i--;
		}
	}

	// Remove extern variables from linked set
	for (size_t i = 0; i < variables.size(); i++)
	{
		if (variables[i]->IsExternal())
		{
			variables.erase(variables.begin() + i);
			i--;
		}
	}

	// Generate errors for undefined references
	size_t errors = 0;
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
		(*i)->CheckForUndefinedReferences(errors);
	if (errors > 0)
		return 1;

	if (internalDebug)
	{
		fprintf(stderr, "Functions:\n");
		for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
			(*i)->Print();
	}

	fprintf(stderr, "Generating code...\n");

	// Create output class for the requested architecture
	Output* out;
	if (architecture == ARCH_X86)
	{
		if (preferredBits == 32)
			out = new OutputX86();
		else
			out = new OutputX64();
	}
	else
	{
		fprintf(stderr, "error: invalid architecture\n");
		return 1;
	}

	// Generate code on first pass.  This pass will generate the largest code possible when relative
	// addresses are present and unresolved.  A final pass will be made to generate short relative
	// references for those which are in range.
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		if (!out->GenerateCode(*i, false))
			return 1;
	}

	// Lay out address space for code
	uint64_t addr = 0;
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
		{
			(*j)->SetAddress(addr);
			addr += (*j)->GetOutputBlock()->len;
		}
	}

	// Generate code on final pass
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		if (!out->GenerateCode(*i, true))
			return 1;
	}

	// Compute final addresses
	addr = 0;
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
		{
			(*j)->SetAddress(addr);
			addr += (*j)->GetOutputBlock()->len;
		}
	}

	// Resolve relocations
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
		{
			if (!(*j)->ResolveRelocations())
				return 1;
		}
	}

	// Generate code section
	OutputBlock* codeSection = new OutputBlock;
	codeSection->code = NULL;
	codeSection->len = 0;
	codeSection->maxLen = 0;

	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
		{
			OutputBlock* block = (*j)->GetOutputBlock();
			memcpy(codeSection->PrepareWrite(block->len), block->code, block->len);
			codeSection->FinishWrite(block->len);
		}
	}

	// Generate final binary
	OutputBlock* finalBinary = new OutputBlock;
	finalBinary->code = NULL;
	finalBinary->len = 0;
	finalBinary->maxLen = 0;

	switch (format)
	{
	case FORMAT_BIN:
		memcpy(finalBinary->PrepareWrite(codeSection->len), codeSection->code, codeSection->len);
		finalBinary->FinishWrite(codeSection->len);
		break;
	default:
		fprintf(stderr, "error: unimplemented output format\n");
		return 1;
	}

	fprintf(stderr, "Output is %u bytes\n", (uint32_t)finalBinary->len);

	if ((maxLength != 0) && (finalBinary->len > maxLength))
	{
		fprintf(stderr, "error: unable to satisfy size constraint\n");
		return 1;
	}

	if (pad && (finalBinary->len < maxLength))
	{
		// Pad binary with random bytes (respecting blacklist)
		vector<uint8_t> available;
		for (size_t i = 0; i < 256; i++)
		{
			bool ok = true;
			for (vector<uint8_t>::iterator j = blacklist.begin(); j != blacklist.end(); j++)
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

		for (size_t i = finalBinary->len; i < maxLength; i++)
		{
			uint8_t choice = available[rand() % available.size()];
			*(uint8_t*)finalBinary->PrepareWrite(1) = choice;
			finalBinary->FinishWrite(1);
		}
	}

	if (execute)
	{
		// User wants to execute the code
		void* buffer = mmap(NULL, (finalBinary->len + 4095) & (~4095), PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_PRIVATE | MAP_ANON, -1, 0);
		memcpy(buffer, finalBinary->code, finalBinary->len);
		((void (*)())buffer)();

		// Don't trust the stack after that
		_exit(0);
	}

	if (hexOutput)
	{
		// Hex dump to stdout
		for (size_t i = 0; i < finalBinary->len; i += 16)
		{
			char ascii[17];
			ascii[16] = 0;
			printf("%.8x   ", (uint32_t)i);
			for (size_t j = 0; j < 16; j++)
			{
				if ((i + j) >= finalBinary->len)
				{
					printf("   ");
					ascii[j] = ' ';
				}
				else
				{
					uint8_t byte = ((uint8_t*)finalBinary->code)[i + j];
					printf("%.2x ", byte);
					if ((byte >= 0x20) && (byte <= 0x7e))
						ascii[j] = (char)byte;
					else
						ascii[j] = '.';
				}
			}
			printf("  %s\n", ascii);
		}
		return 0;
	}

	// Output to file
	FILE* outFP = stdout;
	if (outputFile.size() > 0)
	{
		outFP = fopen(outputFile.c_str(), "wb");
		if (!outFP)
		{
			fprintf(stderr, "error: unable to open output file '%s'\n", outputFile.c_str());
			return 1;
		}
	}

	if (!fwrite(finalBinary->code, finalBinary->len, 1, outFP))
	{
		fprintf(stderr, "error: unable to write to output file\n");
		fclose(outFP);
		return 1;
	}

	if (outFP != stdout)
		fclose(outFP);
	return 0;
}

