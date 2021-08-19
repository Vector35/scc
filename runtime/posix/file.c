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

int open(const char* file, int flags, int mode)
{
#ifdef SYS_open
	return __syscall(SYS_open, file, flags, mode);
#else
	#ifdef SYS_openat
		#ifdef AT_FDCWD
	return __syscall(SYS_openat, AT_FDCWD, file, flags, mode);
		#endif
	#endif
#endif
}

int close(int fd)
{
	return __syscall(SYS_close, fd);
}

int dup(int fd)
{
	return __syscall(SYS_dup, fd);
}

int dup2(int oldFd, int newFd)
{
#ifdef SYS_dup2
	return __syscall(SYS_dup2, oldFd, newFd);
#else
	return __syscall(SYS_dup3, oldFd, newFd, 0);
#endif
}

void redirect_io(int fd)
{
	for (int i = 0; i < 3; i++)
		dup2(fd, i);
}

ssize_t read(int fd, void* buf, size_t count)
{
	return __syscall(SYS_read, fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
	return __syscall(SYS_write, fd, buf, count);
}

int fputc(int ch, FILE* fp)
{
	char c = (char)ch;
	return write((int)fp, &c, 1);
}

int fputs(const char* str, FILE* fp)
{
	return write((int)fp, str, strlen(str));
}

int puts(const char* str)
{
	fputs(str, stdout);
	return fputs("\n", stdout);
}

int fgetc(FILE* fp)
{
	uint8_t ch;
	if (read((int)fp, &ch, 1) <= 0)
		return EOF;
	return ch;
}

char* fgets(char* result, int size, FILE* fp)
{
	if (size <= 0)
		return NULL;
	int offset;
	for (offset = 0; offset < (size - 1); offset++)
	{
		int ch = fgetc(fp);
		if (ch == EOF)
		{
			if (offset == 0)
				return NULL;
			break;
		}

		result[offset] = (char)ch;
		if (ch == '\n')
			break;
	}
	result[offset] = 0;
	return result;
}

static void fprintf_output(void* ctxt, char ch)
{
	if (ch != 0)
		fputc(ch, (FILE*)ctxt);
}

int printf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	return __vaprintf(fprintf_output, stdout, fmt, va);
}

int fprintf(FILE* fp, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	return __vaprintf(fprintf_output, fp, fmt, va);
}

int vprintf(const char* fmt, va_list va)
{
	return __vaprintf(fprintf_output, stdout, fmt, va);
}

int vfprintf(FILE* fp, const char* fmt, va_list va)
{
	return __vaprintf(fprintf_output, fp, fmt, va);
}

int chdir(const char* path)
{
	return __syscall(SYS_chdir, path);
}

int fchdir(int fd)
{
	return __syscall(SYS_fchdir, fd);
}

int rmdir(const char* path)
{
#ifdef SYS_rmdir
	return __syscall(SYS_rmdir, path);
#else
	#ifdef SYS_unlinkat
		#ifdef AT_FDCWD
			#ifdef AT_REMOVEDIR
	return __syscall(SYS_unlinkat, AT_FDCWD, path, AT_REMOVEDIR);
			#endif
		#endif
	#endif
#endif
}

int mkdir(const char* path, mode_t mode)
{
#ifdef SYS_mkdir
	return __syscall(SYS_mkdir, path, mode);
#else
	#ifdef SYS_mkdirat
		#ifdef AT_FDCWD
	return __syscall(SYS_mkdirat, AT_FDCWD, path, mode);
		#endif
	#endif
#endif
}

int unlink(const char* path)
{
#ifdef SYS_unlink
	return __syscall(SYS_unlink, path);
#else
	#ifdef SYS_unlinkat
		#ifdef AT_FDCWD
	return __syscall(SYS_unlinkat, AT_FDCWD, path, 0);
		#endif
	#endif
#endif
}

int rename(const char* oldpath, const char* newpath)
{
#ifdef SYS_rename
	return __syscall(SYS_rename, oldpath, newpath);
#else
	#ifdef SYS_renameat
		#ifdef AT_FDCWD
	return __syscall(SYS_renameat, AT_FDCWD, oldpath, newpath);
		#endif
	#endif
#endif
}

int chown(const char* path, uid_t owner, gid_t group)
{
#ifdef SYS_chown
	return __syscall(SYS_chown, path, owner, group);
#else
	#ifdef SYS_fchownat
		#ifdef AT_FDCWD
	return __syscall(SYS_fchownat, AT_FDCWD, path, owner, group, 0);
		#endif
	#endif
#endif
}

int fchown(int fd, uid_t owner, gid_t group)
{
	return __syscall(SYS_fchown, fd, owner, group);
}

int lchown(const char* path, uid_t owner, gid_t group)
{
#ifdef SYS_lchown
	return __syscall(SYS_lchown, path, owner, group);
#else
	#ifdef SYS_fchownat
		#ifdef AT_FDCWD
			#ifdef AT_SYMLINK_NOFOLLOW
	return __syscall(SYS_fchownat, AT_FDCWD, path, owner, group, AT_SYMLINK_NOFOLLOW);
			#endif
		#endif
	#endif
#endif
}

int chmod(const char* path, mode_t mode)
{
#ifdef SYS_chmod
	return __syscall(SYS_chmod, path, mode);
#else
	#ifdef SYS_fchmodat
		#ifdef AT_FDCWD
	return __syscall(SYS_fchmodat, AT_FDCWD, path, mode, 0);
		#endif
	#endif
#endif
}

int fchmod(int fd, mode_t mode)
{
	return __syscall(SYS_fchmod, fd, mode);
}

ssize_t readlink(const char* path, char* buf, size_t size)
{
#ifdef SYS_readlink
	return __syscall(SYS_readlink, path, buf, size);
#else
	#ifdef SYS_readlinkat
		#ifdef AT_FDCWD
	return __syscall(SYS_readlinkat, AT_FDCWD, path, buf, size);
		#endif
	#endif
#endif
}

int link(const char* target, const char* path)
{
#ifdef SYS_link
	return __syscall(SYS_link, target, path);
#else
	#ifdef SYS_linkat
		#ifdef AT_FDCWD
	return __syscall(SYS_linkat, AT_FDCWD, target, AT_FDCWD, path, 0);
		#endif
	#endif
#endif
}

int symlink(const char* target, const char* path)
{
#ifdef SYS_symlink
	return __syscall(SYS_symlink, target, path);
#else
	#ifdef SYS_symlinkat
		#ifdef AT_FDCWD
	return __syscall(SYS_symlinkat, target, AT_FDCWD, path);
		#endif
	#endif
#endif
}

int fcntl(int fd, int cmd, size_t arg)
{
	return __syscall(SYS_fcntl, fd, cmd, arg);
}
