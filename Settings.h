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
	FORMAT_PE
};

enum OptimizationLevel
{
	OPTIMIZE_DISABLE,
	OPTIMIZE_SIZE,
	OPTIMIZE_NORMAL
};

struct Settings
{
	std::vector<uint8_t> blacklist;
	std::vector<std::string> preservedRegs;

	Architecture architecture;
	OperatingSystem os;
	OutputFormat format;
	OptimizationLevel optimization;
	uint32_t preferredBits;

	bool allowReturn;
	bool assumeSafeStack;
	bool concat;
	bool encodePointers;
	bool sharedLibrary;

	bool polymorph;
	uint32_t seed;

	bool staticBase;
	uint64_t base;
};

#endif

