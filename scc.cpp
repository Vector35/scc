#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <sys/mman.h>
#endif
#include <time.h>
#include "asmx86.h"
#include "Linker.h"
#include "ElfOutput.h"
#include "MachOOutput.h"
#include "PeOutput.h"

using namespace std;


// Create weak symbol for version
#ifdef RELEASE
extern const char* g_versionString;
#else
const char* g_versionString = "git";
#endif


void Usage()
{
	fprintf(stderr, "scc [options] <input files> [...]\n\n");
	fprintf(stderr, "Shellcode Compiler version %s\n", g_versionString);
	fprintf(stderr, "Copyright (C) 2015-2022 Vector 35 Inc\n");
	fprintf(stderr, "BETA RELEASE - NOT ALL OPTIONS ARE IMPLEMENTED\n\n");
	fprintf(stderr, "This compiler accepts a subset of C99 syntax, with extensions for creating a standalone\n");
	fprintf(stderr, "environment for writing shellcode.  Many standard system calls and C library functions\n");
	fprintf(stderr, "are automatically available without the need for include files.\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "    --arch <value>                    Specify processor architecture\n");
	fprintf(stderr, "                                      Can be: x86 (default), x64, arm, armeb, aarch64,\n");
	fprintf(stderr, "                                              mips, mipsel, ppc, ppcel\n");
	fprintf(stderr, "    --align <boundary>                Ensure output is aligned on the given boundary\n");
	fprintf(stderr, "    --allow-return                    Allow return from shellcode (default is to exit)\n");
	fprintf(stderr, "    --anti-disasm                     Generate anti-disassembly blocks\n");
	fprintf(stderr, "    --anti-disasm-freq <n>            Emit anti-disassembly blocks every <n> instructions\n");
	fprintf(stderr, "    --base <expr>                     Set base address of output (can be a runtime computed\n");
	fprintf(stderr, "                                      expression, such as \"[eax+8]-12\")\n");
	fprintf(stderr, "    --base-reg <reg>                  Global register that will hold base of code\n");
	fprintf(stderr, "    --blacklist <byte>                Blacklist the given byte value\n");
	fprintf(stderr, "    --concat                          Jump to end of output on return for concatenating code\n");
	fprintf(stderr, "    -D <define>[=<value>]             Define a preprocessor macro\n");
	fprintf(stderr, "    --decoder <source>                Use decoder to decode shellcode before executing\n");
	fprintf(stderr, "    --encode-pointers                 All code pointers are encoded with a random canary\n");
	fprintf(stderr, "    --encoder <source>                Use encoder to encode shellcode\n");
	fprintf(stderr, "    --exec                            Execute shellcode after generation (does not write\n");
	fprintf(stderr, "                                      output to a file)\n");
	fprintf(stderr, "    --exec-stack                      When outputting an executable, make stack executable\n");
	fprintf(stderr, "    --format <value>, -f <value>      Specify output format\n");
	fprintf(stderr, "                                      Can be: bin (default), lib, elf, pe, macho\n");
	fprintf(stderr, "    --frame-reg <reg>                 Use alternate register as the frame pointer\n");
	fprintf(stderr, "    --func <name> <address>           Assume function is at a specific address\n");
	fprintf(stderr, "    --funcptr <name> <address>        Assume function is pointed to by a specific address\n");
	fprintf(stderr, "    --gui                             For PE output, use GUI subsystem\n");
	fprintf(stderr, "    --header <file>                   Include a precompiled header\n");
	fprintf(stderr, "    -I <path>                         Add additional directory for include files\n");
	fprintf(stderr, "    -L <lib>                          Include pre-built library\n");
	fprintf(stderr, "    -m32, -m64                        Specify target address size\n");
	fprintf(stderr, "    --map <file>                      Generate map file\n");
	fprintf(stderr, "    --markov-chain                    Generate random instruction sequences for padding\n");
	fprintf(stderr, "    --markov-chain-file <file>        Use file for generating random instruction sequences\n");
	fprintf(stderr, "    --max-length <value>              Do not let output size exceed given number of bytes\n");
	fprintf(stderr, "    --mixed-mode                      Randomly choose subarchitecture for each function\n");
	fprintf(stderr, "    -o <filename>                     Set output filename (default is hex dump to stdout)\n");
	fprintf(stderr, "    -O0                               Do not run the optimizer\n");
	fprintf(stderr, "    -Os                               Try to generate the smallest code possible\n");
	fprintf(stderr, "    --pad                             Pad output to be exactly the maximum length\n");
	fprintf(stderr, "    --pie                             Always generate position independent code\n");
	fprintf(stderr, "    --platform <value>                Specify operating system\n");
	fprintf(stderr, "                                      Can be: linux (default), freebsd, mac, windows, none\n");
	fprintf(stderr, "    --polymorph                       Generate different code on each run\n");
	fprintf(stderr, "    --preserve <reg>                  Preserve the value of the given register\n");
	fprintf(stderr, "    --unloaded-modules                Uses modules that have not been loaded yet\n");
	fprintf(stderr, "    --unsafe-stack                    Stack pointer may be near the code\n");
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
	vector<string> defines;
	string outputFile = "";
	string mapFile = "";
	bool hexOutput = true;
	bool architectureIsExplicit = false;
	bool osIsExplicit = false;
	string decoder, encoder;
	bool execute = false;
	bool useSpecificSeed = false;
	bool positionIndependentExplicit = false;
	bool alignmentExplicit = false;
	Settings settings;

	settings.architecture = ARCH_X86;
	settings.os = OS_LINUX;
	settings.format = FORMAT_BIN;
	settings.optimization = OPTIMIZE_NORMAL;
	settings.preferredBits = 32;
	settings.bigEndian = false;
	settings.gui = false;
	settings.forcePebScan = false;
	settings.usesUnloadedModule = false;
	settings.allowReturn = false;
	settings.unsafeStack = false;
	settings.execStack = false;
	settings.concat = false;
	settings.pad = false;
	settings.maxLength = 0;
	settings.encodePointers = false;
	settings.stackGrowsUp = false;
	settings.sharedLibrary = false;
	settings.polymorph = false;
	settings.mixedMode = false;
	settings.antiDisasm = false;
	settings.antiDisasmFrequency = DEFAULT_ANTIDISASM_FREQUENCY;
	settings.seed = 0;
	settings.markovChains = false;
	settings.positionIndependent = true;
	settings.base = 0;
	settings.internalDebug = false;
	settings.sizeInfo = false;
	settings.alignment = 1;

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
				settings.bigEndian = false;
			}
			else if ((!strcmp(argv[i], "x64")) || (!strcmp(argv[i], "x86_64")) || (!strcmp(argv[i], "amd64")))
			{
				settings.architecture = ARCH_X86;
				settings.preferredBits = 64;
				settings.bigEndian = false;
			}
			else if (!strcmp(argv[i], "quark"))
			{
				settings.architecture = ARCH_QUARK;
				settings.preferredBits = 32;
				settings.bigEndian = false;
			}
			else if ((!strcmp(argv[i], "mips")) || (!strcmp(argv[i], "mipseb")))
			{
				settings.architecture = ARCH_MIPS;
				settings.preferredBits = 32;
				settings.bigEndian = true;
			}
			else if (!strcmp(argv[i], "mipsel"))
			{
				settings.architecture = ARCH_MIPS;
				settings.preferredBits = 32;
				settings.bigEndian = false;
			}
			else if ((!strcmp(argv[i], "arm")) || (!strcmp(argv[i], "armel")))
			{
				settings.architecture = ARCH_ARM;
				settings.preferredBits = 32;
				settings.bigEndian = false;
			}
			else if (!strcmp(argv[i], "armeb"))
			{
				settings.architecture = ARCH_ARM;
				settings.preferredBits = 32;
				settings.bigEndian = true;
			}
			else if (!strcmp(argv[i], "aarch64"))
			{
				settings.architecture = ARCH_AARCH64;
				settings.preferredBits = 64;
				settings.bigEndian = false;
			}
			else if ((!strcmp(argv[i], "ppc")) || (!strcmp(argv[i], "ppceb")))
			{
				settings.architecture = ARCH_PPC;
				settings.preferredBits = 32;
				settings.bigEndian = true;
			}
			else if (!strcmp(argv[i], "ppcel"))
			{
				settings.architecture = ARCH_PPC;
				settings.preferredBits = 32;
				settings.bigEndian = false;
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
		else if (!strcmp(argv[i], "--align"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.alignment = atoi(argv[i]);
			alignmentExplicit = true;
			if (settings.alignment == 0)
			{
				fprintf(stderr, "error: invalid alignment\n");
				return 1;
			}
			continue;
		}
		else if (!strcmp(argv[i], "--allow-return"))
		{
			settings.allowReturn = true;
			continue;
		}
		else if (!strcmp(argv[i], "--anti-disasm"))
		{
			settings.antiDisasm = true;
			continue;
		}
		else if (!strcmp(argv[i], "--anti-disasm-freq"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.antiDisasmFrequency = atoi(argv[i]);
			if (settings.antiDisasmFrequency == 0)
			{
				fprintf(stderr, "error: invalid anti-disassembly frequency\n");
				return 1;
			}
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
		else if (!strcmp(argv[i], "-D"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			defines.push_back(argv[i]);
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
				settings.os = OS_MAC;
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
		else if (!strcmp(argv[i], "--exec-stack"))
		{
			settings.execStack = true;
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
			settings.frameRegName = argv[i];
			continue;
		}
		else if (!strcmp(argv[i], "--func"))
		{
			if ((i + 2) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.funcAddrs[argv[i]] = strtoull(argv[i + 1], NULL, 0);
			i++;
			continue;
		}
		else if (!strcmp(argv[i], "--funcptr"))
		{
			if ((i + 2) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.funcPtrAddrs[argv[i]] = strtoull(argv[i + 1], NULL, 0);
			i++;
			continue;
		}
		else if (!strcmp(argv[i], "--force-pebscan"))
		{
			settings.forcePebScan = true;
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
			else if (!strcmp(argv[i], "macho"))
				settings.format = FORMAT_MACHO;
			else if (!strcmp(argv[i], "lib"))
				settings.format = FORMAT_LIB;
			else
			{
				fprintf(stderr, "error: unsupported format '%s'\n", argv[i]);
			}

			continue;
		}
		else if (!strcmp(argv[i], "--gui"))
		{
			settings.gui = true;
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
			settings.internalDebug = true;
			continue;
		}
		else if (!strcmp(argv[i], "-I"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.includeDirs.push_back(argv[i]);
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
		else if (!strcmp(argv[i], "--markov-chain"))
		{
			settings.markovChains = true;
			continue;
		}
		else if (!strcmp(argv[i], "--markov-chain-file"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "error: missing value after '%s'\n", argv[i]);
				return 1;
			}

			i++;
			settings.markovChains = true;
			settings.markovFile = argv[i];
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
			settings.maxLength = strtoul(argv[i], NULL, 0);
			continue;
		}
		else if (!strcmp(argv[i], "--mixed-mode"))
		{
			settings.mixedMode = true;
			continue;
		}
		else if (!strcmp(argv[i], "--pad"))
		{
			settings.pad = true;
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
			else if ((!strcmp(argv[i], "mac")) || (!strcmp(argv[i], "macos")) ||
				(!strcmp(argv[i], "macosx")) || (!strcmp(argv[i], "darwin")))
				settings.os = OS_MAC;
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
		else if (!strcmp(argv[i], "--unloaded-modules"))
		{
			settings.usesUnloadedModule = true;
			continue;
		}
		else if (!strcmp(argv[i], "--unsafe-stack"))
		{
			settings.unsafeStack = true;
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
			settings.returnRegName = argv[i];
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
			settings.returnHighRegName = argv[i];
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
		else if (!strcmp(argv[i], "--size-info"))
		{
			settings.sizeInfo = true;
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
			settings.stackRegName = argv[i];
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
		else if (!strcmp(argv[i], "--version"))
		{
			fprintf(stderr, "%s\n", g_versionString);
			return 0;
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
	if (settings.polymorph || settings.mixedMode || settings.antiDisasm || settings.pad)
	{
		if (!useSpecificSeed)
		{
#ifdef WIN32
			LARGE_INTEGER pc;
			QueryPerformanceCounter(&pc);
			settings.seed = pc.LowPart ^ pc.HighPart ^ (uint32_t)time(NULL);
#else
			FILE* fp = fopen("/dev/urandom", "rb");
			if (fread(&settings.seed, sizeof(settings.seed), 1, fp) != 1)
			{
				fprintf(stderr, "error: unable to generate random seed\n");
				return 1;
			}
			fclose(fp);
#endif
			fprintf(stderr, "Seed is %u\n", settings.seed);
		}

		srand(settings.seed);
	}

	// Warn about incompatible options
	if (settings.sharedLibrary && (settings.format == FORMAT_BIN))
	{
		fprintf(stderr, "warning: trying to generate shared library in raw binary output mode\n");
		settings.sharedLibrary = false;
	}

	if ((settings.maxLength != 0) && (settings.format != FORMAT_BIN))
	{
		fprintf(stderr, "warning: maximum size only supported in raw binary output mode\n");
		settings.maxLength = 0;
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

	if ((settings.architecture == ARCH_QUARK) && (settings.preferredBits != 32))
	{
		fprintf(stderr, "error: invalid architecture settings\n");
		return 1;
	}

	if ((settings.architecture == ARCH_MIPS) && (settings.preferredBits != 32))
	{
		fprintf(stderr, "error: invalid architecture settings\n");
		return 1;
	}

	if ((settings.architecture == ARCH_ARM) && (settings.preferredBits != 32))
	{
		fprintf(stderr, "error: invalid architecture settings\n");
		return 1;
	}

	if ((settings.architecture == ARCH_AARCH64) && (settings.preferredBits != 64))
	{
		fprintf(stderr, "error: invalid architecture settings\n");
		return 1;
	}

	if ((settings.architecture == ARCH_PPC) && (settings.preferredBits != 32))
	{
		fprintf(stderr, "error: invalid architecture settings\n");
		return 1;
	}

	// Adjust base address for executables
	if (settings.format == FORMAT_ELF)
	{
		if (!positionIndependentExplicit)
			settings.positionIndependent = false;
		if (!settings.positionIndependent)
			settings.base = 0x8040000;
		settings.base = AdjustBaseForElfFile(settings.base, settings);
	}
	if (settings.format == FORMAT_MACHO)
	{
		if (!positionIndependentExplicit)
			settings.positionIndependent = false;
		settings.base = (settings.preferredBits == 32) ? 0x1000 : 0x100000000LL;
		settings.base = AdjustBaseForMachOFile(settings.base, settings);
	}
	if (settings.format == FORMAT_PE)
	{
		if (!positionIndependentExplicit)
			settings.positionIndependent = false;
		settings.base = 0x1000000;
		settings.base = AdjustBaseForPeFile(settings.base, settings);
	}

	// Adjust alignment for architectures that require it
	if ((settings.architecture == ARCH_QUARK) && (!alignmentExplicit))
		settings.alignment = 4;
	if ((settings.architecture == ARCH_MIPS) && (!alignmentExplicit))
		settings.alignment = 4;
	if ((settings.architecture == ARCH_ARM) && (!alignmentExplicit))
		settings.alignment = 4;
	if ((settings.architecture == ARCH_PPC) && (!alignmentExplicit))
		settings.alignment = 4;

	if (settings.architecture == ARCH_AARCH64)
		settings.stackAlignment = 16;

	// Set pointer size
	if (settings.preferredBits == 32)
		SetTargetPointerSize(4);
	else
		SetTargetPointerSize(8);

	Linker linker(settings);

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
		if (fread(data, 1, size, fp) != (size_t)size)
		{
			fprintf(stderr, "%s: error: unable to read file\n", library.c_str());
			return 1;
		}
		fclose(fp);

		InputBlock input;
		input.code = data;
		input.len = size;
		input.offset = 0;

		if (!linker.ImportLibrary(&input))
		{
			fprintf(stderr, "%s: error: invalid format\n", library.c_str());
			return 1;
		}
	}
	else
	{
		// No library given by user, find the correct internal library for this OS and architecture
		if (!linker.ImportStandardLibrary())
		{
			fprintf(stderr, "error: invalid format in internal library\n");
			return 1;
		}
	}

	if (defines.size() != 0)
	{
		// Add the defines from the command line
		for (vector<string>::iterator i = defines.begin(); i != defines.end(); i++)
		{
			string source = "#define ";

			size_t equals = i->find('=');
			if (equals == string::npos)
				source += *i;
			else
			{
				source += i->substr(0, equals);
				source += " ";
				source += i->substr(equals + 1);
			}

			source += "\n";

			if (!linker.PrecompileSource(source))
			{
				fprintf(stderr, "error: invalid define '%s'\n", i->c_str());
				return 1;
			}
		}
	}

	// Add define for endianness
	if (settings.bigEndian)
	{
		string source = "#ifndef BIG_ENDIAN\n#define BIG_ENDIAN\n#endif\n";
		if (!linker.PrecompileSource(source))
		{
			fprintf(stderr, "internal error: unable to define BIG_ENDIAN\n");
			return 1;
		}
	}
	else
	{
		string source = "#ifndef LITTLE_ENDIAN\n#define LITTLE_ENDIAN\n#endif\n";
		if (!linker.PrecompileSource(source))
		{
			fprintf(stderr, "internal error: unable to define LITTLE_ENDIAN\n");
			return 1;
		}
	}

	if (precompiledHeaders.size() != 0)
	{
		// Process the precompiled headers
		for (vector<string>::iterator i = precompiledHeaders.begin(); i != precompiledHeaders.end(); i++)
		{
			if (!linker.PrecompileHeader(*i))
				return 1;
		}
	}

	if ((precompiledHeaders.size() != 0) || (defines.size() != 0))
	{
		// Parse the precompiled headers
		if (!linker.FinalizePrecompiledHeaders())
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
			data = new char[max + 2];
			while (!feof(stdin))
			{
				char ch;
				if (!fread(&ch, 1, 1, stdin))
					break;

				if (size >= max)
				{
					max = (size + 1) * 2;
					char* newData = new char[max + 2];
					memcpy(newData, data, size);
					if (data)
						delete[] data;
					data = newData;
				}

				data[size++] = ch;
			}

			data[size++] = '\n';
			data[size] = 0;
		}
		else
		{
			FILE* fp = fopen(i->c_str(), "rb");
			if (!fp)
			{
				fprintf(stderr, "%s: error: file not found\n", i->c_str());
				return 1;
			}

			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			data = new char[size + 2];
			if (fread(data, 1, size, fp) != (size_t)size)
			{
				fprintf(stderr, "%s: error: unable to read file\n", i->c_str());
				return 1;
			}
			data[size++] = '\n';
			data[size] = 0;
			fclose(fp);
		}

		if (!linker.CompileSource(data, (i->size() == 0) ? string("stdin") : *i))
			return 1;
		delete[] data;
	}

	// If producing a library, serialize state
	if (settings.format == FORMAT_LIB)
	{
		OutputBlock output;
		output.code = NULL;
		output.len = 0;
		output.maxLen = 0;
		output.bigEndian = false;

		if (!linker.OutputLibrary(&output))
			return 1;

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

	// Finalize link and output code
	if (!linker.FinalizeLink())
		return 1;

	OutputBlock finalBinary;
	finalBinary.code = NULL;
	finalBinary.len = 0;
	finalBinary.maxLen = 0;
	finalBinary.bigEndian = settings.bigEndian;
	if (!linker.OutputCode(&finalBinary))
		return 1;

	// Generate map file
	if (mapFile.size() != 0)
	{
		if (!linker.WriteMapFile(mapFile))
			return 1;
	}

	if ((settings.maxLength != 0) && (finalBinary.len > settings.maxLength))
	{
		fprintf(stderr, "Output is %u bytes\n", (uint32_t)finalBinary.len);
		fprintf(stderr, "error: unable to satisfy size constraint\n");
		return 1;
	}

	if (settings.pad && (finalBinary.len < settings.maxLength))
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

		for (size_t i = finalBinary.len; i < settings.maxLength; i++)
		{
			uint8_t choice = available[rand() % available.size()];
			*(uint8_t*)finalBinary.PrepareWrite(1) = choice;
			finalBinary.FinishWrite(1);
		}
	}

	fprintf(stderr, "Output is %u bytes\n", (uint32_t)finalBinary.len);

	if (execute)
	{
#ifdef WIN32
		fprintf(stderr, "error: --exec not yet supported on Windows\n");
#else
		// User wants to execute the code
		void* buffer = mmap(NULL, (finalBinary.len + 4095) & (~4095), PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_PRIVATE | MAP_ANON, -1, 0);
		memcpy(buffer, finalBinary.code, finalBinary.len);
		((void (*)())buffer)();
#endif

		// Don't trust the stack after that
		_exit(0);
	}

	if (hexOutput)
	{
		// Hex dump to stdout
		for (size_t i = 0; i < finalBinary.len; i += 16)
		{
			char ascii[17];
			ascii[16] = 0;
			printf("%.8x   ", (uint32_t)i);
			for (size_t j = 0; j < 16; j++)
			{
				if ((i + j) >= finalBinary.len)
				{
					printf("   ");
					ascii[j] = ' ';
				}
				else
				{
					uint8_t byte = ((uint8_t*)finalBinary.code)[i + j];
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

	if (!fwrite(finalBinary.code, finalBinary.len, 1, outFP))
	{
		fprintf(stderr, "error: unable to write to output file\n");
		fclose(outFP);
		return 1;
	}

	if (outFP != stdout)
		fclose(outFP);
	return 0;
}
