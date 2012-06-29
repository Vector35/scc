// Copyright (c) 2012 Rusty Wagner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

int socket(int domain, int type, int protocol)
{
	size_t args[3];
	args[0] = domain;
	args[1] = type;
	args[2] = protocol;
	return __syscall(SYS_socketcall, SYS_socket, args);
}

int socketpair(int domain, int type, int protocol, int* fds)
{
	size_t args[4];
	args[0] = domain;
	args[1] = type;
	args[2] = protocol;
	args[3] = (size_t)fds;
	return __syscall(SYS_socketcall, SYS_socketpair, args);
}

int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	size_t args[3];
	args[0] = sockfd;
	args[1] = (size_t)addr;
	args[2] = addrlen;
	return __syscall(SYS_socketcall, SYS_bind, args);
}

int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	size_t args[3];
	args[0] = sockfd;
	args[1] = (size_t)addr;
	args[2] = (size_t)addrlen;
	return __syscall(SYS_socketcall, SYS_accept, args);
}

int accept4(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags)
{
	size_t args[4];
	args[0] = sockfd;
	args[1] = (size_t)addr;
	args[2] = (size_t)addrlen;
	args[3] = flags;
	return __syscall(SYS_socketcall, SYS_accept4, args);
}

int listen(int sockfd, int backlog)
{
	size_t args[2];
	args[0] = sockfd;
	args[1] = backlog;
	return __syscall(SYS_socketcall, SYS_listen, args);
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
	size_t args[3];
	args[0] = sockfd;
	args[1] = (size_t)addr;
	args[2] = addrlen;
	return __syscall(SYS_socketcall, SYS_connect, args);
}

int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	size_t args[3];
	args[0] = sockfd;
	args[1] = (size_t)addr;
	args[2] = (size_t)addrlen;
	return __syscall(SYS_socketcall, SYS_getsockname, args);
}

int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
	size_t args[3];
	args[0] = sockfd;
	args[1] = (size_t)addr;
	args[2] = (size_t)addrlen;
	return __syscall(SYS_socketcall, SYS_getpeername, args);
}

int shutdown(int sockfd, int how)
{
	size_t args[2];
	args[0] = sockfd;
	args[1] = how;
	return __syscall(SYS_socketcall, SYS_shutdown, args);
}

ssize_t send(int fd, const void* buf, size_t n, int flags)
{
	size_t args[4];
	args[0] = fd;
	args[1] = (size_t)buf;
	args[2] = n;
	args[3] = flags;
	return __syscall(SYS_socketcall, SYS_send, args);
}

ssize_t recv(int fd, void* buf, size_t n, int flags)
{
	size_t args[4];
	args[0] = fd;
	args[1] = (size_t)buf;
	args[2] = n;
	args[3] = flags;
	return __syscall(SYS_socketcall, SYS_recv, args);
}

ssize_t sendto(int fd, const void* buf, size_t n, int flags, const struct sockaddr* addr, socklen_t addrlen)
{
	size_t args[6];
	args[0] = fd;
	args[1] = (size_t)buf;
	args[2] = n;
	args[3] = flags;
	args[4] = (size_t)addr;
	args[5] = addrlen;
	return __syscall(SYS_socketcall, SYS_sendto, args);
}

ssize_t recvfrom(int fd, void* buf, size_t n, int flags, struct sockaddr* addr, socklen_t* addrlen)
{
	size_t args[6];
	args[0] = fd;
	args[1] = (size_t)buf;
	args[2] = n;
	args[3] = flags;
	args[4] = (size_t)addr;
	args[5] = (size_t)addrlen;
	return __syscall(SYS_socketcall, SYS_recvfrom, args);
}

int getsockopt(int fd, int level, int optname, void* optval, socklen_t* optlen)
{
	size_t args[5];
	args[0] = fd;
	args[1] = level;
	args[2] = optname;
	args[3] = (size_t)optval;
	args[4] = (size_t)optlen;
	return __syscall(SYS_socketcall, SYS_getsockopt, args);
}

int setsockopt(int fd, int level, int optname, const void* optval, socklen_t optlen)
{
	size_t args[5];
	args[0] = fd;
	args[1] = level;
	args[2] = optname;
	args[3] = (size_t)optval;
	args[4] = optlen;
	return __syscall(SYS_socketcall, SYS_setsockopt, args);
}

