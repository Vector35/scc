#ifndef __LIBC__FREEBSD_MEMORY_H__
#define __LIBC__FREEBSD_MEMORY_H__

#define PROT_READ         1
#define PROT_WRITE        2
#define PROT_EXEC         4
#define PROT_NONE         0

#define MAP_SHARED        1
#define MAP_PRIVATE       2
#define MAP_COPY          2
#define MAP_FIXED         0x10
#define MAP_RENAME        0x20
#define MAP_NORESERVE     0x40
#define MAP_RESERVED0080  0x80
#define MAP_RESERVED0100  0x100
#define MAP_HASSEMAPHORE  0x200
#define MAP_STACK         0x400
#define MAP_NOSYNC        0x800
#define MAP_ANON          0x1000
#define MAP_ANONYMOUS     0x1000
#define MAP_NOCORE        0x20000

#endif

