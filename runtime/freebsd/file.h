#ifndef __LIBC__FREEBSD_FILE_H__
#define __LIBC__FREEBSD_FILE_H__

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

ssize_t sendfile(int outFd, int inFd, size_t* offset, size_t count);

#endif

