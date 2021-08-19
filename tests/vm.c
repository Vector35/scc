void main()
{
	int fd = open("Obj/test", O_RDONLY, 0);
	void* buf = malloc(1048576);
	read(fd, buf, 1048576);
	close(fd);

	quark_exec(buf);
}
