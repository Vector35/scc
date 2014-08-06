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

#ifndef __LIBC__POSIX_NET_H__
#define __LIBC__POSIX_NET_H__

#define IPPROTO_IP        0
#define IPPROTO_HOPOPTS   0
#define IPPROTO_ICMP      1
#define IPPROTO_IGMP      2
#define IPPROTO_GGP       3
#define IPPROTO_IPV4      4
#define IPPROTO_IPIP      IPPROTO_IPV4
#define IPPROTO_TCP       6
#define IPPROTO_ST        7
#define IPPROTO_EGP       8
#define IPPROTO_PUP       12
#define IPPROTO_ARGUS     13
#define IPPROTO_EMCON     14
#define IPPROTO_XNET      15
#define IPPROTO_CHAOS     16
#define IPPROTO_UDP       17
#define IPPROTO_MUX       18
#define IPPROTO_MEAS      19
#define IPPROTO_HMP       20
#define IPPROTO_PRM       21
#define IPPROTO_IDP       22
#define IPPROTO_TRUNK1    23
#define IPPROTO_TRUNK2    24
#define IPPROTO_LEAF1     25
#define IPPROTO_LEAF2     26
#define IPPROTO_RDP       27
#define IPPROTO_IRTP      28
#define IPPROTO_TP        29
#define IPPROTO_BLT       30
#define IPPROTO_NSP       31
#define IPPROTO_INP       32
#define IPPROTO_SEP       33
#define IPPROTO_DCCP      33
#define IPPROTO_3PC       34
#define IPPROTO_IDPR      35
#define IPPROTO_XTP       36
#define IPPROTO_DDP       37
#define IPPROTO_CMTP      38
#define IPPROTO_TPXX      39
#define IPPROTO_IL        40
#define IPPROTO_IPV6      41
#define IPPROTO_SDRP      42
#define IPPROTO_ROUTING   43
#define IPPROTO_FRAGMENT  44
#define IPPROTO_IDRP      45
#define IPPROTO_RSVP      46
#define IPPROTO_GRE       47
#define IPPROTO_MHRP      48
#define IPPROTO_BHA       49
#define IPPROTO_ESP       50
#define IPPROTO_AH        51
#define IPPROTO_INLSP     52
#define IPPROTO_SWIPE     53
#define IPPROTO_NHRP      54
#define IPPROTO_MOBILE    55
#define IPPROTO_TLSP      56
#define IPPROTO_SKIP      57
#define IPPROTO_ICMPV6    58
#define IPPROTO_NONE      59
#define IPPROTO_DSTOPTS   60
#define IPPROTO_AHIP      61
#define IPPROTO_CFTP      62
#define IPPROTO_HELLO     63
#define IPPROTO_SATEXPAK  64
#define IPPROTO_KRYPTOLAN 65
#define IPPROTO_RVD       66
#define IPPROTO_IPPC      67
#define IPPROTO_ADFS      68
#define IPPROTO_SATMON    69
#define IPPROTO_VISA      70
#define IPPROTO_IPCV      71
#define IPPROTO_CPNX      72
#define IPPROTO_CPHB      73
#define IPPROTO_WSN       74
#define IPPROTO_PVP       75
#define IPPROTO_BRSATMON  76
#define IPPROTO_ND        77
#define IPPROTO_WBMON     78
#define IPPROTO_WBEXPAK   79
#define IPPROTO_EON       80
#define IPPROTO_VMTP      81
#define IPPROTO_SVMTP     82
#define IPPROTO_VINES     83
#define IPPROTO_TTP       84
#define IPPROTO_IGP       85
#define IPPROTO_DGP       86
#define IPPROTO_TCF       87
#define IPPROTO_IGRP      88
#define IPPROTO_OSPFIGP   89
#define IPPROTO_SRPC      90
#define IPPROTO_LARP      91
#define IPPROTO_MTP       92
#define IPPROTO_AX25      93
#define IPPROTO_IPEIP     94
#define IPPROTO_BEETPH    94
#define IPPROTO_MICP      95
#define IPPROTO_SCCSP     96
#define IPPROTO_ETHERIP   97
#define IPPROTO_ENCAP     98
#define IPPROTO_APES      99
#define IPPROTO_GMTP      100
#define IPPROTO_PIM       103
#define IPPROTO_COMP      108
#define IPPROTO_IPCOMP    IPPROTO_COMP
#define IPPROTO_CARP      112
#define IPPROTO_PGM       113
#define IPPROTO_SCTP      132
#define IPPROTO_MH        135
#define IPPROTO_UDPLITE   136
#define IPPROTO_PFSYNC    240
#define IPPROTO_RAW       255

struct sockaddr;
typedef uint32_t socklen_t;

int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int* fds);
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
int listen(int sockfd, int backlog);
int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
int shutdown(int sockfd, int how);
int closesocket(int sockfd);

ssize_t send(int fd, const void* buf, size_t n, int flags);
ssize_t recv(int fd, void* buf, size_t n, int flags);
ssize_t sendto(int fd, const void* buf, size_t n, int flags, const struct sockaddr* addr, socklen_t addrlen);
ssize_t recvfrom(int fd, void* buf, size_t n, int flags, struct sockaddr* addr, socklen_t* addrlen);

int getsockopt(int fd, int level, int optname, void* optval, socklen_t* optlen);
int setsockopt(int fd, int level, int optname, const void* optval, socklen_t optlen);

int create_tcp4_connection(uint32_t ip, uint16_t port);
int create_udp4_connection(uint32_t ip, uint16_t port);
int create_tcp6_connection(const void* ip, uint16_t port);
int create_udp6_connection(const void* ip, uint16_t port);
ssize_t send_all(int fd, const void* buf, size_t n, int flags);
ssize_t recv_all(int fd, void* buf, size_t n, int flags);
ssize_t send_string(int fd, const char* str);

#endif

