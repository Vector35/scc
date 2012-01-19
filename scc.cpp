#include "asmx86.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <sys/mman.h>
#include "ParserState.h"
#include "PreprocessState.h"
#include "CodeParser.h"
#include "CodeLexer.h"
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


bool ImportLibrary(InputBlock* input, vector< Ref<Function> >& functions, map< string, Ref<Function> >& functionsByName,
	vector< Ref<Variable> >& variables, map< string, Ref<Variable> >& variablesByName, Ref<Expr>& initExpression,
	PreprocessState& precompiledPreprocess, ParserState& precompileState)
{
	// Deserialize precompiled header state
	if (!precompiledPreprocess.Deserialize(input))
		return false;
	if (!precompileState.Deserialize(input))
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
		functions.push_back(func);
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
		variables.push_back(var);
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

		functionsByName[name] = func;
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

		variablesByName[name] = var;
	}

	// Deserialize initialization expression
	initExpression = Expr::Deserialize(input);
	if (!initExpression)
		return false;

	return true;
}


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
	fprintf(stderr, "    --frame-reg <reg>                 Use alternate register as the frame pointer\n");
	fprintf(stderr, "    --header <file>                   Include a precompiled header\n");
	fprintf(stderr, "    --internal-debug                  Enable internal debugging output\n");
	fprintf(stderr, "    -L <lib>                          Include pre-built library\n");
	fprintf(stderr, "    -m32, -m64                        Specify target address size\n");
	fprintf(stderr, "    --map <file>                      Generate map file\n");
	fprintf(stderr, "    --max-length <value>              Do not let output size exceed given number of bytes\n");
	fprintf(stderr, "    -o <filename>                     Set output filename (default is hex dump to stdout)\n");
	fprintf(stderr, "    -O0                               Do not run the optimizer\n");
	fprintf(stderr, "    -Os                               Try to generate the smallest code possible\n");
	fprintf(stderr, "    --pad                             Pad output to be exactly the maximum length\n");
	fprintf(stderr, "    --pie                             Always generate position independent code\n");
	fprintf(stderr, "    --platform <value>                Specify operating system\n");
	fprintf(stderr, "                                      Can be: linux (default), freebsd, mach, windows, none\n");
	fprintf(stderr, "    --polymorph                       Generate different code on each run\n");
	fprintf(stderr, "    --preserve <reg>                  Preserve the value of the given register\n");
	fprintf(stderr, "    --return-reg <reg>                Use alternate register as the return value\n");
	fprintf(stderr, "    --return-high-reg <reg>           Use alternate register as the upper 32 bits of return\n");
	fprintf(stderr, "                                      value (32-bit output only)\n");
	fprintf(stderr, "    --seed <value>                    Specify random seed (to reproduce --polymorph runs)\n");
	fprintf(stderr, "    --shared                          Generate shared library instead of executable\n");
	fprintf(stderr, "    --stack-grows-up                  Stack grows toward larger addresses\n");
	fprintf(stderr, "    --stack-reg <reg>                 Use alternate register as the stack pointer\n");
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
	string library;
	vector<string> precompiledHeaders;
	string outputFile = "";
	string mapFile = "";
	bool hexOutput = true;
	bool architectureIsExplicit = false;
	bool osIsExplicit = false;
	string decoder, encoder;
	bool execute = false;
	bool internalDebug = false;
	uint32_t maxLength = 0;
	bool pad = false;
	bool useSpecificSeed = false;
	bool positionIndependentExplicit = false;
	Settings settings;

	settings.architecture = ARCH_X86;
	settings.os = OS_LINUX;
	settings.format = FORMAT_BIN;
	settings.optimization = OPTIMIZE_NORMAL;
	settings.preferredBits = 32;
	settings.allowReturn = false;
	settings.assumeSafeStack = false;
	settings.concat = false;
	settings.encodePointers = false;
	settings.stackGrowsUp = false;
	settings.sharedLibrary = false;
	settings.polymorph = false;
	settings.seed = 0;
	settings.staticBase = false;
	settings.positionIndependent = true;
	settings.base = 0;

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
				settings.architecture = ARCH_X86;
				settings.preferredBits = 32;
			}
			else if ((!strcmp(argv[i], "x64")) || (!strcmp(argv[i], "x86_64")) || (!strcmp(argv[i], "amd64")))
			{
				settings.architecture = ARCH_X86;
				settings.preferredBits = 64;
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
			settings.preferredBits = 32;
			architectureIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "-m64"))
		{
			settings.preferredBits = 64;
			architectureIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "--allow-return"))
		{
			settings.allowReturn = true;
			continue;
		}
		else if (!strcmp(argv[i], "--assume-safe-stack"))
		{
			settings.assumeSafeStack = true;
			continue;
		}
		else if (!strcmp(argv[i], "--concat"))
		{
			settings.concat = true;
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
			settings.blacklist.push_back((uint8_t)strtoul(argv[i], NULL, 0));
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
			settings.encodePointers = true;
			continue;
		}
		else if (!strcmp(argv[i], "--exec"))
		{
#ifdef __x86_64
			if (!architectureIsExplicit)
				settings.preferredBits = 64;
#endif

			if (!osIsExplicit)
			{
				// Use current OS
#ifdef __APPLE__
				settings.os = OS_MACH;
#elif defined(WIN32)
				settings.os = OS_WINDOWS;
#elif defined(linux)
				settings.os = OS_LINUX;
#else
				settings.os = OS_FREEBSD;
#endif
			}

			execute = true;
			continue;
		}
		else if (!strcmp(argv[i], "--frame-reg"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.frameReg = argv[i];
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
				settings.format = FORMAT_BIN;
			else if (!strcmp(argv[i], "elf"))
				settings.format = FORMAT_ELF;
			else if (!strcmp(argv[i], "pe"))
				settings.format = FORMAT_PE;
			else if (!strcmp(argv[i], "lib"))
				settings.format = FORMAT_LIB;
			else
			{
				fprintf(stderr, "error: unsupported format '%s'\n", argv[i]);
			}

			continue;
		}
		else if (!strcmp(argv[i], "--header"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			precompiledHeaders.push_back(argv[i]);
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

			if (library.size() != 0)
			{
				fprintf(stderr, "error: only one precompiled library is allowed\n");
				return 1;
			}

			i++;
			library = argv[i];
			continue;
		}
		else if (!strcmp(argv[i], "--map"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			mapFile = argv[i];
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
			settings.optimization = OPTIMIZE_DISABLE;
			continue;
		}
		else if (!strcmp(argv[i], "-Os"))
		{
			settings.optimization = OPTIMIZE_SIZE;
			continue;
		}
		else if (!strcmp(argv[i], "--pie"))
		{
			settings.positionIndependent = true;
			positionIndependentExplicit = true;
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
				settings.os = OS_LINUX;
			else if (!strcmp(argv[i], "freebsd"))
				settings.os = OS_FREEBSD;
			else if ((!strcmp(argv[i], "mach")) || (!strcmp(argv[i], "macos")) || (!strcmp(argv[i], "macosx")))
				settings.os = OS_MACH;
			else if ((!strcmp(argv[i], "win32")) || (!strcmp(argv[i], "windows")))
				settings.os = OS_WINDOWS;
			else if (!strcmp(argv[i], "none"))
				settings.os = OS_NONE;
			else
			{
				fprintf(stderr, "error: unsupported platform '%s'\n", argv[i]);
			}

			osIsExplicit = true;
			continue;
		}
		else if (!strcmp(argv[i], "--polymorph"))
		{
			settings.polymorph = true;
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
			settings.preservedRegs.push_back(argv[i]);
			continue;
		}
		else if (!strcmp(argv[i], "--return-reg"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.returnReg = argv[i];
			continue;
		}
		else if (!strcmp(argv[i], "--return-high-reg"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.returnHighReg = argv[i];
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
			settings.seed = (uint32_t)strtoul(argv[i], NULL, 0);
			continue;
		}
		else if (!strcmp(argv[i], "--shared"))
		{
			settings.sharedLibrary = true;
			continue;
		}
		else if (!strcmp(argv[i], "--stack-grows-up"))
		{
			settings.stackGrowsUp = true;
			continue;
		}
		else if (!strcmp(argv[i], "--stack-reg"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.stackReg = argv[i];
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
	if (settings.polymorph || pad)
	{
		if (!useSpecificSeed)
		{
			FILE* fp = fopen("/dev/urandom", "rb");
			fread(&settings.seed, sizeof(settings.seed), 1, fp);
			fclose(fp);
			printf("Seed is %u\n", settings.seed);
		}

		srand(settings.seed);
	}

	// Warn about incompatible options
	if (settings.sharedLibrary && (settings.format == FORMAT_BIN))
	{
		fprintf(stderr, "warning: trying to generate shared library in raw binary output mode\n");
		settings.sharedLibrary = false;
	}

	if ((maxLength != 0) && (settings.format != FORMAT_BIN))
	{
		fprintf(stderr, "warning: maximum size only supported in raw binary output mode\n");
		maxLength = 0;
	}

	if ((settings.blacklist.size() > 0) && (settings.format != FORMAT_BIN))
	{
		fprintf(stderr, "warning: blacklist only supported in raw binary output mode\n");
		settings.blacklist.clear();
	}

	if ((settings.format == FORMAT_LIB) && hexOutput)
	{
		fprintf(stderr, "error: output filename expected for library output\n");
		return 1;
	}

	// Normal executables always have a normal stack
	if (settings.format != FORMAT_BIN)
		settings.assumeSafeStack = true;

	// Set base address of executable if not provided
	if ((!settings.staticBase) && (settings.format == FORMAT_ELF) && (!settings.sharedLibrary))
	{
		settings.staticBase = true;
		if (!positionIndependentExplicit)
			settings.positionIndependent = false;
		settings.base = AdjustBaseForElfFile(0x8040000, settings);
	}
	if ((!settings.staticBase) && (settings.format == FORMAT_PE) && (!settings.sharedLibrary))
	{
		settings.staticBase = true;
		if (!positionIndependentExplicit)
			settings.positionIndependent = false;
		settings.base = 0x1001000;
	}

	// Set pointer size
	if (settings.preferredBits == 32)
		SetTargetPointerSize(4);
	else
		SetTargetPointerSize(8);

	// Set up precompiled state
	PreprocessState precompiledPreprocess("precompiled headers", NULL);
	ParserState precompileState("precompiled headers", NULL);
	vector< Ref<Function> > functions;
	map< string, Ref<Function> > functionsByName;
	vector< Ref<Variable> > variables;
	map< string, Ref<Variable> > variablesByName;
	Ref<Expr> initExpression = new Expr(EXPR_SEQUENCE);

	// If there is a precompiled library, import it now
	if (library.size() != 0)
	{
		// Read library data into memory
		FILE* fp = fopen(library.c_str(), "rb");
		if (!fp)
		{
			fprintf(stderr, "%s: error: file not found\n", library.c_str());
			return 1;
		}

		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		uint8_t* data = new uint8_t[size];
		fread(data, 1, size, fp);
		fclose(fp);

		InputBlock input;
		input.code = data;
		input.len = size;
		input.offset = 0;

		if (!ImportLibrary(&input, functions, functionsByName, variables, variablesByName, initExpression,
			precompiledPreprocess, precompileState))
		{
			fprintf(stderr, "%s: error: invalid format\n", library.c_str());
			return 1;
		}
	}
	else
	{
		// No library given by user, find the correct internal library for this OS and architecture
		unsigned char* lib;
		unsigned int len;

		switch (settings.os)
		{
		case OS_LINUX:
			if (settings.preferredBits == 32)
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
			if (settings.preferredBits == 32)
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
			if (settings.preferredBits == 32)
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
			if (settings.preferredBits == 32)
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
			if (settings.preferredBits == 32)
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

			if (!ImportLibrary(&input, functions, functionsByName, variables, variablesByName, initExpression,
				precompiledPreprocess, precompileState))
			{
				fprintf(stderr, "error: invalid format in internal library\n");
				return 1;
			}
		}
	}

	if (precompiledHeaders.size() != 0)
	{
		// Process the precompiled headers
		for (vector<string>::iterator i = precompiledHeaders.begin(); i != precompiledHeaders.end(); i++)
		{
			precompiledPreprocess.IncludeFile(*i);
			if (precompiledPreprocess.HasErrors())
				return 1;
		}

		// Parse the precompiled headers
		yyscan_t scanner;
		Code_lex_init(&scanner);
		precompileState.SetScanner(scanner);

		YY_BUFFER_STATE buf = Code__scan_string(precompiledPreprocess.GetOutput().c_str(), scanner);
		Code__switch_to_buffer(buf, scanner);
		Code_set_lineno(1, scanner);

		bool ok = true;
		if (Code_parse(&precompileState) != 0)
			ok = false;
		if (precompileState.HasErrors())
			ok = false;

		Code_lex_destroy(scanner);

		if (!ok)
			return 1;
	}

	// Start parsing source files
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

		string preprocessed;
		if (!PreprocessState::PreprocessSource(data, (i->size() == 0) ? string("stdin") : *i,
			preprocessed, &precompiledPreprocess))
			return 1;

		yyscan_t scanner;
		Code_lex_init(&scanner);
		ParserState parser(&precompileState, (i->size() == 0) ? "stdin" : i->c_str(), scanner);

		YY_BUFFER_STATE buf = Code__scan_string(preprocessed.c_str(), scanner);
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

	// If producing a library, serialize state
	if (settings.format == FORMAT_LIB)
	{
		OutputBlock output;
		output.code = NULL;
		output.len = 0;
		output.maxLen = 0;

		// Serialize precompiled header state
		precompiledPreprocess.Serialize(&output);
		precompileState.Serialize(&output);

		// Serialize function objects
		output.WriteInteger(functions.size());
		for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
			(*i)->Serialize(&output);

		// Serialize variable objects
		output.WriteInteger(variables.size());
		for (vector< Ref<Variable> >::iterator i = variables.begin(); i != variables.end(); i++)
			(*i)->Serialize(&output);

		// Serialize function name map
		output.WriteInteger(functionsByName.size());
		for (map< string, Ref<Function> >::iterator i = functionsByName.begin(); i != functionsByName.end(); i++)
		{
			output.WriteString(i->first);
			i->second->Serialize(&output);
		}

		// Serialize variable name map
		output.WriteInteger(variablesByName.size());
		for (map< string, Ref<Variable> >::iterator i = variablesByName.begin(); i != variablesByName.end(); i++)
		{
			output.WriteString(i->first);
			i->second->Serialize(&output);
		}

		// Serialize initialization expression
		initExpression->Serialize(&output);

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

		if (!fwrite(output.code, output.len, 1, outFP))
		{
			fprintf(stderr, "error: unable to write to output file\n");
			fclose(outFP);
			return 1;
		}

		if (outFP != stdout)
			fclose(outFP);

		return 0;
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

	// Find main function
	map< string, Ref<Function> >::iterator mainFuncRef = functionsByName.find("main");
	if (mainFuncRef == functionsByName.end())
	{
		fprintf(stderr, "error: function 'main' is undefined\n");
		return 1;
	}
	Ref<Function> mainFunc = mainFuncRef->second;

	if (mainFunc->HasVariableArguments())
	{
		fprintf(stderr, "error: function 'main' can not have variable arguments\n");
		return 1;
	}

	// Find exit function
	map< string, Ref<Function> >::iterator exitFuncRef = functionsByName.find("exit");
	if (exitFuncRef == functionsByName.end())
	{
		fprintf(stderr, "error: function 'exit' is undefined\n");
		return 1;
	}
	Ref<Function> exitFunc = exitFuncRef->second;

	// Generate _start function
	map< string, Ref<Function> >::iterator entryFuncRef = functionsByName.find("_start");
	if (entryFuncRef != functionsByName.end())
	{
		fprintf(stderr, "error: cannot override internal function '_start'\n");
		return 1;
	}

	FunctionInfo startInfo;
	startInfo.returnValue = mainFunc->GetReturnValue();
	startInfo.callingConvention = mainFunc->GetCallingConvention();
	startInfo.name = "_start";
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
	functions.insert(functions.begin(), startFunction);
	functionsByName["_start"] = startFunction;

	Ref<Expr> startBody = new Expr(EXPR_SEQUENCE);

	// If using encoded pointers, choose the key now
	if (settings.encodePointers)
	{
		settings.encodePointerKey = new Variable(VAR_GLOBAL, Type::IntType(GetTargetPointerSize(), false), "@pointer_key");
		variables.push_back(settings.encodePointerKey);
		variablesByName["@pointer_key"] = settings.encodePointerKey;

		Ref<Expr> keyExpr = Expr::VariableExpr(mainFunc->GetLocation(), settings.encodePointerKey);
		Ref<Expr> valueExpr = new Expr(mainFunc->GetLocation(), (GetTargetPointerSize() == 4) ? EXPR_RDTSC_LOW : EXPR_RDTSC);
		startBody->AddChild(Expr::BinaryExpr(mainFunc->GetLocation(), EXPR_ASSIGN, keyExpr, valueExpr));
	}

	// Add global variable initialization expression
	startBody->AddChild(initExpression);

	Ref<Expr> mainExpr = Expr::FunctionExpr(mainFunc->GetLocation(), mainFunc);
	Ref<Expr> exitExpr = Expr::FunctionExpr(mainFunc->GetLocation(), exitFunc);

	// Generate call to main
	vector< Ref<Expr> > params;
	for (size_t i = 0; i < mainFunc->GetParameters().size(); i++)
		params.push_back(Expr::VariableExpr(mainFunc->GetLocation(), paramVars[i]));
	Ref<Expr> callExpr = Expr::CallExpr(mainFunc->GetLocation(), mainExpr, params);

	// Handle result of main
	if (settings.allowReturn)
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
		return 1;

	// Generate IL
	startFunction->GenerateIL(&startState);
	startFunction->ReportUndefinedLabels(&startState);
	if (startState.HasErrors())
		return 1;

	// Make string constants into global const character arrays
	map< string, Ref<Variable> > stringMap;
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			(*j)->ConvertStringsToVariables(stringMap);
	}

	for (map< string, Ref<Variable> >::iterator i = stringMap.begin(); i != stringMap.end(); i++)
		variables.push_back(i->second);

	// Tag everything referenced from the main function
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
		(*i)->ResetTagCount();
	for (vector< Ref<Variable> >::iterator i = variables.begin(); i != variables.end(); i++)
		(*i)->ResetTagCount();
	functions[0]->TagReferences();

	// Remove anything not referenced
	for (size_t i = 0; i < functions.size(); i++)
	{
		if (functions[i]->GetTagCount() == 0)
		{
			functions.erase(functions.begin() + i);
			i--;
		}
	}

	for (size_t i = 0; i < variables.size(); i++)
	{
		if (variables[i]->GetTagCount() == 0)
		{
			variables.erase(variables.begin() + i);
			i--;
		}
	}

	if (internalDebug)
	{
		fprintf(stderr, "Functions:\n");
		for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
			(*i)->Print();
	}

	// Generate errors for undefined references
	size_t errors = 0;
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
		(*i)->CheckForUndefinedReferences(errors);
	if (errors > 0)
		return 1;

	fprintf(stderr, "Generating code...\n");

	// Create output class for the requested architecture
	Output* out;
	if (settings.architecture == ARCH_X86)
	{
		if (settings.preferredBits == 32)
			out = new OutputX86(settings);
		else
			out = new OutputX64(settings);
	}
	else
	{
		fprintf(stderr, "error: invalid architecture\n");
		return 1;
	}

	// Generate data section
	OutputBlock* dataSection = new OutputBlock;
	dataSection->code = NULL;
	dataSection->len = 0;
	dataSection->maxLen = 0;

	// Lay out address space for data
	uint64_t addr = 0;
	for (vector< Ref<Variable> >::iterator i = variables.begin(); i != variables.end(); i++)
	{
		if (addr & ((*i)->GetType()->GetAlignment() - 1))
			addr += (*i)->GetType()->GetAlignment() - (addr & ((*i)->GetType()->GetAlignment() - 1));

		(*i)->SetDataSectionOffset(addr);

		dataSection->Write((*i)->GetData().code, (*i)->GetData().len);
		if ((*i)->GetData().len < (*i)->GetType()->GetWidth())
		{
			uint8_t zero = 0;
			for (size_t j = (*i)->GetData().len; j < (*i)->GetType()->GetWidth(); j++)
				dataSection->Write(&zero, 1);
		}

		addr += (*i)->GetType()->GetWidth();
	}

	// Ensure IL blocks have global indexes
	size_t globalBlockIndex = 0;
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			(*j)->SetGlobalIndex(globalBlockIndex++);
	}

	// Generate code for each block
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		if (!out->GenerateCode(*i))
			return 1;
	}

	// Check relocations and ensure that everything is within bounds, and expand any references that are not
	while (true)
	{
		// Lay out address space for code
		addr = settings.base;
		for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
		{
			for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			{
				(*j)->SetAddress(addr);
				addr += (*j)->GetOutputBlock()->len;
			}
		}

		settings.dataSectionBase = addr;
		if (settings.format == FORMAT_ELF)
			settings.dataSectionBase = AdjustDataSectionBaseForElfFile(settings.dataSectionBase);

		// Check relocations and gather the overflow list
		vector<RelocationReference> overflows;
		for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
		{
			for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
			{
				if (!(*j)->CheckRelocations(settings.dataSectionBase, overflows))
					return 1;
			}
		}

		if (overflows.size() == 0)
		{
			// All relocations are within limits, ready to finalize
			break;
		}

		// There are relocations that do not fit within the size allocated, need to call the overflow handlers
		for (vector<RelocationReference>::iterator i = overflows.begin(); i != overflows.end(); i++)
		{
			i->reloc->overflow(i->block, i->reloc->start, i->reloc->offset);

			if (i->reloc->type == CODE_RELOC_RELATIVE_8)
				i->reloc->type = CODE_RELOC_RELATIVE_32;
			else if (i->reloc->type == DATA_RELOC_RELATIVE_8)
				i->reloc->type = DATA_RELOC_RELATIVE_32;
		}
	}

	// Resolve relocations
	for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
	{
		for (vector<ILBlock*>::const_iterator j = (*i)->GetIL().begin(); j != (*i)->GetIL().end(); j++)
		{
			if (!(*j)->ResolveRelocations(settings.dataSectionBase))
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

	switch (settings.format)
	{
	case FORMAT_BIN:
		memcpy(finalBinary->PrepareWrite(codeSection->len), codeSection->code, codeSection->len);
		finalBinary->FinishWrite(codeSection->len);
		memcpy(finalBinary->PrepareWrite(dataSection->len), dataSection->code, dataSection->len);
		finalBinary->FinishWrite(dataSection->len);
		break;
	case FORMAT_ELF:
		if (!GenerateElfFile(finalBinary, settings, codeSection, dataSection))
		{
			fprintf(stderr, "error: failed to output ELF format\n");
			return 1;
		}
		break;
	default:
		fprintf(stderr, "error: unimplemented output format\n");
		return 1;
	}

	// Generate map file
	if (mapFile.size() != 0)
	{
		FILE* outFP = fopen(mapFile.c_str(), "w");
		if (!outFP)
		{
			fprintf(stderr, "error: unable to open map file\n");
			return 1;
		}

		for (vector< Ref<Function> >::iterator i = functions.begin(); i != functions.end(); i++)
		{
			if ((*i)->GetName().size() == 0)
				continue;
			fprintf(outFP, "%llx %s\n", (unsigned long long)(*i)->GetIL()[0]->GetAddress(), (*i)->GetName().c_str());
		}

		for (vector< Ref<Variable> >::iterator i = variables.begin(); i != variables.end(); i++)
		{
			if ((*i)->GetName().size() == 0)
				continue;
			if ((*i)->GetName()[0] == '$')
				continue;
			fprintf(outFP, "%llx %s\n", (unsigned long long)(settings.dataSectionBase +
				(*i)->GetDataSectionOffset()), (*i)->GetName().c_str());
		}

		fclose(outFP);
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
			for (vector<uint8_t>::iterator j = settings.blacklist.begin(); j != settings.blacklist.end(); j++)
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

