// Copyright (c) 2012 Rusty Wagner
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

#ifndef __LIBC__MIPS_STAT_H__
#define __LIBC__MIPS_STAT_H__

struct stat
{
	uint32_t st_dev;
	uint32_t __pad0;
	uint32_t __pad1;
	uint32_t __pad2;
	uint64_t st_ino;
	uint32_t st_mode;
	uint32_t st_nlink;
	uint32_t st_uid;
	uint32_t st_gid;
	uint32_t st_rdev;
	uint32_t __pad3;
	uint32_t __pad4;
	uint32_t __pad5;
	uint64_t st_size;
	uint32_t st_atime;
	uint32_t st_atime_nsec;
	uint32_t st_mtime;
	uint32_t st_mtime_nsec;
	uint32_t st_ctime;
	uint32_t st_ctime_nsec;
	uint32_t st_blksize;
	uint32_t __pad6;
	uint64_t st_blocks;
};

int fstat(int fd, struct stat* buf);
int stat(const char* path, struct stat* buf);
int lstat(const char* path, struct stat* buf);

#endif

