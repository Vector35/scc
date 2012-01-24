void main()
{
	void* buffer = mmap(NULL, 65536, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	read(0, buffer, 65536);
	goto *buffer;
}

