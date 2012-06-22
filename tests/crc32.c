int main()
{
	char* msg = "OMGitsfullofAAAAAAAAs";
	printf("%x\n", crc32(msg, strlen(msg)));
	return 0;
}

