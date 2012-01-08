#include <stdlib.h>
#include <string.h>
#include "ElfOutput.h"
#include "Struct.h"


struct ElfIdent
{
	char signature[4];
	uint8_t fileClass;
	uint8_t encoding;
	uint8_t version;
	uint8_t os;
	uint8_t abiVersion;
	uint8_t pad[7];
};

struct ElfCommonHeader
{
	uint16_t type;
	uint16_t arch;
	uint32_t version;
};

struct Elf32Header
{
	uint32_t entry;
	uint32_t programHeaderOffset;
	uint32_t sectionHeaderOffset;
	uint32_t flags;
	uint16_t headerSize;
	uint16_t programHeaderSize;
	uint16_t programHeaderCount;
	uint16_t sectionHeaderSize;
	uint16_t sectionHeaderCount;
	uint16_t stringTable;
};

struct Elf32ProgramHeader
{
	uint32_t type;
	uint32_t offset;
	uint32_t virtualAddress;
	uint32_t physicalAddress;
	uint32_t fileSize;
	uint32_t memorySize;
	uint32_t flags;
	uint32_t align;
};

struct Elf32SectionHeader
{
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t address;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t align;
	uint32_t entrySize;
};

struct Elf64Header
{
	uint64_t entry;
	uint64_t programHeaderOffset;
	uint64_t sectionHeaderOffset;
	uint32_t flags;
	uint16_t headerSize;
	uint16_t programHeaderSize;
	uint16_t programHeaderCount;
	uint16_t sectionHeaderSize;
	uint16_t sectionHeaderCount;
	uint16_t stringTable;
};

struct Elf64ProgramHeader
{
	uint32_t type;
	uint32_t flags;
	uint64_t offset;
	uint64_t virtualAddress;
	uint64_t physicalAddress;
	uint64_t fileSize;
	uint64_t memorySize;
	uint64_t align;
};

struct Elf64SectionHeader
{
	uint32_t name;
	uint32_t type;
	uint64_t flags;
	uint64_t address;
	uint64_t offset;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t align;
	uint64_t entrySize;
};


bool GenerateElfFile(OutputBlock* output, const Settings& settings, OutputBlock* codeSection, OutputBlock* dataSection)
{
	// Output file identification header
	ElfIdent ident;
	memcpy(ident.signature, "\x7f""ELF", 4);

	ident.fileClass = 1; // 32-bit
	if (settings.preferredBits == 64)
		ident.fileClass = 2; // 64-bit

	ident.encoding = 1; // Little endian
	ident.version = 1;

	ident.os = 0;
	if (settings.os == OS_FREEBSD)
		ident.os = 9;

	ident.abiVersion = 0;
	memset(ident.pad, 0, sizeof(ident.pad));

	output->Write(&ident, sizeof(ident));

	// Write out part of header that is common between 32-bit and 64-bit
	ElfCommonHeader commonHeader;
	commonHeader.type = 2; // Executable
	if (settings.sharedLibrary)
		commonHeader.type = 3; // Shared library

	switch (settings.architecture)
	{
	case ARCH_X86:
		commonHeader.arch = 3; // x86
		if (settings.preferredBits == 64)
			commonHeader.arch = 62;
		break;
	default:
		return false;
	}

	commonHeader.version = 1;
	output->Write(&commonHeader, sizeof(commonHeader));

	// Write out rest of header
	if (settings.preferredBits == 64)
	{
		return false;
	}
	else
	{
		Elf32Header header;
		header.headerSize = sizeof(ElfIdent) + sizeof(ElfCommonHeader) + sizeof(Elf32Header);
		header.sectionHeaderOffset = 0;
		header.sectionHeaderSize = sizeof(Elf32SectionHeader);
		header.sectionHeaderCount = 0;
		header.stringTable = 0;
		header.programHeaderOffset = header.headerSize;
		header.programHeaderSize = sizeof(Elf32ProgramHeader);
		header.programHeaderCount = 2;
		header.flags = 0;

		header.entry = 0;
		if (settings.staticBase)
			header.entry = (uint32_t)settings.base;

		output->Write(&header, sizeof(header));

		Elf32ProgramHeader code;
		code.type = 1; // Loadable segment
		code.flags = 5; // Read execute
		code.offset = 0;
		code.virtualAddress = header.entry & (~0xfff);
		code.physicalAddress = code.virtualAddress;
		code.fileSize = (header.entry & 0xfff) + codeSection->len;
		code.memorySize = code.fileSize;
		code.align = 0x1000;
		output->Write(&code, sizeof(code));

		uint32_t dataStart = code.virtualAddress + code.fileSize;
		if (dataStart & 0xfff)
			dataStart += 0x1000;

		Elf32ProgramHeader data;
		data.type = 1; // Loadable segment
		data.flags = 6; // Read write
		data.offset = header.headerSize + (sizeof(Elf32ProgramHeader) * 2) + codeSection->len;
		data.virtualAddress = dataStart;
		data.physicalAddress = data.virtualAddress;
		data.fileSize = dataSection->len;
		data.memorySize = data.fileSize;
		data.align = 0x1000;
		output->Write(&data, sizeof(data));
	}

	output->Write(codeSection->code, codeSection->len);
	output->Write(dataSection->code, dataSection->len);
	return true;
}


uint64_t AdjustBaseForElfFile(uint64_t fileBase, const Settings& settings)
{
	if (settings.preferredBits == 64)
		return fileBase + sizeof(ElfIdent) + sizeof(ElfCommonHeader) + sizeof(Elf64Header) + (sizeof(Elf64ProgramHeader) * 2);
	return fileBase + sizeof(ElfIdent) + sizeof(ElfCommonHeader) + sizeof(Elf32Header) + (sizeof(Elf32ProgramHeader) * 2);
}


uint64_t GetDataSectionBaseForElfFile(OutputBlock* codeSection, uint64_t entry)
{
	uint64_t base = entry + codeSection->len;
	if (base & 0xfff)
		base += 0x1000;
	return base;
}

