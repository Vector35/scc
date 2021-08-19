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

#ifndef __LIBC__LINUX_NET_H__
#define __LIBC__LINUX_NET_H__

#define PF_UNSPEC     0
#define PF_LOCAL      1
#define PF_UNIX       PF_LOCAL
#define PF_FILE       PF_LOCAL
#define PF_INET       2
#define PF_AX25       3
#define PF_IPX        4
#define PF_APPLETALK  5
#define PF_NETROM     6
#define PF_BRIDGE     7
#define PF_ATMPVC     8
#define PF_X25        9
#define PF_INET6      10
#define PF_ROSE       11
#define PF_DECnet     12
#define PF_NETBEUI    13
#define PF_SECURITY   14
#define PF_KEY        15
#define PF_NETLINK    16
#define PF_ROUTE      PF_NETLINK
#define PF_PACKET     17
#define PF_ASH        18
#define PF_ECONET     19
#define PF_ATMSVC     20
#define PF_RDS        21
#define PF_SNA        22
#define PF_IRDA       23
#define PF_PPPOX      24
#define PF_WANPIPE    25
#define PF_LLC        26
#define PF_CAN        29
#define PF_TIPC       30
#define PF_BLUETOOTH  31
#define PF_IUCV       32
#define PF_RXRPC      33
#define PF_ISDN       34
#define PF_PHONET     35
#define PF_IEEE802154 36
#define PF_CAIF       37
#define PF_ALG        38
#define PF_NFC        39

#define AF_UNSPEC     PF_UNSPEC
#define AF_LOCAL      PF_LOCAL
#define AF_UNIX       PF_UNIX
#define AF_FILE       PF_FILE
#define AF_INET       PF_INET
#define AF_AX25       PF_AX25
#define AF_IPX        PF_IPX
#define AF_APPLETALK  PF_APPLETALK
#define AF_NETROM     PF_NETROM
#define AF_BRIDGE     PF_BRIDGE
#define AF_ATMPVC     PF_ATMPVC
#define AF_X25        PF_X25
#define AF_INET6      PF_INET6
#define AF_ROSE       PF_ROSE
#define AF_DECnet     PF_DECnet
#define AF_NETBEUI    PF_NETBEUI
#define AF_SECURITY   PF_SECURITY
#define AF_KEY        PF_KEY
#define AF_NETLINK    PF_NETLINK
#define AF_ROUTE      PF_ROUTE
#define AF_PACKET     PF_PACKET
#define AF_ASH        PF_ASH
#define AF_ECONET     PF_ECONET
#define AF_ATMSVC     PF_ATMSVC
#define AF_RDS        PF_RDS
#define AF_SNA        PF_SNA
#define AF_IRDA       PF_IRDA
#define AF_PPPOX      PF_PPPOX
#define AF_WANPIPE    PF_WANPIPE
#define AF_LLC        PF_LLC
#define AF_CAN        PF_CAN
#define AF_TIPC       PF_TIPC
#define AF_BLUETOOTH  PF_BLUETOOTH
#define AF_IUCV       PF_IUCV
#define AF_RXRPC      PF_RXRPC
#define AF_ISDN       PF_ISDN
#define AF_PHONET     PF_PHONET
#define AF_IEEE802154 PF_IEEE802154
#define AF_CAIF       PF_CAIF
#define AF_ALG        PF_ALG
#define AF_NFC        PF_NFC

#define SOMAXCONN 128

#define MSG_OOB          1
#define MSG_PEEK         2
#define MSG_DONTROUTE    4
#define MSG_TRYHARD      MSG_DONTROUTE
#define MSG_CTRUNC       8
#define MSG_PROXY        0x10
#define MSG_TRUNC        0x20
#define MSG_DONTWAIT     0x40
#define MSG_EOR          0x80
#define MSG_WAITALL      0x100
#define MSG_FIN          0x200
#define MSG_SYN          0x400
#define MSG_CONFIRM      0x800
#define MSG_RST          0x1000
#define MSG_ERRQUEUE     0x2000
#define MSG_NOSIGNAL     0x4000
#define MSG_MORE         0x8000
#define MSG_WAITFORONE   0x10000
#define MSG_CMSG_CLOEXEC 0x40000000

#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2

struct sockaddr
{
	uint16_t sa_family;
	char sa_data[14];
};

struct in_addr
{
	uint32_t s_addr;
};

struct sockaddr_in
{
	uint16_t sin_family;
	uint16_t sin_port;
	struct in_addr sin_addr;
	char __pad[8];
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

struct sockaddr_in6
{
	uint16_t sin6_family;
	uint16_t sin6_port;
	uint32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t sin6_scope_id;
};

struct sockaddr_un
{
	uint16_t sun_family;
	char sun_path[108];
};

int accept4(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags);

#endif
