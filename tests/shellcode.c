int main()
{
	char buffer[128];
	int fd = open("tests/shellcode_output", O_RDONLY, 0);
	int bytes = read(fd, buffer, 128);
	write(1, buffer, bytes);
	return 0;
}

