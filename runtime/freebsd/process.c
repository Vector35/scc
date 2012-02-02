void exit(int result) __noreturn
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

