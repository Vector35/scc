// Copyright (c) 2011-2014 Rusty Wagner
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

#include <stdlib.h>
#include <string.h>
#include "PeOutput.h"
#include "Struct.h"

using namespace std;


struct MzHeader
{
	uint16_t magic;
	uint16_t lastSize;
	uint16_t nBlocks;
	uint16_t nReloc;
	uint16_t hdrSize;
	uint16_t minAlloc;
	uint16_t maxAlloc;
	uint16_t ss, sp;
	uint16_t checksum;
	uint16_t ip, cs;
	uint16_t relocPos;
	uint16_t nOverlay;
	uint8_t reserved1[8];
	uint16_t oemId, oemInfo;
	uint8_t reserved2[20];
	uint32_t peOffset;
};

struct PeHeader
{
	uint32_t magic;
	uint16_t machine;
	uint16_t numberOfSections;
	uint32_t timestamp;
	uint32_t coffSymbolTable;
	uint32_t coffSymbolCount;
	uint16_t sizeOfOptionalHeader;
	uint16_t characteristics;
};

struct Pe32OptionalHeader
{
	uint16_t magic;
	uint8_t majorLinkerVersion, minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitData;
	uint32_t sizeOfUninitData;
	uint32_t addressOfEntryPoint;
	uint32_t baseOfCode;
	uint32_t baseOfData;
	uint32_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
	uint16_t majorOsVersion, minorOsVersion;
	uint16_t majorImageVersion, minorImageVersion;
	uint16_t majorSubsystemVersion, minorSubsystemVersion;
	uint32_t win32Version;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checksum;
	uint16_t subsystem;
	uint16_t dllCharacteristics;
	uint32_t sizeOfStackReserve;
	uint32_t sizeOfStackCommit;
	uint32_t sizeOfHeapReserve;
	uint32_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t dataDirCount;
};

struct Pe64OptionalHeader
{
	uint16_t magic;
	uint8_t majorLinkerVersion, minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitData;
	uint32_t sizeOfUninitData;
	uint32_t addressOfEntryPoint;
	uint32_t baseOfCode;
	uint64_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
	uint16_t majorOsVersion, minorOsVersion;
	uint16_t majorImageVersion, minorImageVersion;
	uint16_t majorSubsystemVersion, minorSubsystemVersion;
	uint32_t win32Version;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checksum;
	uint16_t subsystem;
	uint16_t dllCharacteristics;
	uint64_t sizeOfStackReserve;
	uint64_t sizeOfStackCommit;
	uint64_t sizeOfHeapReserve;
	uint64_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t dataDirCount;
};

struct PeDataDirectoryEntry
{
	uint32_t virtualAddress;
	uint32_t size;
};

struct PeSection
{
	char name[8];
	uint32_t virtualSize;
	uint32_t virtualAddress;
	uint32_t sizeOfRawData;
	uint32_t pointerToRawData;
	uint32_t pointerToRelocs;
	uint32_t pointerToLineNumbers;
	uint16_t relocCount;
	uint16_t lineNumberCount;
	uint32_t characteristics;
};

struct PeImportDirectoryEntry
{
	uint32_t lookup;
	uint32_t timestamp;
	uint32_t forwardChain;
	uint32_t name;
	uint32_t iat;
};

struct PeExportDirectoryEntry
{
	uint32_t characteristics;
	uint32_t timestamp;
	uint16_t majorVersion, minorVersion;
	uint32_t dllName;
	uint32_t base;
	uint32_t functionCount;
	uint32_t nameCount;
	uint32_t addressOfFunctions;
	uint32_t addressOfNames;
	uint32_t addressOfNameOrdinals;
};


