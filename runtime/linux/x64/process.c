pid_t wait(int* status)
{
	return __syscall(SYS_wait4, -1, status, 0, NULL);
}

pid_t waitpid(pid_t pid, int* status, int options)
{
	return __syscall(SYS_wait4, pid, status, options, NULL);
}

