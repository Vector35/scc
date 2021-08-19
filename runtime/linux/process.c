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

unsigned int alarm(unsigned int seconds)
{
#ifdef SYS_alarm
	return __syscall(SYS_alarm, seconds);
#else
	struct itimerval it;
	it.it_value.tv_sec = seconds;
	it.it_value.tv_usec = 0;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	__syscall(SYS_setitimer, ITIMER_REAL, &it, &it);
	uint32_t result = it.it_value.tv_sec;
	if (it.it_value.tv_usec > 500000 || (!result && it.it_value.tv_usec))
		result++;
	return result;
#endif
}

int tgkill(int tgid, int tid, int sig)
{
	return __syscall(SYS_tgkill, tgid, tid, sig);
}

pid_t fork(void)
{
#ifdef SYS_fork
	return __syscall(SYS_fork);
#else
	return __syscall(SYS_clone, SIGCHLD, 0);
#endif
}
