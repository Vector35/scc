// Copyright (c) 2011-2012 Rusty Wagner
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

#include "runtime/posix/process.h"

#define stdin  (FILE*)0
#define stdout (FILE*)1
#define stderr (FILE*)2

#define EOF    -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef void* FILE;
typedef int mode_t;

int open(const char* file, int flags, int mode);
int close(int fd);
int dup(int fd);
int dup2(int oldFd, int newFd);

size_t lseek(int fd, size_t offset, int whence);
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
int fchdir(int fd);
int rmdir(const char* path);
int mkdir(const char* path, mode_t mode);
int unlink(const char* path);
int rename(const char* oldpath, const char* newpath);

int chown(const char* path, uid_t owner, gid_t group);
int fchown(int fd, uid_t owner, gid_t group);
int lchown(const char* path, uid_t owner, gid_t group);
int chmod(const char* path, mode_t mode);
int fchmod(int fd, mode_t mode);

ssize_t readlink(const char* path, char* buf, size_t size);

#endif

