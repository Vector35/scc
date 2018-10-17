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

void exit(int result) __noreturn
{
	__syscall(SYS_exit, result);
}

int execve(const char* filename, const char** argv, const char** envp)
{
	return __syscall(SYS_execve, filename, argv, envp);
}

int execl(const char* filename, const char* arg0, ...)
{
	return execve(filename, &arg0, NULL);
}

int getitimer(int which, struct itimerval* value)
{
	return __syscall(SYS_getitimer, which, value);
}

int setitimer(int which, const struct itimerval* value, struct itimerval* old)
{
	return __syscall(SYS_setitimer, which, value, old);
}

uid_t getuid(void)
{
	return __syscall(SYS_getuid);
}

gid_t getgid(void)
{
	return __syscall(SYS_getgid);
}

uid_t geteuid(void)
{
	return __syscall(SYS_geteuid);
}

gid_t getegid(void)
{
	return __syscall(SYS_getegid);
}

int getgroups(int size, gid_t* groups)
{
	return __syscall(SYS_getgroups, size, groups);
}

int setuid(uid_t id)
{
	return __syscall(SYS_setuid, id);
}

int seteuid(uid_t id)
{
	return __syscall(SYS_setreuid, -1, id);
}

int setreuid(uid_t rid, uid_t eid)
{
	return __syscall(SYS_setreuid, rid, eid);
}

int setgid(gid_t id)
{
	return __syscall(SYS_setgid, id);
}

int setegid(gid_t id)
{
	return __syscall(SYS_setregid, -1, id);
}

int setregid(uid_t rid, uid_t eid)
{
	return __syscall(SYS_setregid, rid, eid);
}

pid_t getpid(void)
{
	return __syscall(SYS_getpid);
}

pid_t getppid(void)
{
	return __syscall(SYS_getppid);
}

pid_t setsid(void)
{
	return __syscall(SYS_setsid);
}

int setpgid(pid_t pid, pid_t pgid)
{
	return __syscall(SYS_setpgid, pid, pgid);
}

pid_t getpgid(pid_t pid)
{
	return __syscall(SYS_getpgid, pid);
}

pid_t getpgrp(void)
{
#ifdef SYS_getpgrp
	return __syscall(SYS_getpgrp);
#else
	return getpgid(0);
#endif
}

int kill(pid_t pid, int sig)
{
	return __syscall(SYS_kill, pid, sig);
}

