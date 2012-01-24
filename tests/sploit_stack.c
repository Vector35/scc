void* sploit(void* tempBuffer)
{
	// Place shellcode buffer in this stack frame, which means the
	// stack pointer will be past the code.  This will test the
	// stack pivot logic to ensure code doesn't get corrupted.
	size_t bytes = read(0, tempBuffer, 65536);
	void* buffer = alloca(bytes);
	memcpy(buffer, tempBuffer, bytes);
	memset(tempBuffer, bytes, 0);
	return buffer;
}

void main()
{
	char data[65536];
	goto *sploit(data);
}

