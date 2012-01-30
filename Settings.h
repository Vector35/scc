#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <vector>
#include <string>
#include <stdint.h>

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
	FORMAT_PE,
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
	std::string stackReg, frameReg, returnReg, returnHighReg, baseReg;

	Architecture architecture;
	OperatingSystem os;
	OutputFormat format;
	OptimizationLevel optimization;
	uint32_t preferredBits;

	bool allowReturn;
	bool unsafeStack;
	bool execStack;
	bool concat;
	bool sharedLibrary;

	bool stackGrowsUp;
	bool encodePointers;
	Ref<Variable> encodePointerKey;

	bool polymorph, mixedMode;
	uint32_t seed;

	bool positionIndependent;
	bool staticBase;
	uint64_t base;
	uint64_t dataSectionBase;

	bool internalDebug;
};

#endif

