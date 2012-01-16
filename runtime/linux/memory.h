#ifndef __LIBC__LINUX_MEMORY_H__
#define __LIBC__LINUX_MEMORY_H__

#define PROT_READ         1
#define PROT_WRITE        2
#define PROT_EXEC         4
#define PROT_SEM          8
#define PROT_NONE         0
#define PROT_GROWSDOWN    0x1000000
#define PROT_GROWSUP      0x2000000

#define MAP_SHARED        1
#define MAP_PRIVATE       2
#define MAP_COPY          2
#define MAP_TYPE          0xf
#define MAP_FIXED         0x10
#define MAP_ANON          0x20
#define MAP_ANONYMOUS     0x20
#define MAP_UNINITIALIZED 0x4000000

#endif

