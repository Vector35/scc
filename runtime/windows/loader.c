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

#define IMPLEMENT_RESOLVE_FUNC(name, loadModuleCode) \
void name(const uint8_t* importDesc, void*** iats) \
{ \
	while (*iats != NULL) \
	{ \
		size_t moduleNameLen = *(importDesc++); \
		void* module = loadModuleCode; \
		importDesc += moduleNameLen + 1; \
\
		size_t i = 0; \
		while (true) \
		{ \
			size_t nameLen = *(importDesc++); \
			if (nameLen == 0) \
				break; \
			(*iats)[i++] = GetProcAddress(module, (const char*)importDesc); \
			importDesc += nameLen + 1; \
		} \
\
		iats++; \
	} \
}

IMPLEMENT_RESOLVE_FUNC(__resolve_imports_GetModuleHandle, GetModuleHandleA((const char*)importDesc))
IMPLEMENT_RESOLVE_FUNC(__resolve_imports_LoadLibrary, LoadLibraryA((const char*)importDesc))
IMPLEMENT_RESOLVE_FUNC(__resolve_imports_LoadLibraryEx, LoadLibraryExA((const char*)importDesc, NULL, 0))

void* __find_function_by_pebscan(uint32_t moduleHash, uint32_t funcHash)
{
	for (LDR_DATA_TABLE_ENTRY* module = (LDR_DATA_TABLE_ENTRY*)(GetCurrentPeb()->Ldr->InLoadOrderModuleList); module->DllBase;
		module = (LDR_DATA_TABLE_ENTRY*)(((LIST_ENTRY*)module)->Flink))
	{
		uint32_t hash = 0;
		for (uint16_t i = 0; i < module->BaseDllNameLength; i += 2)
		{
			hash = (hash >> 13) | (hash << 19);
			uint32_t ch = ((char*)module->BaseDllNameBuffer)[i];
			if (ch >= 'a')
				hash += ch - 0x20;
			else
				hash += ch;
		}

		if (moduleHash != hash)
			continue;

		size_t base = (size_t)module->DllBase;

		IMAGE_NT_HEADERS* peHeader = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
		IMAGE_EXPORT_DIRECTORY* exportTable = (IMAGE_EXPORT_DIRECTORY*)(base +
			peHeader->ExportDirectoryVirtualAddress);

		for (uint32_t i = 0; i < exportTable->NumberOfNames; i++)
		{
			char* name = (char*)(base + ((uint32_t*)(base + exportTable->AddressOfNames))[i]);
			hash = 0;
			while (true)
			{
				uint32_t ch = *(name++);
				if (ch == 0)
					break;
				hash = ((hash >> 13) | (hash << 19)) + ch;
			}

			if (funcHash != hash)
				continue;

			return (void*)(base + ((size_t*)(base + exportTable->AddressOfFunctions))
				[((uint16_t*)(base + exportTable->AddressOfNameOrdinals))[i]]);
		}
	}
	return NULL;
}

void __resolve_imports_pebscan(const uint32_t* importDesc, void*** iats)
{
	while (*iats != NULL)
	{
		uint32_t moduleHash = *(importDesc++);

		size_t i;
		while (true)
		{
			uint32_t nameHash = *importDesc;
			void* func = __find_function_by_pebscan(moduleHash, nameHash);
			if (!func)
				break;
			(*iats)[i++] = func;
			importDesc++;
		}

		iats++;
	}
}

void __resolve_imports_pebscan_loadlibrary(const uint8_t* importDesc, void*** iats)
{
	void* (__stdcall *loadLibraryA)(const char*) = (void* (__stdcall*)(const char*))
		__find_function_by_pebscan(0x6e2bca17, 0xec0e4e8e);
	void* (__stdcall *getProcAddress)(void*, const char*) = (void* (__stdcall*)(void*, const char*))
		__find_function_by_pebscan(0x6e2bca17, 0x7c0dfcaa);

	while (*iats != NULL)
	{
		size_t moduleNameLen = *(importDesc++);
		void* module = loadLibraryA((const char*)importDesc);
		importDesc += moduleNameLen + 1;

		size_t i = 0;
		while (true)
		{
			size_t nameLen = *(importDesc++);
			if (nameLen == 0)
				break;
			(*iats)[i++] = getProcAddress(module, (const char*)importDesc);
			importDesc += nameLen + 1;
		}

		iats++;
	}
}

