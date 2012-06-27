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

#ifndef __LIBC__MAC_FILE_H__
#define __LIBC__MAC_FILE_H__

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
#define O_EVTONLY   0x8000
#define O_NOCTTY    0x20000
#define O_DIRECTORY 0x100000
#define O_SYMLINK   0x200000
#define O_DSYNC     0x400000
#define O_CLOEXEC   0x1000000

struct stat
{
	uint32_t st_dev;
	uint32_t st_ino;
	uint16_t st_mode;
	uint16_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	uint32_t st_rdev;
	struct timespec st_atimespec;
	struct timespec st_mtimespec;
	struct timespec st_ctimespec;
	int64_t st_size;
	uint64_t st_blocks;
	uint32_t st_blksize;
	uint32_t st_flags;
	uint32_t st_gen;
	int32_t st_lspare;
	int64_t st_qspare[2];
};

size_t lseek(int fd, int64_t offset, int whence);

int fstat(int fd, struct stat* buf);
int stat(const char* path, struct stat* buf);
int lstat(const char* path, struct stat* buf);

#endif