bool GeneratePeFile(OutputBlock* output, const Settings& settings, OutputBlock* codeSection, OutputBlock* dataSection,
	map<string, ImportTable>& imports)
{
	uint8_t padding[0x200];
	memset(padding, 0, sizeof(padding));

	size_t sizeOfCode = (codeSection->len + 0xfff) & (~0xfff);

	// Prepare import table name strings
	map<string, size_t> moduleNameOffsets;
	for (map<string, ImportTable>::iterator i = imports.begin(); i != imports.end(); ++i)
	{
		moduleNameOffsets[i->first] = dataSection->len;
		string name = i->first + ".dll";
		dataSection->Write(name.c_str(), strlen(name.c_str()) + 1);

		for (vector< Ref<Function> >::iterator j = i->second.functions.begin(); j != i->second.functions.end(); ++j)
		{
			if (dataSection->len & 1)
				dataSection->Write(padding, 1);

			i->second.nameOffsets.push_back(dataSection->len);
			dataSection->WriteUInt16(0);
			dataSection->Write((*j)->GetName().c_str(), strlen((*j)->GetName().c_str()) + 1);
		}
	}

	size_t nativeAlign = (settings.preferredBits == 64) ? 8 : 4;
	if (dataSection->len & (nativeAlign - 1))
		dataSection->Write(padding, nativeAlign - (dataSection->len & (nativeAlign - 1)));

	// Prepare import lookup table
	map<string, size_t> moduleLookupOffsets;
	for (map<string, ImportTable>::iterator i = imports.begin(); i != imports.end(); ++i)
	{
		moduleLookupOffsets[i->first] = dataSection->len;

		for (size_t j = 0; j < i->second.nameOffsets.size(); j++)
		{
			size_t nameOffset = 0x1000 + sizeOfCode + i->second.nameOffsets[j];
			if (settings.preferredBits == 64)
			{
				dataSection->WriteUInt64((uint64_t)nameOffset);
				dataSection->WriteOffsetUInt64(i->second.table->GetDataSectionOffset() + (j * 8), (uint64_t)nameOffset);
			}
			else
			{
				dataSection->WriteUInt32((uint32_t)nameOffset);
				dataSection->WriteOffsetUInt32(i->second.table->GetDataSectionOffset() + (j * 4), (uint32_t)nameOffset);
			}
		}

		if (settings.preferredBits == 64)
			dataSection->WriteUInt64(0);
		else
			dataSection->WriteUInt32(0);
	}

	// Prepare import directory
	size_t importDirStart = dataSection->len;
	for (map<string, ImportTable>::iterator i = imports.begin(); i != imports.end(); ++i)
	{
		PeImportDirectoryEntry import;
		memset(&import, 0, sizeof(import));

		import.lookup = 0x1000 + sizeOfCode + moduleLookupOffsets[i->first];
		import.name = 0x1000 + sizeOfCode + moduleNameOffsets[i->first];
		import.iat = 0x1000 + sizeOfCode + i->second.table->GetDataSectionOffset();

		dataSection->Write(&import, sizeof(import));
	}

	dataSection->Write(padding, sizeof(PeImportDirectoryEntry));
	size_t importDirSize = dataSection->len - importDirStart;

	// Output MZ header
	MzHeader mz;
	memset(&mz, 0, sizeof(mz));
	memcpy(&mz.magic, "MZ", 2);
	mz.hdrSize = sizeof(mz);
	mz.relocPos = 0x40;
	mz.peOffset = sizeof(mz);
	output->Write(&mz, sizeof(mz));

	// Output PE header
	PeHeader pe;
	memset(&pe, 0, sizeof(pe));
	memcpy(&pe.magic, "PE\0\0", 4);

	switch (settings.architecture)
	{
	case ARCH_X86:
		pe.machine = 0x14c; // x86
		if (settings.preferredBits == 64)
			pe.machine = 0x8664;
		break;
	case ARCH_MIPS:
		pe.machine = 0x366;
		break;
	case ARCH_PPC:
		pe.machine = 0x1f1;
		break;
	case ARCH_ARM:
		pe.machine = 0x1c0;
		break;
	case ARCH_QUARK:
		pe.machine = 0x4242;
		break;
	default:
		return false;
	}

	pe.numberOfSections = 2;
	pe.characteristics = 0x32e; // Executable, large address aware, stripped

	if (settings.preferredBits == 64)
		pe.sizeOfOptionalHeader = sizeof(Pe64OptionalHeader) + (sizeof(PeDataDirectoryEntry) * 16);
	else
	{
		pe.sizeOfOptionalHeader = sizeof(Pe32OptionalHeader) + (sizeof(PeDataDirectoryEntry) * 16);
		pe.characteristics |= 0x100; // 32-bit machine
	}

	output->Write(&pe, sizeof(pe));

	// Write out rest of header
	if (settings.preferredBits == 64)
	{
		Pe64OptionalHeader opt;
		memset(&opt, 0, sizeof(opt));

		opt.magic = 0x20b;
		opt.sizeOfCode = sizeOfCode;
		opt.sizeOfInitData = (dataSection->len + 0xfff) & (~0xfff);
		opt.addressOfEntryPoint = 0x1000;
		opt.baseOfCode = 0x1000;
		opt.imageBase = settings.base - 0x1000;
		opt.sectionAlignment = 0x1000;
		opt.fileAlignment = 0x200;
		opt.majorOsVersion = 6;
		opt.majorSubsystemVersion = 6;
		opt.sizeOfImage = sizeOfCode + opt.sizeOfInitData + 0x1000;
		opt.sizeOfHeaders = 0x200;
		opt.subsystem = settings.gui ? 2 : 3;
		opt.sizeOfStackReserve = 1 << 20;
		opt.sizeOfStackCommit = 1 << 12;
		opt.sizeOfHeapReserve = 1 << 20;
		opt.sizeOfHeapCommit = 1 << 12;
		opt.dataDirCount = 16;

		if (settings.positionIndependent)
			opt.dllCharacteristics |= 0x40;
		if (!settings.execStack)
			opt.dllCharacteristics |= 0x100;

		output->Write(&opt, sizeof(opt));

		PeDataDirectoryEntry dataDir[16];
		memset(dataDir, 0, sizeof(dataDir));

		dataDir[1].virtualAddress = 0x1000 + sizeOfCode + importDirStart;
		dataDir[1].size = importDirSize;

		output->Write(dataDir, sizeof(dataDir));

		PeSection code;
		memset(&code, 0, sizeof(code));
		strcpy(code.name, ".text");
		code.virtualSize = sizeOfCode;
		code.virtualAddress = 0x1000;
		code.sizeOfRawData = (codeSection->len + 0x1ff) & (~0x1ff);
		code.pointerToRawData = 0x200;
		code.characteristics = 0x60000020; // Code, execute read
		output->Write(&code, sizeof(code));

		PeSection data;
		memset(&data, 0, sizeof(data));
		strcpy(data.name, ".data");
		data.virtualSize = opt.sizeOfInitData;
		data.virtualAddress = sizeOfCode + 0x1000;
		data.sizeOfRawData = (dataSection->len + 0x1ff) & (~0x1ff);
		data.pointerToRawData = code.sizeOfRawData + 0x200;
		data.characteristics = 0xc0000040; // Initialized data, read/write
		output->Write(&data, sizeof(data));
	}
	else
	{
		Pe32OptionalHeader opt;
		memset(&opt, 0, sizeof(opt));

		opt.magic = 0x10b;
		opt.sizeOfCode = sizeOfCode;
		opt.sizeOfInitData = (dataSection->len + 0xfff) & (~0xfff);
		opt.addressOfEntryPoint = 0x1000;
		opt.baseOfCode = 0x1000;
		opt.baseOfData = sizeOfCode + 0x1000;
		opt.imageBase = settings.base - 0x1000;
		opt.sectionAlignment = 0x1000;
		opt.fileAlignment = 0x200;
		opt.majorOsVersion = 6;
		opt.majorSubsystemVersion = 6;
		opt.sizeOfImage = sizeOfCode + opt.sizeOfInitData + 0x1000;
		opt.sizeOfHeaders = 0x200;
		opt.subsystem = settings.gui ? 2 : 3;
		opt.sizeOfStackReserve = 1 << 20;
		opt.sizeOfStackCommit = 1 << 12;
		opt.sizeOfHeapReserve = 1 << 20;
		opt.sizeOfHeapCommit = 1 << 12;
		opt.dataDirCount = 16;

		if (settings.positionIndependent)
			opt.dllCharacteristics |= 0x40;
		if (!settings.execStack)
			opt.dllCharacteristics |= 0x100;

		output->Write(&opt, sizeof(opt));

		PeDataDirectoryEntry dataDir[16];
		memset(dataDir, 0, sizeof(dataDir));

		dataDir[1].virtualAddress = 0x1000 + sizeOfCode + importDirStart;
		dataDir[1].size = importDirSize;

		output->Write(dataDir, sizeof(dataDir));

		PeSection code;
		memset(&code, 0, sizeof(code));
		strcpy(code.name, ".text");
		code.virtualSize = sizeOfCode;
		code.virtualAddress = 0x1000;
		code.sizeOfRawData = (codeSection->len + 0x1ff) & (~0x1ff);
		code.pointerToRawData = 0x200;
		code.characteristics = 0x60000020; // Code, execute read
		output->Write(&code, sizeof(code));

		PeSection data;
		memset(&data, 0, sizeof(data));
		strcpy(data.name, ".data");
		data.virtualSize = opt.sizeOfInitData;
		data.virtualAddress = sizeOfCode + 0x1000;
		data.sizeOfRawData = (dataSection->len + 0x1ff) & (~0x1ff);
		data.pointerToRawData = code.sizeOfRawData + 0x200;
		data.characteristics = 0xc0000040; // Initialized data, read/write
		output->Write(&data, sizeof(data));
	}

	output->Write(padding, 0x200 - output->len);

	output->Write(codeSection->code, codeSection->len);
	if (codeSection->len & 0x1ff)
		output->Write(padding, 0x200 - (codeSection->len & 0x1ff));
	output->Write(dataSection->code, dataSection->len);
	if (dataSection->len & 0x1ff)
		output->Write(padding, 0x200 - (dataSection->len & 0x1ff));
	return true;
}


uint64_t AdjustBaseForPeFile(uint64_t fileBase, const Settings& settings)
{
	return fileBase + 0x1000;
}


uint64_t AdjustDataSectionBaseForPeFile(uint64_t base)
{
	if (base & 0xfff)
		base += 0x1000 - (base & 0xfff);
	return base;
}

