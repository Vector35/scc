ssize_t read(int fd, void* buf, size_t count)
{
	return __syscall(SYS_read, fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
	return __syscall(SYS_write, fd, buf, count);
}

