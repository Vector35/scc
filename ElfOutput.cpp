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


static const char* GetInterpreterName(const Settings& settings)
{
	if (settings.os == OS_LINUX)
	{
		if (settings.preferredBits == 64)
			return "/lib64/ld-linux-x86-64.so.2";
		else
			return "/lib/ld-linux.so.2";
	}

	return "";
}


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
	if (settings.positionIndependent)
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
		Elf64Header header;
		header.headerSize = sizeof(ElfIdent) + sizeof(ElfCommonHeader) + sizeof(Elf64Header);
		header.sectionHeaderOffset = 0;
		header.sectionHeaderSize = sizeof(Elf64SectionHeader);
		header.sectionHeaderCount = 0;
		header.stringTable = 0;
		header.programHeaderOffset = header.headerSize;
		header.programHeaderSize = sizeof(Elf64ProgramHeader);
		header.programHeaderCount = settings.positionIndependent ? 6 : 3;
		header.flags = 0;
		header.entry = settings.base;

		output->Write(&header, sizeof(header));

		if (settings.positionIndependent)
		{
			Elf64ProgramHeader phdr;
			phdr.type = 6; // PHDR
			phdr.flags = 5; // Read execute
			phdr.offset = header.programHeaderOffset;
			phdr.virtualAddress = phdr.offset;
			phdr.physicalAddress = phdr.offset;
			phdr.fileSize = header.programHeaderSize * header.programHeaderCount;
			phdr.memorySize = phdr.fileSize;
			phdr.align = 8;
			output->Write(&phdr, sizeof(phdr));

			const char* interpName = GetInterpreterName(settings);
			Elf64ProgramHeader interp;
			interp.type = 3; // INTERP
			interp.flags = 4; // Read
			interp.offset = header.headerSize + (sizeof(Elf64ProgramHeader) * header.programHeaderCount);
			interp.virtualAddress = interp.offset;
			interp.physicalAddress = interp.offset;
			interp.fileSize = strlen(interpName) + 1;
			interp.memorySize = interp.fileSize;
			interp.align = 1;
			output->Write(&interp, sizeof(interp));
		}

		Elf64ProgramHeader code;
		code.type = 1; // Loadable segment
		code.flags = 5; // Read execute
		code.offset = 0;
		code.virtualAddress = header.entry & (~0xfff);
		code.physicalAddress = code.virtualAddress;
		code.fileSize = (header.entry & 0xfff) + codeSection->len;
		code.memorySize = code.fileSize;
		code.align = 0x1000;
		output->Write(&code, sizeof(code));

		uint64_t dataStart = code.virtualAddress + code.fileSize;
		if (dataStart & 0xfff)
			dataStart += 0x1000;

		if (settings.positionIndependent)
		{
			static const char pad[8] = {0, 0, 0, 0, 0, 0, 0, 0};
			if ((dataStart + dataSection->len) & 7)
				dataSection->Write(pad, 8 - ((dataStart + dataSection->len) & 7));

			uint64_t dynEntries[10];
			dynEntries[0] = 5; // STRTAB;
			dynEntries[1] = 0;
			dynEntries[2] = 6; // SYMTAB;
			dynEntries[3] = 0;
			dynEntries[4] = 0xa; // STRSZ;
			dynEntries[5] = 0;
			dynEntries[6] = 0xb; // SYMENT;
			dynEntries[7] = 16;
			dynEntries[8] = 0; // NULL
			dynEntries[9] = 0;
			dataSection->Write(dynEntries, sizeof(dynEntries));
		}

		Elf64ProgramHeader data;
		data.type = 1; // Loadable segment
		data.flags = 6; // Read write
		data.offset = (header.entry & 0xfff) + codeSection->len;
		data.virtualAddress = dataStart;
		data.physicalAddress = data.virtualAddress;
		data.fileSize = dataSection->len;
		data.memorySize = data.fileSize;
		data.align = 0x1000;
		output->Write(&data, sizeof(data));

		Elf64ProgramHeader stack;
		stack.type = 0x6474e551; // GNU_STACK
		stack.flags = 6; // Read write
		stack.offset = 0;
		stack.virtualAddress = 0;
		stack.physicalAddress = 0;
		stack.fileSize = 0;
		stack.memorySize = 0;
		stack.align = 8;
		output->Write(&stack, sizeof(stack));

		if (settings.positionIndependent)
		{
			Elf64ProgramHeader dynamic;
			dynamic.type = 2; // DYNAMIC
			dynamic.flags = 4; // Read
			dynamic.offset = data.offset + dataSection->len - 80;
			dynamic.virtualAddress = data.virtualAddress + dataSection->len - 80;
			dynamic.physicalAddress = data.physicalAddress + dataSection->len - 80;
			dynamic.fileSize = 80;
			dynamic.memorySize = 80;
			dynamic.align = 8;
			output->Write(&dynamic, sizeof(dynamic));

			const char* interpName = GetInterpreterName(settings);
			output->Write(interpName, strlen(interpName) + 1);
		}
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
		header.programHeaderCount = settings.positionIndependent ? 6 : 3;
		header.flags = 0;
		header.entry = (uint32_t)settings.base;

		output->Write(&header, sizeof(header));

		if (settings.positionIndependent)
		{
			Elf32ProgramHeader phdr;
			phdr.type = 6; // PHDR
			phdr.flags = 5; // Read execute
			phdr.offset = header.programHeaderOffset;
			phdr.virtualAddress = phdr.offset;
			phdr.physicalAddress = phdr.offset;
			phdr.fileSize = header.programHeaderSize * header.programHeaderCount;
			phdr.memorySize = phdr.fileSize;
			phdr.align = 4;
			output->Write(&phdr, sizeof(phdr));

			const char* interpName = GetInterpreterName(settings);
			Elf32ProgramHeader interp;
			interp.type = 3; // INTERP
			interp.flags = 4; // Read
			interp.offset = header.headerSize + (sizeof(Elf32ProgramHeader) * header.programHeaderCount);
			interp.virtualAddress = interp.offset;
			interp.physicalAddress = interp.offset;
			interp.fileSize = strlen(interpName) + 1;
			interp.memorySize = interp.fileSize;
			interp.align = 1;
			output->Write(&interp, sizeof(interp));
		}

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

		if (settings.positionIndependent)
		{
			static const char pad[4] = {0, 0, 0, 0};
			if ((dataStart + dataSection->len) & 3)
				dataSection->Write(pad, 4 - ((dataStart + dataSection->len) & 3));

			uint32_t dynEntries[10];
			dynEntries[0] = 5; // STRTAB;
			dynEntries[1] = 0;
			dynEntries[2] = 6; // SYMTAB;
			dynEntries[3] = 0;
			dynEntries[4] = 0xa; // STRSZ;
			dynEntries[5] = 0;
			dynEntries[6] = 0xb; // SYMENT;
			dynEntries[7] = 16;
			dynEntries[8] = 0; // NULL
			dynEntries[9] = 0;
			dataSection->Write(dynEntries, sizeof(dynEntries));
		}

		Elf32ProgramHeader data;
		data.type = 1; // Loadable segment
		data.flags = 6; // Read write
		data.offset = (header.entry & 0xfff) + codeSection->len;
		data.virtualAddress = dataStart;
		data.physicalAddress = data.virtualAddress;
		data.fileSize = dataSection->len;
		data.memorySize = data.fileSize;
		data.align = 0x1000;
		output->Write(&data, sizeof(data));

		Elf32ProgramHeader stack;
		stack.type = 0x6474e551; // GNU_STACK
		stack.flags = 6; // Read write
		stack.offset = 0;
		stack.virtualAddress = 0;
		stack.physicalAddress = 0;
		stack.fileSize = 0;
		stack.memorySize = 0;
		stack.align = 4;
		output->Write(&stack, sizeof(stack));

		if (settings.positionIndependent)
		{
			Elf32ProgramHeader dynamic;
			dynamic.type = 2; // DYNAMIC
			dynamic.flags = 4; // Read
			dynamic.offset = data.offset + dataSection->len - 40;
			dynamic.virtualAddress = data.virtualAddress + dataSection->len - 40;
			dynamic.physicalAddress = data.physicalAddress + dataSection->len - 40;
			dynamic.fileSize = 40;
			dynamic.memorySize = 40;
			dynamic.align = 4;
			output->Write(&dynamic, sizeof(dynamic));

			const char* interpName = GetInterpreterName(settings);
			output->Write(interpName, strlen(interpName) + 1);
		}
	}

	output->Write(codeSection->code, codeSection->len);
	output->Write(dataSection->code, dataSection->len);
	return true;
}


uint64_t AdjustBaseForElfFile(uint64_t fileBase, const Settings& settings)
{
	uint64_t base = fileBase;
	if (settings.preferredBits == 64)
	{
		base += sizeof(ElfIdent) + sizeof(ElfCommonHeader) + sizeof(Elf64Header);
		base += sizeof(Elf64ProgramHeader) * 3;
		if (settings.positionIndependent)
		{
			base += sizeof(Elf64ProgramHeader) * 3;
			base += strlen(GetInterpreterName(settings)) + 1;
		}
	}
	else
	{
		base += sizeof(ElfIdent) + sizeof(ElfCommonHeader) + sizeof(Elf32Header);
		base += sizeof(Elf32ProgramHeader) * 3;
		if (settings.positionIndependent)
		{
			base += sizeof(Elf32ProgramHeader) * 3;
			base += strlen(GetInterpreterName(settings)) + 1;
		}
	}
	return base;
}


uint64_t AdjustDataSectionBaseForElfFile(uint64_t base)
{
	if (base & 0xfff)
		base += 0x1000;
	return base;
}

