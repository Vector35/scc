int fstat(int fd, struct stat* buf)
{
	return __syscall(SYS_fstat, fd, buf);
}

int stat(const char* path, struct stat* buf)
{
	return __syscall(SYS_fstatat, AT_FDCWD, path, buf, 0);
}

int lstat(const char* path, struct stat* buf)
{
	return __syscall(SYS_fstatat, AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}

