#include <stdlib.h>
#include <string.h>
#include "ElfOutput.h"
#include "Struct.h"


struct MachIdent
{
	uint32_t signature;
	uint32_t cpuType;
	uint32_t cpuSubType;
	uint32_t fileType;
	uint32_t cmds;
	uint32_t cmdSize;
	uint32_t flags;
};

struct MachIdent64
{
	MachIdent ident;
	uint32_t reserved;
};

struct MachCommandHeader
{
	uint32_t cmd;
	uint32_t size;
};

struct MachSegment32
{
	MachCommandHeader header;
	char name[16];
	uint32_t virtualAddress;
	uint32_t memorySize;
	uint32_t fileOffset;
	uint32_t fileSize;
	uint32_t maxProtect;
	uint32_t initProtect;
	uint32_t sections;
	uint32_t flags;
};

struct MachSegment64
{
	MachCommandHeader header;
	char name[16];
	uint64_t virtualAddress;
	uint64_t memorySize;
	uint64_t fileOffset;
	uint64_t fileSize;
	uint32_t maxProtect;
	uint32_t initProtect;
	uint32_t sections;
	uint32_t flags;
};

struct MachThreadX86
{
	MachCommandHeader header;
	uint32_t flavor, count;
	uint32_t eax, ebx, ecx, edx, edi, esi, ebp, esp, ss, eflags;
	uint32_t eip, cs, ds, es, fs, gs;
};

struct MachThreadX64
{
	MachCommandHeader header;
	uint32_t flavor, count;
	uint64_t rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t rip, rflags, cs, fs, gs;
};


