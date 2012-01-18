void* mmap(void* addr, size_t len, int prot, int flags, int fd, uint64_t offset)
{
	return (void*)__syscall(SYS_mmap, addr, len, prot, flags, fd, offset);
}

void* munmap(void* addr, size_t len)
{
	return (void*)__syscall(SYS_munmap, addr, len);
}

