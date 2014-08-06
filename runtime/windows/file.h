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

#ifndef __LIBC__FILE_H__
#define __LIBC__FILE_H__

#define STD_INPUT_HANDLE (-10)
#define STD_OUTPUT_HANDLE (-11)
#define STD_ERROR_HANDLE (-12)

#define stdin  (FILE*)(GetStdHandle(STD_INPUT_HANDLE))
#define stdout (FILE*)(GetStdHandle(STD_OUTPUT_HANDLE))
#define stderr (FILE*)(GetStdHandle(STD_ERROR_HANDLE))
#define fdopen(fd) ((FILE*)(fd))

#define EOF    -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define GENERIC_READ    0x80000000
#define GENERIC_WRITE   0x40000000
#define GENERIC_EXECUTE 0x20000000
#define GENERIC_ALL     0x10000000

#define FILE_SHARE_DELETE 4
#define FILE_SHARE_READ   1
#define FILE_SHARE_WRITE  2

#define CREATE_ALWAYS     2
#define CREATE_NEW        1
#define OPEN_ALWAYS       4
#define OPEN_EXISTING     3
#define TRUNCATE_EXISTING 5

#define FILE_ATTRIBUTE_READONLY            1
#define FILE_ATTRIBUTE_HIDDEN              2
#define FILE_ATTRIBUTE_SYSTEM              4
#define FILE_ATTRIBUTE_DIRECTORY           0x10
#define FILE_ATTRIBUTE_ARCHIVE             0x20
#define FILE_ATTRIBUTE_DEVICE              0x40
#define FILE_ATTRIBUTE_NORMAL              0x80
#define FILE_ATTRIBUTE_TEMPORARY           0x100
#define FILE_ATTRIBUTE_SPARSE_FILE         0x200
#define FILE_ATTRIBUTE_REPARSE_POINT       0x400
#define FILE_ATTRIBUTE_COMPRESSED          0x800
#define FILE_ATTRIBUTE_OFFLINE             0x1000
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x2000
#define FILE_ATTRIBUTE_ENCRYPTED           0x4000
#define FILE_ATTRIBUTE_INTEGRITY_STREAM    0x8000
#define FILE_ATTRIBUTE_VIRTUAL             0x10000
#define FILE_ATTRIBUTE_NO_SCRUB_DATA       0x20000
#define FILE_FLAG_OPEN_NO_RECALL           0x100000
#define FILE_FLAG_OPEN_REPARSE_POINT       0x200000
#define FILE_FLAG_SESSION_AWARE            0x800000
#define FILE_FLAG_POSIX_SEMANTICS          0x1000000
#define FILE_FLAG_BACKUP_SEMANTICS         0x2000000
#define FILE_FLAG_DELETE_ON_CLOSE          0x4000000
#define FILE_FLAG_SEQUENTIAL_SCAN          0x8000000
#define FILE_FLAG_RANDOM_ACCESS            0x10000000
#define FILE_FLAG_NO_BUFFERING             0x20000000
#define FILE_FLAG_OVERLAPPED               0x40000000
#define FILE_FLAG_WRITE_THROUGH            0x80000000

#define O_RDONLY    GENERIC_READ
#define O_WRONLY    GENERIC_WRITE
#define O_RDWR      (GENERIC_READ | GENERIC_WRITE)
#define O_CREAT     0x10
#define O_EXCL      0x20
#define O_TRUNC     0x40
#define O_APPEND    0

#define INVALID_HANDLE_VALUE    ((HANDLE)-1)
#define INVALID_FILE_SIZE       0xffffffff
#define INVALID_FILE_ATTRIBUTES 0xffffffff

#define MAX_PATH 260

#define GetFileExInfoStandard 0

typedef void* FILE;
typedef void* HANDLE;
typedef int mode_t;

typedef struct
{
	uint32_t dwLowDateTime;
	uint32_t dwHighDateTime;
} FILETIME;

