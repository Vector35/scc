pid_t wait(int* status)
{
	return __syscall(SYS_waitpid, -1, status, 0);
}

pid_t waitpid(pid_t pid, int* status, int options)
{
	return __syscall(SYS_waitpid, pid, status, options);
}

