size_t lseek(int fd, size_t offset, int whence)
{
	return __syscall(SYS_lseek, fd, offset, whence);
}

int open(const char* file, int flags, int mode)
{
	return __syscall(SYS_open, file, flags, mode);
}

int close(int fd)
{
	return __syscall(SYS_close, fd);
}

int dup(int fd)
{
	return __syscall(SYS_dup, fd);
}

int dup2(int oldFd, int newFd)
{
	return __syscall(SYS_dup2, oldFd, newFd);
}

ssize_t read(int fd, void* buf, size_t count)
{
	return __syscall(SYS_read, fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
	return __syscall(SYS_write, fd, buf, count);
}

ssize_t sendfile(int outFd, int inFd, size_t* offset, size_t count)
{
	return __syscall(SYS_sendfile, outFd, inFd, offset, count);
}

