int socket(int domain, int type, int protocol)
{
	return __syscall(SYS_socket, domain, type, protocol);
}

int socketpair(int domain, int type, int protocol, int* fds)
{
	return __syscall(SYS_socketpair, domain, type, protocol, fds);
}

int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	return __syscall(SYS_bind, sockfd, addr, addrlen);
}

int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	return __syscall(SYS_accept, sockfd, addr, addrlen);
}

int accept4(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags)
{
	return __syscall(SYS_accept4, sockfd, addr, addrlen, flags);
}

int listen(int sockfd, int backlog)
{
	return __syscall(SYS_listen, sockfd, backlog);
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	return __syscall(SYS_connect, sockfd, addr, addrlen);
}

int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	return __syscall(SYS_getsockname, sockfd, addr, addrlen);
}

int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	return __syscall(SYS_getpeername, sockfd, addr, addrlen);
}

int shutdown(int sockfd, int how)
{
	return __syscall(SYS_shutdown, sockfd, how);
}

ssize_t send(int fd, const void* buf, size_t n, int flags)
{
	return __syscall(SYS_sendto, fd, buf, n, flags, NULL, 0);
}

ssize_t recv(int fd, void* buf, size_t n, int flags)
{
	return __syscall(SYS_recvfrom, fd, buf, n, flags, NULL, NULL);
}

ssize_t sendto(int fd, const void* buf, size_t n, int flags, const struct sockaddr* addr, socklen_t addrlen)
{
	return __syscall(SYS_sendto, fd, buf, n, flags, addr, addrlen);
}

ssize_t recvfrom(int fd, void* buf, size_t n, int flags, struct sockaddr* addr, socklen_t* addrlen)
{
	return __syscall(SYS_recvfrom, fd, buf, n, flags, addr, addrlen);
}

int getsockopt(int fd, int level, int optname, void* optval, socklen_t* optlen)
{
	return __syscall(SYS_getsockopt, fd, level, optname, optval, optlen);
}

int setsockopt(int fd, int level, int optname, const void* optval, socklen_t optlen)
{
	return __syscall(SYS_setsockopt, fd, level, optname, optval, optlen);
}

