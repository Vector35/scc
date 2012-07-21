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

#define MAXPATHLEN  1024

#define F_DUPFD                  0  // duplicate file descriptor
#define F_GETFD                  1  // get file descriptor flags
#define F_SETFD                  2  // set file descriptor flags
#define F_GETFL                  3  // get file status flags
#define F_SETFL                  4  // set file status flags
#define F_GETOWN                 5  // get SIGIO/SIGURG proc/pgrp
#define F_SETOWN                 6  // set SIGIO/SIGURG proc/pgrp
#define F_GETLK                  7  // get record locking information
#define F_SETLK                  8  // set record locking information
#define F_SETLKW                 9  // F_SETLK; wait if blocked
#define F_FLUSH_DATA            40
#define F_CHKCLEAN              41  // Used for regression test
#define F_PREALLOCATE           42  // Preallocate storage
#define F_SETSIZE               43  // Truncate a file without zeroing space
#define F_RDADVISE              44  // Issue an advisory read async with no copy to user
#define F_RDAHEAD               45  // turn read ahead off/on for this fd
#define F_READBOOTSTRAP         46  // Read bootstrap from disk
#define F_WRITEBOOTSTRAP        47  // Write bootstrap on disk
#define F_NOCACHE               48  // turn data caching off/on for this fd
#define F_LOG2PHYS              49  // file offset to device offset
#define F_GETPATH               50  // return the full path of the fd
#define F_FULLFSYNC             51  // fsync + ask the drive to flush to the media
#define F_PATHPKG_CHECK         52  // find which component (if any) is a package
#define F_FREEZE_FS             53  // "freeze" all fs operations
#define F_THAW_FS               54  // "thaw" all fs operations
#define F_GLOBAL_NOCACHE        55  // turn data caching off/on (globally) for this file
#define F_ADDSIGS               59  // add detached signatures
#define F_MARKDEPENDENCY        60  // this process hosts the device supporting the fs backing this fd
#define F_ADDFILESIGS           61  // add signature from same file (used by dyld for shared libs)
#define F_NODIRECT              62  // used in conjunction with F_NOCACHE to indicate that DIRECT, synchonous writes
#define F_GETPROTECTIONCLASS    63  // Get the protection class of a file from the EA, returns int
#define F_SETPROTECTIONCLASS    64  // Set the protection class of a file for the EA, requires int
#define F_LOG2PHYS_EXT          65  // file offset to device offset, extended
#define F_GETLKPID              66  // get record locking information, per-process
#define F_SETBACKINGSTORE       70  // Mark the file as being the backing store for another filesystem
#define F_GETPATH_MTMINFO       71  // return the full path of the FD, but error in specific mtmd circumstances
#define F_SETNOSIGPIPE          73  // No SIGPIPE generated on EPIPE
#define F_GETNOSIGPIPE          74  // Status of SIGPIPE for this fd

#define FD_CLOEXEC   1

#define F_RDLCK      1
#define F_UNLCK      2
#define F_WRLCK      3

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

struct dirent
{
	uint32_t d_fileno;
	uint16_t d_reclen;
	uint8_t d_type;
	uint8_t d_namlen;
	char d_name[256];
};

size_t lseek(int fd, int64_t offset, int whence);
int truncate(const char* path, int64_t length);
int ftruncate(int fd, int64_t length);

int getdents(int fd, struct dirent* dirp, size_t count);
int getdirentries(int fd, struct dirent* dirp, size_t count, ssize_t* basep);

int fstat(int fd, struct stat* buf);
int stat(const char* path, struct stat* buf);
int lstat(const char* path, struct stat* buf);

int shm_open(const char *name, int oflag, mode_t mode);
int shm_unlink(const char *name);

#endif

