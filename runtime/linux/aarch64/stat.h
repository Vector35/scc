#ifndef __LIBC__ARM_STAT_H__
#define __LIBC__ARM_STAT_H__

struct stat
{
	uint64_t st_dev;
	uint64_t st_ino;

	uint32_t st_mode;
	uint32_t st_nlink;

	uint32_t st_uid;
	uint32_t st_gid;

	uint64_t st_rdev;
	uint64_t __pad1;

	uint64_t st_size;
	uint32_t st_blksize;
	uint32_t __pad2;

	uint64_t st_blocks;

	uint64_t st_atime;
	uint64_t st_atime_nsec;
	uint64_t st_mtime;
	uint64_t st_mtime_nsec;
	uint64_t st_ctime;
	uint64_t st_ctime_nsec;

	uint32_t __unused4;
	uint32_t __unused5;
};

int fstat(int fd, struct stat* buf);
int stat(const char* path, struct stat* buf);
int lstat(const char* path, struct stat* buf);

#endif
