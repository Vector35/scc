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

#ifndef __LIBC__LINUX_FILE_H__
#define __LIBC__LINUX_FILE_H__

#define O_RDONLY   0
#define O_WRONLY   1
#define O_RDWR     2
#define O_ACCMODE  3
#define O_CREAT    0x40
#define O_EXCL     0x80
#define O_NOCTTY   0x100
#define O_TRUNC    0x200
#define O_APPEND   0x400
#define O_NONBLOCK 0x800
#define O_DSYNC    0x1000
#define FASYNC     0x2000
#define O_NOATIME  0x40000
#define O_CLOEXEC  0x80000

#define F_DUPFD         0  // duplicate file descriptor
#define F_GETFD         1  // get file descriptor flags
#define F_SETFD         2  // set file descriptor flags
#define F_GETFL         3  // get file status flags
#define F_SETFL         4  // set file status flags
#define F_GETLK         5  // get record locking information
#define F_SETLK         6  // set record locking information
#define F_SETLKW        7  // F_SETLK; wait if blocked
#define F_GETOWN        8  // get SIGIO/SIGURG proc/pgrp
#define F_SETOWN        9  // set SIGIO/SIGURG proc/pgrp
#define F_SETSIG        10
#define F_GETSIG        11
#define F_SETOWN_EX     15
#define F_GETOWN_EX     16
#define F_SETLEASE      1024
#define F_GETLEASE      1025
#define F_NOTIFY        1026
#define F_DUPFD_CLOEXEC 1030
#define F_SETPIPE_SZ    1031
#define F_GETPIPE_SZ    1032

#define FD_CLOEXEC 1

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2
#define F_EXLCK 4
#define F_SHLCK 8

struct dirent
{
	size_t d_ino;
	size_t d_off;
	uint16_t d_reclen;
	char d_name[1];
};

size_t lseek(int fd, ssize_t offset, int whence);
int truncate(const char* path, ssize_t length);
int ftruncate(int fd, ssize_t length);

ssize_t sendfile(int outFd, int inFd, size_t* offset, size_t count);
int getdents(int fd, struct dirent* dirp, size_t count);

#endif
