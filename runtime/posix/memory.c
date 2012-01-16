void* malloc(size_t len)
{
	size_t finalLen = len + sizeof(size_t);
	void* result = mmap(NULL, (finalLen + 4095) & (~4095), PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	*(size_t*)result = len;
	return (void*)((size_t)result + sizeof(size_t));
}

void free(void* ptr)
{
	void* block = (void*)((size_t)ptr - sizeof(size_t));
	size_t len = *(size_t*)block;
	size_t fullLen = len + sizeof(size_t);
	munmap(block, (fullLen + 4095) & (~4095));
}

