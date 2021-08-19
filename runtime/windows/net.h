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

#ifndef __LIBC__WINDOWS_NET_H__
#define __LIBC__WINDOWS_NET_H__

#define AF_UNSPEC    0
#define AF_INET      2
#define AF_IPX       6
#define AF_APPLETALK 16
#define AF_NETBIOS   17
#define AF_INET6     23
#define AF_IRDA      26
#define AF_BTH       32

#define PF_UNSPEC    AF_UNSPEC
#define PF_INET      AF_INET
#define PF_IPX       AF_IPX
#define PF_APPLETALK AF_APPLETALK
#define PF_NETBIOS   AF_NETBIOS
#define PF_INET6     AF_INET6
#define PF_IRDA      AF_IRDA
#define PF_BTH       AF_BTH

#define SOCK_STREAM    1
#define SOCK_DGRAM     2
#define SOCK_RAW       3
#define SOCK_RDM       4
#define SOCK_SEQPACKET 5

#define IPPROTO_ICMP    1
#define IPPROTO_IGMP    2
#define BTHPROTO_RFCOMM 3
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_ICMPV6  58
#define IPPROTO_RM      113

#define SOL_SOCKET        0xffff
#define SO_DEBUG          1
#define SO_ACCEPTCONN     2
#define SO_REUSEADDR      4
#define SO_KEEPALIVE      8
#define SO_DONTROUTE      0x10
#define SO_BROADCAST      0x20
#define SO_USELOOPBACK    0x40
#define SO_LINGER         0x80
#define SO_OOBINLINE      0x100
#define SO_SNDBUF         0x1001
#define SO_RCVBUF         0x1002
#define SO_SNDLOWAT       0x1003
#define SO_RCVLOWAT       0x1004
#define SO_SNDTIMEO       0x1005
#define SO_RCVTIMEO       0x1006
#define SO_ERROR          0x1007
#define SO_TYPE           0x1008
#define SO_GROUP_ID       0x2001
#define SO_GROUP_PRIORITY 0x2002
#define SO_MAX_MSG_SIZE   0x2003
#define SO_PROTOCOL_INFOA 0x2004
#define SO_PROTOCOL_INFOW 0x2005
#define PVD_CONFIG        0x3001

#define TCP_NODELAY 1

#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN  128

typedef struct
{
	uint16_t wVersion;
	uint16_t wHighVersion;
	char szDescription[WSADESCRIPTION_LEN + 1];
	char szSystemStatus[WSASYS_STATUS_LEN + 1];
	uint16_t iMaxSockets;
	uint16_t iMaxUdpDg;
	char* lpVendorInfo;
} WSADATA;

struct sockaddr
{
	int16_t sa_family;
	char sa_data[14];
};

struct inaddr
{
	uint32_t s_addr;
};

struct in6_addr
{
	union
	{
		uint8_t s6_addr[16];
		uint8_t s6_addr8[16];
		uint16_t s6_addr16[8];
		uint32_t s6_addr32[4];
	};
};

struct sockaddr_in
{
	int16_t sin_family;
	uint16_t sin_port;
	struct inaddr sin_addr;
	char sin_zero[8];
};

struct sockaddr_in6
{
	int16_t sin6_family;
	uint16_t sin6_port;
	uint32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t sin6_scope_id;
};

typedef uint32_t socklen_t;

int __stdcall WSAStartup(uint16_t versionRequested, WSADATA* data) __import("ws2_32");

int __stdcall socket(int af, int type, int protocol) __import("ws2_32");
int __stdcall bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) __import("ws2_32");
int __stdcall accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) __import("ws2_32");
int __stdcall listen(int sockfd, int backlog) __import("ws2_32");
int __stdcall connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
    __import("ws2_32");
int __stdcall getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen) __import("ws2_32");
int __stdcall getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen) __import("ws2_32");
int __stdcall shutdown(int sockfd, int how) __import("ws2_32");
int __stdcall closesocket(int sockfd) __import("ws2_32");

int __stdcall send(int fd, const void* buf, int n, int flags) __import("ws2_32");
int __stdcall recv(int fd, void* buf, int n, int flags) __import("ws2_32");
int __stdcall sendto(int fd, const void* buf, int n, int flags, const struct sockaddr* addr,
    socklen_t addrlen) __import("ws2_32");
int __stdcall recvfrom(int fd, void* buf, int n, int flags, struct sockaddr* addr,
    socklen_t* addrlen) __import("ws2_32");

int __stdcall getsockopt(int fd, int level, int optname, void* optval, socklen_t* optlen)
    __import("ws2_32");
int __stdcall setsockopt(int fd, int level, int optname, const void* optval, socklen_t optlen)
    __import("ws2_32");

bool init_sockets();
int create_tcp4_connection(uint32_t ip, uint16_t port);
int create_udp4_connection(uint32_t ip, uint16_t port);
int create_tcp6_connection(const void* ip, uint16_t port);
int create_udp6_connection(const void* ip, uint16_t port);
ssize_t send_all(int fd, const void* buf, size_t n, int flags);
ssize_t recv_all(int fd, void* buf, size_t n, int flags);
ssize_t send_string(int fd, const char* str);

#endif
