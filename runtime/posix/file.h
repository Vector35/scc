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
#include "runtime/posix/time.h"

#define stdin  (FILE*)0
#define stdout (FILE*)1
#define stderr (FILE*)2
#define fdopen(fd) ((FILE*)(fd))

#define EOF    -1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FD_SETSIZE  1024

#define S_IXOTH     0x1
#define S_IWOTH     0x2
#define S_IROTH     0x4
#define S_IRWXO     0x7
#define S_IXGRP     0x8
#define S_IWGRP     0x10
#define S_IRGRP     0x20
#define S_IRWXG     0x38
#define S_IXUSR     0x40
#define S_IWUSR     0x80
#define S_IRUSR     0x100
#define S_IRWXU     0x1c0
#define S_ISVTX     0x200
#define S_ISGID     0x400
#define S_ISUID     0x800
#define S_IFIFO     0x1000
#define S_IFCHR     0x2000
#define S_IFDIR     0x4000
#define S_IFBLK     0x6000
#define S_IFREG     0x8000
#define S_IFLNK     0xa000
#define S_IFSOCK    0xc000
#define S_IFMT      0xf000

#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#define S_ISWHT(m)  (((m) & S_IFMT) == 0xe000)

#define FD_ZERO(fd) memset(fd, 0, sizeof(fd_set))
#define FD_SET(d, set) (set)->fds_bits[(d) >> 5] |= (uint32_t)(1 << ((d) & 31))
#define FD_CLR(d, set) (set)->fds_bits[(d) >> 5] &= (uint32_t)~(1 << ((d) & 31))
#define FD_ISSET(d, set) ((set)->fds_bits[(d) >> 5] & (1 << ((d) & 31)))

typedef void* FILE;
typedef int mode_t;

typedef struct
{
	uint32_t fds_bits[FD_SETSIZE / sizeof(uint32_t)];
} fd_set;

int open(const char* file, int flags, int mode);
int close(int fd);
int dup(int fd);
int dup2(int oldFd, int newFd);
void redirect_io(int fd);

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
int link(const char* target, const char* path);
int symlink(const char* target, const char* path);

int pipe(int* fds);

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds, struct timeval* timeout);

#endif