bool GenerateMachOFile(OutputBlock* output, const Settings& settings, OutputBlock* codeSection, OutputBlock* dataSection)
{
	// Output file identification header
	MachIdent ident;
	if (settings.preferredBits == 64)
		ident.signature = 0xfeedfacf;
	else
		ident.signature = 0xfeedface;

	if (settings.sharedLibrary)
		ident.fileType = 6; // DYLIB
	else
		ident.fileType = 2; // EXECUTE

	size_t threadSize = 0;
	switch (settings.architecture)
	{
	case ARCH_X86:
		ident.cpuType = 7; // x86
		ident.cpuSubType = 3;
		if (settings.preferredBits == 32)
			threadSize = sizeof(MachThreadX86);
		else
		{
			ident.cpuType |= 0x1000000;
			threadSize = sizeof(MachThreadX64);
		}
		break;
	default:
		return false;
	}

	ident.flags = 1; // No undefined references
	if (settings.execStack)
		ident.flags |= 0x20000; // ALLOW_STACK_EXECUTION
	if (settings.positionIndependent)
		ident.flags |= 0x200000; // PIE

	ident.cmds = 4; // Three segments, one thread
	if (settings.preferredBits == 64)
		ident.cmdSize = (sizeof(MachSegment64) * 3) + threadSize;
	else
		ident.cmdSize = (sizeof(MachSegment32) * 3) + threadSize;

	output->Write(&ident, sizeof(ident));
	if (settings.preferredBits == 64)
	{
		uint32_t reserved = 0;
		output->Write(&reserved, sizeof(reserved));
	}

	// Write out commands
	size_t codePad = 0;
	if (settings.preferredBits == 64)
	{
		MachSegment64 pageZero;
		memset(&pageZero, 0, sizeof(pageZero));
		pageZero.header.cmd = 0x19; // SEGMENT64
		pageZero.header.size = sizeof(pageZero);
		strcpy(pageZero.name, "__PAGE_ZERO");
		pageZero.memorySize = 0x100000;
		output->Write(&pageZero, sizeof(pageZero));

		MachSegment64 code;
		memset(&code, 0, sizeof(code));
		code.header.cmd = 0x19; // SEGMENT64
		code.header.size = sizeof(code);
		strcpy(code.name, "__TEXT");
		code.virtualAddress = settings.base & (~0xfff);
		code.memorySize = (settings.base & 0xfff) + codeSection->len;
		code.fileOffset = 0;
		code.fileSize = code.memorySize;
		code.maxProtect = 5; // Read execute
		code.initProtect = 5; // Read execute
		output->Write(&code, sizeof(code));

		uint64_t dataStart = code.virtualAddress + code.fileSize;
		if (dataStart & 0xfff)
		{
			codePad = 0x1000 - (dataStart & 0xfff);
			dataStart += codePad;
		}

		MachSegment64 data;
		memset(&data, 0, sizeof(data));
		data.header.cmd = 0x19; // SEGMENT64
		data.header.size = sizeof(data);
		strcpy(data.name, "__DATA");
		data.virtualAddress = dataStart;
		data.memorySize = dataSection->len;
		data.fileOffset = data.virtualAddress - code.virtualAddress;
		data.fileSize = data.memorySize;
		data.maxProtect = 3; // Read write
		data.initProtect = 3; // Read write
		output->Write(&data, sizeof(data));

		MachThreadX64 thread;
		memset(&thread, 0, sizeof(thread));
		thread.header.cmd = 5; // UNIXTHREAD
		thread.header.size = sizeof(thread);
		thread.flavor = 4; // x86_THREAD_STATE64
		thread.count = 42;
		thread.rip = settings.base;
		output->Write(&thread, sizeof(thread));
	}
	else
	{
		MachSegment32 pageZero;
		memset(&pageZero, 0, sizeof(pageZero));
		pageZero.header.cmd = 1; // SEGMENT
		pageZero.header.size = sizeof(pageZero);
		strcpy(pageZero.name, "__PAGE_ZERO");
		pageZero.memorySize = 0x1000;
		output->Write(&pageZero, sizeof(pageZero));

		MachSegment32 code;
		memset(&code, 0, sizeof(code));
		code.header.cmd = 1; // SEGMENT
		code.header.size = sizeof(code);
		strcpy(code.name, "__TEXT");
		code.virtualAddress = (uint32_t)settings.base & (~0xfff);
		code.memorySize = (settings.base & 0xfff) + codeSection->len;
		code.fileOffset = 0;
		code.fileSize = code.memorySize;
		code.maxProtect = 5; // Read execute
		code.initProtect = 5; // Read execute
		output->Write(&code, sizeof(code));

		uint32_t dataStart = code.virtualAddress + code.fileSize;
		if (dataStart & 0xfff)
		{
			codePad = 0x1000 - (dataStart & 0xfff);
			dataStart += codePad;
		}

		MachSegment32 data;
		memset(&data, 0, sizeof(data));
		data.header.cmd = 1; // SEGMENT
		data.header.size = sizeof(data);
		strcpy(data.name, "__DATA");
		data.virtualAddress = dataStart;
		data.memorySize = dataSection->len;
		data.fileOffset = data.virtualAddress - code.virtualAddress;
		data.fileSize = data.memorySize;
		data.maxProtect = 3; // Read write
		data.initProtect = 3; // Read write
		output->Write(&data, sizeof(data));

		MachThreadX86 thread;
		memset(&thread, 0, sizeof(thread));
		thread.header.cmd = 5; // UNIXTHREAD
		thread.header.size = sizeof(thread);
		thread.flavor = 1; // x86_THREAD_STATE32
		thread.count = 16;
		thread.eip = (uint32_t)settings.base;
		output->Write(&thread, sizeof(thread));
	}

	output->Write(codeSection->code, codeSection->len);

	if (codePad != 0)
	{
		void* pad = calloc(codePad, 1);
		output->Write(pad, codePad);
		free(pad);
	}

	output->Write(dataSection->code, dataSection->len);
	return true;
}


uint64_t AdjustBaseForMachOFile(uint64_t fileBase, const Settings& settings)
{
	uint64_t base = fileBase;
	if (settings.preferredBits == 64)
	{
		base += sizeof(MachIdent64) + (sizeof(MachSegment64) * 3);
		if (settings.architecture == ARCH_X86)
			base += sizeof(MachThreadX64);
	}
	else
	{
		base += sizeof(MachIdent) + (sizeof(MachSegment32) * 3);
		if (settings.architecture == ARCH_X86)
			base += sizeof(MachThreadX86);
	}
	return base;
}


uint64_t AdjustDataSectionBaseForMachOFile(uint64_t base)
{
	if (base & 0xfff)
		base += 0x1000 - (base & 0xfff);
	return base;
}

