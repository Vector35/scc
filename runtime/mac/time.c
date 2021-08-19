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

time_t time(time_t* t)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if (t)
		*t = tv.tv_sec;
	return tv.tv_sec;
}

int gettimeofday(struct timeval* t, struct timezone* tz)
{
	struct timezone tmp;
	if (!tz)
	{
		tmp.tz_minuteswest = 0;
		tmp.tz_dsttime = 0;
		tz = &tmp;
	}
	t->tv_sec = __syscall2(t->tv_usec, SYS_gettimeofday, tz);
	return 0;
}

int nanosleep(const struct timespec* amount, struct timespec* unslept)
{
	// Mac OS X does not have a nanosleep system, need to emulate it
	// TODO: The official runtime does this with an undocumented call, look into this

	// Currently emulating by creating a pipe (which is never accessed), then calling
	// select() with the desired timeout.
	int fds[2];
	fd_set set;
	pipe(fds);
	FD_ZERO(&set);
	FD_SET(fds[0], &set);

	struct timeval tv;
	tv.tv_sec = amount->tv_sec;
	tv.tv_usec = amount->tv_nsec / 1000;
	select(fds[0] + 1, &set, NULL, NULL, &tv);

	close(fds[0]);
	close(fds[1]);
	return 0;
}
