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

	void WriteByteSwap(OutputBlock* output)
	{
		ElfCommonHeader copy;
		copy.type = BE16(type);
		copy.arch = BE16(arch);
		copy.version = BE32(version);
		output->Write(&copy, sizeof(copy));
	}
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

	void WriteByteSwap(OutputBlock* output)
	{
		Elf32Header copy;
		copy.entry = BE32(entry);
		copy.programHeaderOffset = BE32(programHeaderOffset);
		copy.sectionHeaderOffset = BE32(sectionHeaderOffset);
		copy.flags = BE32(flags);
		copy.headerSize = BE16(headerSize);
		copy.programHeaderSize = BE16(programHeaderSize);
		copy.programHeaderCount = BE16(programHeaderCount);
		copy.sectionHeaderSize = BE16(sectionHeaderSize);
		copy.sectionHeaderCount = BE16(sectionHeaderCount);
		copy.stringTable = BE16(stringTable);
		output->Write(&copy, sizeof(copy));
	}
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

	void WriteByteSwap(OutputBlock* output)
	{
		Elf32ProgramHeader copy;
		copy.type = BE32(type);
		copy.offset = BE32(offset);
		copy.virtualAddress = BE32(virtualAddress);
		copy.physicalAddress = BE32(physicalAddress);
		copy.fileSize = BE32(fileSize);
		copy.memorySize = BE32(memorySize);
		copy.flags = BE32(flags);
		copy.align = BE32(align);
		output->Write(&copy, sizeof(copy));
	}
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

	void WriteByteSwap(OutputBlock* output)
	{
		Elf32SectionHeader copy;
		copy.name = BE32(name);
		copy.type = BE32(type);
		copy.flags = BE32(flags);
		copy.address = BE32(address);
		copy.offset = BE32(offset);
		copy.size = BE32(size);
		copy.link = BE32(link);
		copy.info = BE32(info);
		copy.align = BE32(align);
		copy.entrySize = BE32(entrySize);
		output->Write(&copy, sizeof(copy));
	}
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

	void WriteByteSwap(OutputBlock* output)
	{
		Elf64Header copy;
		copy.entry = BE64(entry);
		copy.programHeaderOffset = BE64(programHeaderOffset);
		copy.sectionHeaderOffset = BE64(sectionHeaderOffset);
		copy.flags = BE32(flags);
		copy.headerSize = BE16(headerSize);
		copy.programHeaderSize = BE16(programHeaderSize);
		copy.programHeaderCount = BE16(programHeaderCount);
		copy.sectionHeaderSize = BE16(sectionHeaderSize);
		copy.sectionHeaderCount = BE16(sectionHeaderCount);
		copy.stringTable = BE16(stringTable);
		output->Write(&copy, sizeof(copy));
	}
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

	void WriteByteSwap(OutputBlock* output)
	{
		Elf64ProgramHeader copy;
		copy.type = BE32(type);
		copy.flags = BE32(flags);
		copy.offset = BE64(offset);
		copy.virtualAddress = BE64(virtualAddress);
		copy.physicalAddress = BE64(physicalAddress);
		copy.fileSize = BE64(fileSize);
		copy.memorySize = BE64(memorySize);
		copy.align = BE64(align);
		output->Write(&copy, sizeof(copy));
	}
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

	void WriteByteSwap(OutputBlock* output)
	{
		Elf64SectionHeader copy;
		copy.name = BE32(name);
		copy.type = BE32(type);
		copy.flags = BE64(flags);
		copy.address = BE64(address);
		copy.offset = BE64(offset);
		copy.size = BE64(size);
		copy.link = BE32(link);
		copy.info = BE32(info);
		copy.align = BE64(align);
		copy.entrySize = BE64(entrySize);
		output->Write(&copy, sizeof(copy));
	}
};


static const char* GetInterpreterName(const Settings& settings)
{
	if (settings.os == OS_LINUX)
	{
		switch (settings.architecture)
		{
		case ARCH_X86:
			if (settings.preferredBits == 64)
				return "/lib64/ld-linux-x86-64.so.2";
			else
				return "/lib/ld-linux.so.2";
		default:
			return "/lib/ld.so.1";
		}
	}

	if (settings.os == OS_FREEBSD)
		return "/libexec/ld-elf.so.1";

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

	ident.encoding = settings.bigEndian ? 2 : 1;
	ident.version = 1;

	ident.os = 0;
	if (settings.architecture != ARCH_PPC)
	{
		if (settings.os == OS_LINUX)
			ident.os = 3;
		if (settings.os == OS_FREEBSD)
			ident.os = 9;
	}

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
	case ARCH_MIPS:
		commonHeader.arch = 8;
		break;
	case ARCH_PPC:
		commonHeader.arch = 20;
		break;
	case ARCH_ARM:
		commonHeader.arch = 40;
		break;
	case ARCH_AARCH64:
		commonHeader.arch = 183;
		break;
	case ARCH_QUARK:
		commonHeader.arch = 4242;
		break;
	default:
		return false;
	}

	commonHeader.version = 1;
	if (settings.bigEndian)
		commonHeader.WriteByteSwap(output);
	else
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
		if (settings.bigEndian)
			header.WriteByteSwap(output);
		else
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

			if (settings.bigEndian)
				phdr.WriteByteSwap(output);
			else
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
			if (settings.bigEndian)
				interp.WriteByteSwap(output);
			else
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
		if (settings.bigEndian)
			code.WriteByteSwap(output);
		else
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

			if (settings.bigEndian)
			{
				for (size_t i = 0; i < 10; i++)
					dynEntries[i] = BE64(dynEntries[i]);
			}
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
		if (settings.bigEndian)
			data.WriteByteSwap(output);
		else
			output->Write(&data, sizeof(data));

		Elf64ProgramHeader stack;
		stack.type = 0x6474e551; // GNU_STACK
		stack.flags = settings.execStack ? 7 : 6;
		stack.offset = 0;
		stack.virtualAddress = 0;
		stack.physicalAddress = 0;
		stack.fileSize = 0;
		stack.memorySize = 0;
		stack.align = 8;
		if (settings.bigEndian)
			stack.WriteByteSwap(output);
		else
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
			if (settings.bigEndian)
				dynamic.WriteByteSwap(output);
			else
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
		if (settings.bigEndian)
			header.WriteByteSwap(output);
		else
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
			if (settings.bigEndian)
				phdr.WriteByteSwap(output);
			else
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
			if (settings.bigEndian)
				interp.WriteByteSwap(output);
			else
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
		if (settings.bigEndian)
			code.WriteByteSwap(output);
		else
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

			if (settings.bigEndian)
			{
				for (size_t i = 0; i < 10; i++)
					dynEntries[i] = BE32(dynEntries[i]);
			}
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
		if (settings.bigEndian)
			data.WriteByteSwap(output);
		else
			output->Write(&data, sizeof(data));

		Elf32ProgramHeader stack;
		stack.type = 0x6474e551; // GNU_STACK
		stack.flags = settings.execStack ? 7 : 6;
		stack.offset = 0;
		stack.virtualAddress = 0;
		stack.physicalAddress = 0;
		stack.fileSize = 0;
		stack.memorySize = 0;
		stack.align = 4;
		if (settings.bigEndian)
			stack.WriteByteSwap(output);
		else
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
			if (settings.bigEndian)
				dynamic.WriteByteSwap(output);
			else
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

