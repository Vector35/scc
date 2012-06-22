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

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_ACCMODE   3
#define O_CREAT     0x40
#define O_EXCL      0x80
#define O_NOCTTY    0x100
#define O_TRUNC     0x200
#define O_APPEND    0x400
#define O_NONBLOCK  0x800
#define O_DSYNC     0x1000
#define FASYNC      0x2000
#define O_DIRECT    0x4000
#define O_LARGEFILE 0x8000
#define O_DIRECTORY 0x10000
#define O_NOFOLLOW  0x20000
#define O_NOATIME   0x40000
#define O_CLOEXEC   0x80000

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

ssize_t sendfile(int outFd, int inFd, size_t* offset, size_t count);

#endif

