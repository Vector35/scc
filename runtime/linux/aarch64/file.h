#ifndef __LIBC__LINUX_ARCH_FILE_H__
#define __LIBC__LINUX_ARCH_FILE_H__

#define AT_FDCWD            -100
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR        0x200
#define AT_SYMLINK_FOLLOW   0x400

#define O_DIRECTORY 0x4000
#define O_NOFOLLOW  0x8000
#define O_DIRECT    0x10000
#define O_LARGEFILE 0x20000

#endif
