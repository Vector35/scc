#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <vector>
#include <string>
#include <stdint.h>

#define DEFAULT_ANTIDISASM_FREQUENCY 20

enum Architecture
{
	ARCH_X86,
	ARCH_QUARK,
	ARCH_MIPS,
	ARCH_ARM,
	ARCH_PPC
};

enum OperatingSystem
{
	OS_NONE,
	OS_LINUX,
	OS_FREEBSD,
	OS_MAC,
	OS_WINDOWS
};

enum OutputFormat
{
	FORMAT_BIN,
	FORMAT_ELF,
	FORMAT_PE,
	FORMAT_MACHO,
	FORMAT_LIB
};

enum OptimizationLevel
{
	OPTIMIZE_DISABLE,
	OPTIMIZE_SIZE,
	OPTIMIZE_NORMAL
};

class Variable;

struct Settings
{
	std::vector<uint8_t> blacklist;
	std::vector<std::string> preservedRegs;
	std::string stackRegName, frameRegName, returnRegName, returnHighRegName;
	uint32_t stackPointer, framePointer, returnReg, returnHighReg, basePointer;

	std::vector<std::string> includeDirs;

	std::map<std::string, uint64_t> funcAddrs;
	std::map<std::string, uint64_t> funcPtrAddrs;

	Architecture architecture;
	OperatingSystem os;
	OutputFormat format;
	OptimizationLevel optimization;
	uint32_t preferredBits;
	bool bigEndian;
	bool gui, forcePebScan, usesUnloadedModule;

	bool allowReturn;
	bool unsafeStack;
	bool execStack;
	bool concat;
	bool sharedLibrary;

	bool pad;
	size_t maxLength;

	bool stackGrowsUp;
	bool encodePointers;
	Ref<Variable> encodePointerKey;

	bool polymorph, mixedMode;
	uint32_t seed;
	bool markovChains;
	std::string markovFile;

	bool antiDisasm;
	uint32_t antiDisasmFrequency;

	bool positionIndependent;
	uint64_t base;
	uint64_t dataSectionBase;
	uint32_t alignment;

	bool internalDebug;
	bool sizeInfo;
};

#endif

