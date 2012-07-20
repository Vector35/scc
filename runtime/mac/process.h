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

#ifndef __LIBC__MAC_PROCESS_H__
#define __LIBC__MAC_PROCESS_H__

#define SIGHUP     1
#define SIGINT     2
#define SIGQUIT    3
#define SIGILL     4
#define SIGTRAP    5
#define SIGABRT    6
#define SIGIOT     7
#define SIGPOLL    7
#define SIGEMT     7
#define SIGFPE     8
#define SIGKILL    9
#define SIGBUS     10
#define SIGSEGV    11
#define SIGSYS     12
#define SIGPIPE    13
#define SIGALRM    14
#define SIGTERM    15
#define SIGURG     16
#define SIGSTOP    17
#define SIGTSTP    18
#define SIGCONT    19
#define SIGCHLD    20
#define SIGTTIN    21
#define SIGTTOU    22
#define SIGIO      23
#define SIGXCPU    24
#define SIGXFSZ    25
#define SIGVTALRM  26
#define SIGPROF    27
#define SIGWINCH   28
#define SIGINFO    29
#define SIGUSR1    30
#define SIGUSR2    31

#define SIG_DFL     ((sig_t)0)
#define SIG_IGN     ((sig_t)1)
#define SIG_HOLD    ((sig_t)5)

#define SA_ONSTACK    1
#define SA_RESTART    2
#define SA_RESETHAND  4
#define SA_NOCLDSTOP  8
#define SA_NODEFER    0x10
#define SA_NOCLDWAIT  0x20
#define SA_SIGINFO    0x40
#define SA_USERTRAMP  0x100
#define SA_64REGSET   0x200

typedef void (*sig_t)(int);
typedef uint32_t sigset_t;

typedef struct
{
	int si_signo;
	int si_errno;
	int si_code;
	pid_t si_pid;
	uid_t si_uid;
	int si_status;
	void* si_addr;
} siginfo_t;

struct sigaction
{
	union
	{
		sig_t sa_handler;
		void (*sa_sigaction)(int, siginfo_t*, void*);
	};
	void (*sa_tramp)(void*, int, int, siginfo_t*, void*);
	sigset_t sa_mask;
	int sa_flags;
};

int sysctl(const int* name, size_t namelen, void* oldp, size_t* oldlenp, const void* newp, size_t newlen);

sig_t signal(int sig, sig_t func);
int sigaction(int sig, const struct sigaction* act, struct sigaction* old);

#endif