typedef struct
{
	uint32_t dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	uint32_t nFileSizeHigh;
	uint32_t nFileSizeLow;
	uint32_t dwReserved0;
	uint32_t dwReserved1;
	char cFileName[MAX_PATH];
	char cAlternateFileName[14];
} WIN32_FIND_DATAA;

typedef struct
{
	uint32_t dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	uint32_t nFileSizeHigh;
	uint32_t nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;

typedef struct
{
	uint32_t dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	uint32_t dwVolumeSerialNumber;
	uint32_t nFileSizeHigh;
	uint32_t nFileSizeLow;
	uint32_t nNumberOfLinks;
	uint32_t nFileIndexHigh;
	uint32_t nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION;

HANDLE __stdcall GetStdHandle(uint32_t type) __import("kernel32");
HANDLE __stdcall CreateFileA(const char* name, uint32_t access, uint32_t share, void* security, uint32_t disposition,
	uint32_t flags, HANDLE templateFile) __import("kernel32");
bool __stdcall CloseHandle(HANDLE handle) __import("kernel32");
bool __stdcall ReadFile(HANDLE file, void* buf, uint32_t bytesToRead, uint32_t* bytesRead, void* overlapped) __import("kernel32");
bool __stdcall WriteFile(HANDLE file, const void* buf, uint32_t bytesToWrite, uint32_t* bytesWritten,
	void* overlapped) __import("kernel32");
bool __stdcall DeleteFileA(const char* name) __import("kernel32");
bool __stdcall MoveFileA(const char* from, const char* to) __import("kernel32");
uint32_t __stdcall SetFilePointer(HANDLE file, int32_t dist, int32_t* distHigh, uint32_t method) __import("kernel32");
bool __stdcall SetEndOfFile(HANDLE file) __import("kernel32");
bool __stdcall SetCurrentDirectoryA(const char* name) __import("kernel32");
bool __stdcall CreateDirectoryA(const char* name, void* security) __import("kernel32");
bool __stdcall RemoveDirectoryA(const char* name) __import("kernel32");
uint32_t __stdcall GetCurrentDirectoryA(uint32_t len, char* buf) __import("kernel32");
HANDLE __stdcall FindFirstFileA(const char* name, WIN32_FIND_DATAA* data) __import("kernel32");
bool __stdcall FindNextFileA(HANDLE find, WIN32_FIND_DATAA* data) __import("kernel32");
bool __stdcall FindClose(HANDLE find) __import("kernel32");
uint32_t __stdcall GetFileAttributesA(const char* name) __import("kernel32");
bool __stdcall GetFileAttributesExA(const char* name, uint32_t level, void* info) __import("kernel32");
uint32_t __stdcall GetFileSize(HANDLE file, uint32_t* high) __import("kernel32");
bool __stdcall GetFileInformationByHandle(HANDLE file, BY_HANDLE_FILE_INFORMATION* info) __import("kernel32");
bool __stdcall CreatePipe(HANDLE* read, HANDLE* write, void* security, uint32_t size) __import("kernel32");

int open(const char* file, uint32_t flags, int mode);
int close(int fd);

ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);

int fputc(int ch, FILE* fp);
int fputs(const char* str, FILE* fp);
int puts(const char* str);

int fgetc(FILE* fp);
char* fgets(char* result, int size, FILE* fp);

int printf(const char* fmt, ...);
int fprintf(FILE* fp, const char* fmt, ...);
int vprintf(const char* fmt, va_list va);
int vfprintf(FILE* fp, const char* fmt, va_list va);

int chdir(const char* path);
char* getcwd(char* buf, size_t size);
int rmdir(const char* path);
int mkdir(const char* path, mode_t mode);
int unlink(const char* path);
int rename(const char* oldpath, const char* newpath);

size_t lseek(int fd, ssize_t offset, int whence);
int ftruncate(int fd, ssize_t length);

int pipe(int* fds);

#endif

