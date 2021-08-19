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

int getdents(int fd, struct dirent* dirp, size_t count)
{
	ssize_t basep;
	return getdirentries(fd, dirp, count, &basep);
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

char* getcwd(char* buf, size_t size)
{
	char tmp[MAXPATHLEN];
	int fd = open(".", O_RDONLY, 0);
	if (fd < 0)
		return NULL;
	__syscall(SYS_fcntl, fd, F_GETPATH, tmp);
	close(fd);
	strncpy(buf, tmp, size);
	return buf;
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

// FIXME: This should actually be "int shm_open(const char *name, int oflag, ...)",
// with the mode being optional, but I don't think our syscall interface supports it.
// If not creating a file, the mode parameter is ignored.
int shm_open(const char* name, int oflag, mode_t mode)
{
	return __syscall(SYS_shm_open, name, oflag, mode);
}

int shm_unlink(const char* name)
{
	return __syscall(SYS_shm_unlink, name);
}
