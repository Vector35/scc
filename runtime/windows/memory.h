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

#ifndef __LIBC__MEMORY_H__
#define __LIBC__MEMORY_H__

#include "runtime/windows/file.h"

#define PAGE_NOACCESS          1
#define PAGE_READONLY          2
#define PAGE_READWRITE         4
#define PAGE_WRITECOPY         8
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD             0x100
#define PAGE_NOCACHE           0x200
#define PAGE_WRITECOMBINE      0x400

#define MEM_COMMIT      0x1000
#define MEM_RESERVE     0x2000
#define MEM_DECOMMIT    0x4000
#define MEM_RELEASE     0x8000
#define MEM_RESET       0x80000
#define MEM_TOP_DOWN    0x100000
#define MEM_WRITE_WATCH 0x200000
#define MEM_PHYSICAL    0x400000
#define MEM_RESET_UNDO  0x1000000
#define MEM_LARGE_PAGES 0x20000000

#define FILE_MAP_COPY       1
#define FILE_MAP_WRITE      2
#define FILE_MAP_READ       4
#define FILE_MAP_EXECUTE    0x20
#define FILE_MAP_ALL_ACCESS 0xf001f

#define HeapAlloc RtlAllocateHeap
#define HeapFree  RtlFreeHeap

void* __stdcall GetProcessHeap() __import("kernel32");
void* __stdcall RtlAllocateHeap(void* heap, uint32_t flags, size_t bytes) __import("ntdll");
void __stdcall RtlFreeHeap(void* heap, uint32_t flags, void* ptr) __import("ntdll");
void* __stdcall VirtualAlloc(void* addr, size_t size, uint32_t alloc, uint32_t protect)
    __import("kernel32");
bool __stdcall VirtualFree(void* addr, size_t size, uint32_t type) __import("kernel32");
bool __stdcall VirtualProtect(void* addr, size_t size, uint32_t prot, uint32_t* oldProt)
    __import("kernel32");
HANDLE __stdcall CreateFileMappingA(HANDLE file, void* security, uint32_t prot,
    uint32_t maxSizeHigh, uint32_t maxSizeLow, const char* name) __import("kernel32");
void* __stdcall MapViewOfFile(HANDLE mapping, uint32_t access, uint32_t offsetHigh,
    uint32_t offsetLow, size_t size) __import("kernel32");
bool __stdcall UnmapViewOfFile(void* addr) __import("kernel32");

void* malloc(size_t len);
void free(void* ptr);

char* strdup(const char* str);

#endif
