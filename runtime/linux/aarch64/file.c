int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds, struct timeval* timeout)
{
	struct timespec t;
	t.tv_sec = timeout->tv_sec;
	t.tv_nsec = timeout->tv_usec * 1000;
	return __syscall(SYS_pselect6, nfds, readfds, writefds, errorfds, &t, NULL);
}

int pipe(int* fds)
{
	return __syscall(SYS_pipe2, fds, 0);
}

