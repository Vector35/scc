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

size_t lseek(int fd, int64_t offset, int whence)
{
	return __syscall(SYS_lseek, fd, offset, whence);
}

int truncate(const char* path, int64_t length)
{
	return __syscall(SYS_truncate, path, length);
}

int ftruncate(int fd, int64_t length)
{
	return __syscall(SYS_ftruncate, fd, length);
}

char* getcwd(char* buf, size_t size)
{
	if (__syscall(SYS___getcwd, buf, size) < 0)
		return NULL;
	return buf;
}

int getdents(int fd, struct dirent* dirp, size_t count)
{
	return __syscall(SYS_getdents, fd, dirp, count);
}

int getdirentries(int fd, struct dirent* dirp, size_t count, ssize_t* basep)
{
	return __syscall(SYS_getdirentries, fd, dirp, count, basep);
}

int fstat(int fd, struct stat* buf)
{
	return __syscall(SYS_fstat, fd, buf);
}

int stat(const char* path, struct stat* buf)
{
	return __syscall(SYS_stat, path, buf);
}

int lstat(const char* path, struct stat* buf)
{
	return __syscall(SYS_lstat, path, buf);
}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds, struct timeval* timeout)
{
	return __syscall(SYS_select, nfds, readfds, writefds, errorfds, timeout);
}

int pipe(int* fds)
{
	int a, b;
	a = (int)__syscall2(b, SYS_pipe);
	fds[0] = a;
	fds[1] = b;
	return a;
}

int chflags(const char* path, int flags)
{
	return __syscall(SYS_chflags, path, flags);
}

int fchflags(int fd, int flags)
{
	return __syscall(SYS_fchflags, fd, flags);
}

int lchflags(const char* path, int flags)
{
	return __syscall(SYS_lchflags, path, flags);
}

int shm_open(const char *name, int oflag, mode_t mode)
{
	return __syscall(SYS_shm_open, name, oflag, mode);
}

int shm_unlink(const char *name)
{
	return __syscall(SYS_shm_unlink, name);
}
