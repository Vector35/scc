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

#ifndef __LIBC__MAC_NET_H__
#define __LIBC__MAC_NET_H__

#define SOCK_STREAM       1
#define SOCK_DGRAM        2
#define SOCK_RAW          3
#define SOCK_RDM          4
#define SOCK_SEQPACKET    5

#define PF_UNSPEC         0
#define PF_LOCAL          AF_UNIX
#define PF_UNIX           1
#define PF_INET           2
#define PF_IMPLINK        3
#define PF_PUP            4
#define PF_CHAOS          5
#define PF_NS             6
#define PF_ISO            7
#define PF_OSI            AF_ISO
#define PF_ECMA           8
#define PF_DATAKIT        9
#define PF_CCITT          10
#define PF_SNA            11
#define PF_DECnet         12
#define PF_DLI            13
#define PF_LAT            14
#define PF_HYLINK         15
#define PF_APPLETALK      16
#define PF_ROUTE          17
#define PF_LINK           18
#define PF_XTP            19
#define PF_COIP           20
#define PF_CNT            21
#define PF_RTIP           22
#define PF_IPX            23
#define PF_SIP            24
#define PF_PIP            25
#define PF_NDRV           27
#define PF_ISDN           28
#define PF_E164           AF_ISDN
#define PF_KEY            29
#define PF_INET6          30
#define PF_NATM           31
#define PF_SYSTEM         32
#define PF_NETBIOS        33
#define PF_PPP            34
#define PF_ATM            30
#define PF_HDRCMPLT       35
#define PF_NETGRAPH       32
#define PF_IEEE80211      37

#define AF_INET           ((PF_INET << 8) | sizeof(struct sockaddr_in))
#define AF_INET6          ((PF_INET6 << 8) | sizeof(struct sockaddr_in6))
#define AF_UNIX           ((PF_UNIX << 8) | sizeof(struct sockaddr_un))
#define AF_LOCAL          AF_UNIX

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
#define SO_REUSEPORT      0x200
#define SO_TIMESTAMP      0x400
#define SO_TIMESTAMP_MONOTONIC 0x800
#define SO_ACCEPTFILTER   0x1000
#define SO_DONTTRUNC      0x2000
#define SO_WANTMORE       0x4000
#define SO_WANTOOBFLAG    0x8000
#define SO_SNDBUF         0x1001
#define SO_RCVBUF         0x1002
#define SO_SNDLOWAT       0x1003
#define SO_RCVLOWAT       0x1004
#define SO_SNDTIMEO       0x1005
#define SO_RCVTIMEO       0x1006
#define SO_ERROR          0x1007
#define SO_TYPE           0x1008
#define SO_LABEL          0x1010
#define SO_PEERLABEL      0x1011
#define SO_NREAD          0x1020
#define SO_NKE            0x1021
#define SO_NOSIGPIPE      0x1022
#define SO_NOADDRERR      0x1023
#define SO_NWRITE         0x1024
#define SO_REUSESHAREUID  0x1025
#define SO_NOTIFYCONFLICT 0x1026
#define SO_UPCALLCLOSEWAIT 0x1027
#define SO_LINGER_SEC     0x1080
#define SO_RESTRICTIONS   0x1081
#define SO_RESTRICT_DENYIN 1
#define SO_RESTRICT_DENYOUT 2
#define SO_RESTRICT_DENYSET 0x80000000
#define SO_RANDOMPORT     0x1082
#define SO_NP_EXTENSIONS  0x1083

#define SOMAXCONN         128

#define MSG_OOB           1
#define MSG_PEEK          2
#define MSG_DONTROUTE     4
#define MSG_EOR           8
#define MSG_TRUNC         0x10
#define MSG_CTRUNC        0x20
#define MSG_WAITALL       0x40
#define MSG_DONTWAIT      0x80
#define MSG_EOF           0x100
#define MSG_WAITSTREAM    0x200
#define MSG_FLUSH         0x400
#define MSG_HOLD          0x800
#define MSG_SEND          0x1000
#define MSG_HAVEMORE      0x2000
#define MSG_RCVMORE       0x4000
#define MSG_NEEDSA        0x10000

#define SHUT_RD           0
#define SHUT_WR           1
#define SHUT_RDWR         2

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
	char sin_zero[8];
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
	char sun_path[104];
};

#endif

