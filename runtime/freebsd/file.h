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

#ifndef __LIBC__FREEBSD_FILE_H__
#define __LIBC__FREEBSD_FILE_H__

#include "runtime/posix/time.h"

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_ACCMODE   3
#define O_NONBLOCK  4
#define O_APPEND    8
#define O_SHLOCK    0x10
#define O_EXLOCK    0x20
#define O_ASYNC     0x40
#define O_FSYNC     0x80
#define O_SYNC      0x80
#define O_NOFOLLOW  0x100
#define O_CREAT     0x200
#define O_TRUNC     0x400
#define O_EXCL      0x800
#define O_NOCTTY    0x8000
#define O_DIRECT    0x10000
#define O_DIRECTORY 0x20000
#define O_EXEC      0x40000
#define O_TTY_INIT  0x80000
#define O_CLOEXEC   0x100000

#define F_DUPFD         0  // duplicate file descriptor
#define F_GETFD         1  // get file descriptor flags
#define F_SETFD         2  // set file descriptor flags
#define F_GETFL         3  // get file status flags
#define F_SETFL         4  // set file status flags
#define F_GETOWN        5  // get SIGIO/SIGURG proc/pgrp
#define F_SETOWN        6  // set SIGIO/SIGURG proc/pgrp
#define F_OGETLK        7
#define F_OSETLK        8
#define F_OSETLKW       9
#define F_DUP2FD        10
#define F_GETLK         11 // get record locking information
#define F_SETLK         12 // set record locking information
#define F_SETLKW        13 // F_SETLK; wait if blocked
#define F_SETLK_REMOTE  14
#define F_READAHEAD     15
#define F_RDAHEAD       16

#define FD_CLOEXEC      1

#define F_RDLCK         1
#define F_UNLCK         2
#define F_WRLCK         3
#define F_UNLCKSYS      4
#define F_CANCEL        5

#define S_ISTXT     0x200

#define UF_SETTABLE  0xffff
#define UF_NODUMP    0x0001
#define UF_IMMUTABLE 0x0002
#define UF_APPEND    0x0004
#define UF_OPAQUE    0x0008
#define UF_NOUNLINK  0x0010
#define SF_SETTABLE  0xffff0000
#define SF_ARCHIVED  0x00010000
#define SF_IMMUTABLE 0x00020000
#define SF_APPEND    0x00040000
#define SF_NOUNLINK  0x00100000
#define SF_SNAPSHOT  0x00200000

struct stat
{
	uint32_t st_dev;
	uint32_t st_ino;
	uint16_t st_mode;
	uint16_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	uint32_t st_rdev;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	int64_t st_size;
	uint64_t st_blocks;
	uint32_t st_blksize;
	uint32_t st_flags;
	uint32_t st_gen;
	int32_t st_lspare;
	struct timespec st_birthtim;
#ifdef i386
	int32_t __padding1, __padding2;
#endif
};

struct dirent
{
	uint32_t d_fileno;
	uint16_t d_reclen;
	uint8_t d_type;
	uint8_t d_namlen;
	char d_name[257];
};

size_t lseek(int fd, int64_t offset, int whence);
int truncate(const char* path, int64_t length);
int ftruncate(int fd, int64_t length);

ssize_t sendfile(int outFd, int inFd, size_t* offset, size_t count);
int getdents(int fd, struct dirent* dirp, size_t count);
int getdirentries(int fd, struct dirent* dirp, size_t count, ssize_t* basep);

int fstat(int fd, struct stat* buf);
int stat(const char* path, struct stat* buf);
int lstat(const char* path, struct stat* buf);

int chflags(const char* path, int flags);
int fchflags(int fd, int flags);
int lchflags(const char* path, int flags);

int shm_open(const char *name, int oflag, mode_t mode);
int shm_unlink(const char *name);

#endif

