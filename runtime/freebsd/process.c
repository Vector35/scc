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

int sysctl(const int* name, size_t namelen, void* oldp, size_t* oldlenp, const void* newp, size_t newlen)
{
	return __syscall(SYS___sysctl, name, namelen, oldp, oldlenp, newp, newlen);
}

pid_t waitpid(pid_t pid, int* status, int options)
{
	return __syscall(SYS_wait4, pid, status, options, NULL);
}

pid_t fork(void)
{
	return __syscall(SYS_fork);
}

sig_t signal(int sig, sig_t func)
{
	// FIXME: Handlers do not work on 64-bit because there is no way to force the correct calling convention.  Special
	// values such as SIG_IGN do work.
	struct sigaction act, old;
	memset(&act, 0, sizeof(act));
	act.sa_handler = func;

	int result = sigaction(sig, &act, &old);
	if (result < 0)
		return (sig_t)result;
	return old.sa_handler;
}

int sigaction(int sig, const struct sigaction* act, struct sigaction* old)
{
	return __syscall(SYS_sigaction, sig, act, old);
}

