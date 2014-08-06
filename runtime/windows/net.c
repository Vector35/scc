// Copyright (c) 2014 Rusty Wagner
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

bool init_sockets()
{
	WSADATA data;
	return WSAStartup(0x202, &data) == 0;
}

int create_tcp4_connection(uint32_t ip, uint16_t port)
{
	int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s < 0)
		return s;

	union
	{
		struct sockaddr addr;
		struct sockaddr_in addrIn;
	} addr;
	memset(&addr, 0, sizeof(addr));
	addr.addrIn.sin_family = AF_INET;
	addr.addrIn.sin_port = htons(port);
	addr.addrIn.sin_addr.s_addr = htonl(ip);

	int result = connect(s, (struct sockaddr*)&addr.addr, sizeof(addr));
	if (result < 0)
		return result;

	return s;
}

int create_udp4_connection(uint32_t ip, uint16_t port)
{
	int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0)
		return s;

	union
	{
		struct sockaddr addr;
		struct sockaddr_in addrIn;
	} addr;
	memset(&addr, 0, sizeof(addr));
	addr.addrIn.sin_family = AF_INET;
	addr.addrIn.sin_port = htons(port);
	addr.addrIn.sin_addr.s_addr = htonl(ip);

	int result = connect(s, (struct sockaddr*)&addr.addr, sizeof(addr));
	if (result < 0)
		return result;

	return s;
}

int create_tcp6_connection(const void* ip, uint16_t port)
{
	int s = socket(PF_INET6, SOCK_STREAM, 0);
	if (s < 0)
		return s;

	union
	{
		struct sockaddr addr;
		struct sockaddr_in6 addrIn;
	} addr;
	memset(&addr, 0, sizeof(addr));
	addr.addrIn.sin6_family = AF_INET6;
	addr.addrIn.sin6_port = htons(port);
	memcpy(addr.addrIn.sin6_addr.s6_addr, ip, sizeof(struct in6_addr));

	int result = connect(s, (struct sockaddr*)&addr.addr, sizeof(addr));
	if (result < 0)
		return result;

	return s;
}

int create_udp6_connection(const void* ip, uint16_t port)
{
	int s = socket(PF_INET6, SOCK_DGRAM, 0);
	if (s < 0)
		return s;

	union
	{
		struct sockaddr addr;
		struct sockaddr_in6 addrIn;
	} addr;
	memset(&addr, 0, sizeof(addr));
	addr.addrIn.sin6_family = AF_INET6;
	addr.addrIn.sin6_port = htons(port);
	memcpy(addr.addrIn.sin6_addr.s6_addr, ip, sizeof(struct in6_addr));

	int result = connect(s, (struct sockaddr*)&addr.addr, sizeof(addr));
	if (result < 0)
		return result;

	return s;
}

ssize_t send_all(int fd, const void* buf, size_t n, int flags)
{
	size_t offset = 0;
	while (offset < n)
	{
		ssize_t result = send(fd, (const void*)((size_t)buf + offset), n - offset, flags);
		if (result < 0)
			return result;
		offset += result;
	}
	return offset;
}

ssize_t recv_all(int fd, void* buf, size_t n, int flags)
{
	size_t offset = 0;
	while (offset < n)
	{
		ssize_t result = recv(fd, (void*)((size_t)buf + offset), n - offset, flags);
		if (result < 0)
			return result;
		offset += result;
	}
	return offset;
}

ssize_t send_string(int fd, const char* str)
{
	return send_all(fd, str, strlen(str), 0);
}

