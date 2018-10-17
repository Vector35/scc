pid_t wait(int* status)
{
	return __syscall(SYS_wait4, -1, status, 0, NULL);
}

pid_t waitpid(pid_t pid, int* status, int options)
{
	return __syscall(SYS_wait4, pid, status, options, NULL);
}

sig_t signal(int sig, sig_t func)
{
	// FIXME: Handlers do not work because there is no way to force the correct calling convention.  Special
	// values such as SIG_IGN do work.
	struct sigaction act, old;
	memset(&act, 0, sizeof(act));
	act.sa_handler = func;

	int result = sigaction(sig, &act, &old);
	if (result < 0)
		return (sig_t)result;
	return old.sa_handler;
}

int sigaction(int sig, const struct sigaction* act, struct sigaction* old)
{
	return __syscall(SYS_rt_sigaction, sig, act, old, sizeof(sigset_t));
}

