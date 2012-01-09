void exit(int result)
{
	__syscall(SYS_exit, result);
}

