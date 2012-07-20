// Copyright (c) 2011-2012 Rusty Wagner
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

#ifndef __LIBC__PROCESS_H__
#define __LIBC__PROCESS_H__

#include "runtime/posix/time.h"

#define ITIMER_REAL     0
#define ITIMER_VIRTUAL  1
#define ITIMER_PROF     2

typedef int pid_t;
typedef int uid_t;
typedef int gid_t;

struct itimerval
{
	struct timeval it_interval;
	struct timeval it_value;
};

void exit(int result) __noreturn;
#define _exit(result) exit(result)

pid_t fork(void);
int execve(const char* filename, const char** argv, const char** envp);
pid_t wait(int* status);
pid_t waitpid(pid_t pid, int* status, int options);

unsigned int alarm(unsigned int seconds);
int getitimer(int which, struct itimerval* value);
int setitimer(int which, const struct itimerval* value, struct itimerval* old);

uid_t getuid(void);
gid_t getgid(void);
uid_t geteuid(void);
gid_t getegid(void);
int getgroups(int size, gid_t* groups);
int setuid(uid_t id);
int seteuid(uid_t id);
int setreuid(uid_t rid, uid_t eid);
int setgid(gid_t id);
int setegid(gid_t id);
int setregid(uid_t rid, uid_t eid);

pid_t getpid(void);
pid_t getppid(void);

pid_t setsid(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);

int kill(pid_t pid, int sig);

#endif

