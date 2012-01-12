void exit(int result)
{
	__syscall(SYS_exit, result);
}

pid_t fork(void)
{
	return __syscall(SYS_fork);
}

int execve(const char* filename, const char** argv, const char** envp)
{
	__syscall(SYS_execve, filename, argv, envp);
}

pid_t wait(int* status)
{
	return __syscall(SYS_wait4, -1, status, 0, NULL);
}

pid_t waitpid(pid_t pid, int* status, int options)
{
	return __syscall(SYS_wait4, pid, status, options, NULL);
}

