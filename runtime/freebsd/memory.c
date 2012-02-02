void* mmap(void* addr, size_t len, int prot, int flags, int fd, uint64_t offset)
{
	size_t shiftedOffset = (size_t)(offset >> 12);
	return (void*)__syscall(SYS_mmap, addr, len, prot, flags, fd, shiftedOffset);
}

void* munmap(void* addr, size_t len)
{
	return (void*)__syscall(SYS_munmap, addr, len);
}

