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

size_t lseek(int fd, ssize_t offset, int whence)
{
	return __syscall(SYS_lseek, fd, offset, whence);
}

int truncate(const char* path, ssize_t length)
{
	return __syscall(SYS_truncate, path, length);
}

int ftruncate(int fd, ssize_t length)
{
	return __syscall(SYS_ftruncate, fd, length);
}

ssize_t sendfile(int outFd, int inFd, size_t* offset, size_t count)
{
	return __syscall(SYS_sendfile, outFd, inFd, offset, count);
}

char* getcwd(char* buf, size_t size)
{
	if (__syscall(SYS_getcwd, buf, size) < 0)
		return NULL;
	return buf;
}

int getdents(int fd, struct dirent* dirp, size_t count)
{
	return __syscall(SYS_getdents, fd, dirp, count);
}

int pipe(int* fds)
{
	return __syscall(SYS_pipe, fds);
}

