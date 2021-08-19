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

#ifndef __LIBC__LINUX_X64_NET_H__
#define __LIBC__LINUX_X64_NET_H__

#define SOCK_STREAM    1
#define SOCK_DGRAM     2
#define SOCK_RAW       3
#define SOCK_RDM       4
#define SOCK_SEQPACKET 5
#define SOCK_DCCP      6
#define SOCK_PACKET    10
#define SOCK_NONBLOCK  0x800
#define SOCK_CLOEXEC   0x80000

#define SO_DEBUG         1
#define SO_REUSEADDR     2
#define SO_TYPE          3
#define SO_ERROR         4
#define SO_DONTROUTE     5
#define SO_BROADCAST     6
#define SO_SNDBUF        7
#define SO_RCVBUF        8
#define SO_SNDBUFFORCE   32
#define SO_RCVBUFFORCE   33
#define SO_KEEPALIVE     9
#define SO_OOBINLINE     10
#define SO_NO_CHECK      11
#define SO_PRIORITY      12
#define SO_LINGER        13
#define SO_BSDCOMPAT     14
#define SO_REUSEPORT     15
#define SO_PASSCRED      16
#define SO_PEERCRED      17
#define SO_RCVLOWAT      18
#define SO_SNDLOWAT      19
#define SO_RCVTIMEO      20
#define SO_SNDTIMEO      21
#define SO_BINDTODEVICE  25
#define SO_ATTACH_FILTER 26
#define SO_DETACH_FILTER 27
#define SO_PEERNAME      28
#define SO_TIMESTAMP     29
#define SCM_TIMESTAMP    SO_TIMESTAMP
#define SO_ACCEPTCONN    30
#define SO_PEERSEC       31
#define SO_PASSSEC       34
#define SO_TIMESTAMPNS   35
#define SCM_TIMESTAMPNS  SO_TIMESTAMPNS
#define SO_MARK          36
#define SO_TIMESTAMPING  37
#define SCM_TIMESTAMPING SO_TIMESTAMPING
#define SO_PROTOCOL      38
#define SO_DOMAIN        39
#define SO_RXQ_OVFL      40

#define SOL_SOCKET 1
#define SOL_RAW    255
#define SOL_DECNET 261
#define SOL_X25    262
#define SOL_PACKET 263
#define SOL_ATM    264
#define SOL_AAL    265
#define SOL_IRDA   266

#endif
