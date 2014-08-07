// Copyright (c) 2014 Rusty Wagner
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

#ifndef __LIBC__LOADER_H__
#define __LIBC__LOADER_H__

#define DONT_RESOLVE_DLL_REFERENCES         1
#define LOAD_LIBRARY_AS_DATAFILE            2
#define LOAD_WITH_ALTERED_SEARCH_PATH       8
#define LOAD_IGNORE_CODE_AUTHZ_LEVEL        0x10
#define LOAD_LIBRARY_AS_IMAGE_RESOURCE      0x20
#define LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE  0x40
#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR    0x100
#define LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0x200
#define LOAD_LIBRARY_SEARCH_USER_DIRS       0x400
#define LOAD_LIBRARY_SEARCH_SYSTEM32        0x800
#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS    0x1000

typedef struct
{
	uint16_t e_magic;
	uint16_t e_cblp;
	uint16_t e_cp;
	uint16_t e_crlc;
	uint16_t e_cparhdr;
	uint16_t e_minalloc;
	uint16_t e_maxalloc;
	uint16_t e_ss, e_sp;
	uint16_t e_csum;
	uint16_t e_ip, e_cs;
	uint16_t e_lfarlc;
	uint16_t e_ovno;
	uint16_t e_res[4];
	uint16_t e_oemid;
	uint16_t e_oeminfo;
	uint16_t e_res2[10];
	uint32_t e_lfanew;
} IMAGE_DOS_HEADER;

typedef struct
{
	uint32_t Signature;

	uint16_t Machine;
	uint16_t NumberOfSections;
	uint32_t TimeDateStamp;
	uint32_t PointerToSymbolTable;
	uint32_t NumberOfSymbols;
	uint16_t SizeOfOptionalHeader;
	uint16_t Characteristics;

	uint16_t Magic;
	uint8_t MajorLinkerVersion;
	uint8_t MinorLinkerVersion;
	uint32_t SizeOfCode;
	uint32_t SizeOfInitializedData;
	uint32_t SizeOfUninitializedData;
	uint32_t AddressOfEntryPoint;
	uint32_t BaseOfCode;
#ifdef __32BIT
	uint32_t BaseOfData;
	uint32_t ImageBase;
#else
	uint64_t ImageBase;
#endif
	uint32_t SectionAlignment;
	uint32_t FileAlignment;
	uint16_t MajorOperatingSystemVersion;
	uint16_t MinorOperatingSystemVersion;
	uint16_t MajorImageVersion;
	uint16_t MinorImageVersion;
	uint16_t MajorSubsystemVersion;
	uint16_t MinorSubsystemVersion;
	uint32_t Win32VersionValue;
	uint32_t SizeOfImage;
	uint32_t SizeOfHeaders;
	uint32_t CheckSum;
	uint16_t Subsystem;
	uint16_t DllCharacteristics;
#ifdef __32BIT
	uint32_t SizeOfStackReserve;
	uint32_t SizeOfStackCommit;
	uint32_t SizeOfHeapReserve;
	uint32_t SizeOfHeapCommit;
#else
	uint64_t SizeOfStackReserve;
	uint64_t SizeOfStackCommit;
	uint64_t SizeOfHeapReserve;
	uint64_t SizeOfHeapCommit;
#endif
	uint32_t LoaderFlags;
	uint32_t NumberOfRvaAndSizes;

	uint32_t ExportDirectoryVirtualAddress;
	uint32_t ExportDirectorySize;
	uint32_t ImportDirectoryVirtualAddress;
	uint32_t ImportDirectorySize;
} IMAGE_NT_HEADERS;

typedef struct
{
	char Name[8];
	uint32_t VirtualSize;
	uint32_t VirtualAddress;
	uint32_t SizeOfRawData;
	uint32_t PointerToRawData;
	uint32_t PointerToRelocations;
	uint32_t PointerToLinenumbers;
	uint16_t NumberOfRelocations;
	uint16_t NumberOfLinenumbers;
	uint32_t Characteristics;
} IMAGE_SECTION_HEADER;

typedef struct
{
	uint32_t Characteristics;
	uint32_t TimeDateStamp;
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	uint32_t Name;
	uint32_t Base;
	uint32_t NumberOfFunctions;
	uint32_t NumberOfNames;
	uint32_t AddressOfFunctions;
	uint32_t AddressOfNames;
	uint32_t AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY;

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY* Flink;
	struct _LIST_ENTRY* Blink;
} LIST_ENTRY;

typedef struct
{
	uint32_t Length;
	uint32_t Initialized;
	void* SsHandle;
	LIST_ENTRY* InLoadOrderModuleList;
	LIST_ENTRY* InLoadOrderModuleListBack;
	LIST_ENTRY* InMemoryOrderModuleList;
	LIST_ENTRY* InMemoryOrderModuleListBack;
	LIST_ENTRY* InInitializerOrderModuleList;
	LIST_ENTRY* InInitializerOrderModuleListBack;
} PEB_LDR_DATA;

typedef struct
{
	uint8_t InheritedAddressSpace;
	uint8_t ReadImageFileExecOptions;
	uint8_t BeingDebugged;
	uint8_t SpareBool;
	void* Mutant;
	void* ImageBaseAddress;
	PEB_LDR_DATA* Ldr;
	void* ProcessParameters;
	void* SubSystemData;
	void* ProcessHeap;
} PEB;

typedef struct _TEB
{
	void* ExceptionList;
	void* StackBase;
	void* StackLimit;
	void* SubSystemTib;
	union
	{
		void* FiberData;
		uint32_t Version;
	};
	void* ArbitraryUserPointer;
	struct _TEB* Self;
	void* EnvironmentPointer;
	void* UniqueProcess;
	void* UniqueThread;
	void* ActiveRpcHandle;
	void* ThreadLocalStoragePointer;
	PEB* ProcessEnvironmentBlock;
} TEB;

typedef struct
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	void* DllBase;
	void* EntryPoint;
	size_t SizeOfImage;
	uint16_t FullDllNameLength;
	uint16_t FullDllNameMaximumLength;
	uint16_t* FullDllNameBuffer;
	uint16_t BaseDllNameLength;
	uint16_t BaseDllNameMaximumLength;
	uint16_t* BaseDllNameBuffer;
} LDR_DATA_TABLE_ENTRY;

#define GetCurrentTeb() ((TEB*)__teb)
#define GetCurrentPeb() ((PEB*)__peb)

void* __stdcall LoadLibraryA(const char* name) __import("kernel32");
void* __stdcall LoadLibraryExA(const char* name, void* reserved, uint32_t flags) __import("kernel32");
void* __stdcall GetModuleHandleA(const char* name) __import("kernel32");
void* __stdcall GetProcAddress(void* module, const char* name) __import("kernel32");

void __resolve_imports_GetModuleHandle(const uint8_t* importDesc, void*** iats);
void __resolve_imports_LoadLibrary(const uint8_t* importDesc, void*** iats);
void __resolve_imports_LoadLibraryEx(const uint8_t* importDesc, void*** iats);
void __resolve_imports_pebscan(const uint32_t* importDesc, void*** iats);
void __resolve_imports_pebscan_loadlibrary(const uint8_t* importDesc, void*** iats);

#endif

