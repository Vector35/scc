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
	return __syscall(SYS_open, file, flags, mode);
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
	return __syscall(SYS_dup2, oldFd, newFd);
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
	return write((int)fp, &ch, 1);
}

int fputs(const char* str, FILE* fp)
{
	return write((int)fp, str, strlen(str));
}

int puts(const char* str)
{
	return fputs(str, stdout);
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
	return __syscall(SYS_rmdir, path);
}

int mkdir(const char* path, mode_t mode)
{
	return __syscall(SYS_mkdir, path, mode);
}

int unlink(const char* path)
{
	return __syscall(SYS_unlink, path);
}

int rename(const char* oldpath, const char* newpath)
{
	return __syscall(SYS_rename, oldpath, newpath);
}

int chown(const char* path, uid_t owner, gid_t group)
{
	return __syscall(SYS_chown, path, owner, group);
}

int fchown(int fd, uid_t owner, gid_t group)
{
	return __syscall(SYS_fchown, fd, owner, group);
}

int lchown(const char* path, uid_t owner, gid_t group)
{
	return __syscall(SYS_lchown, path, owner, group);
}

int chmod(const char* path, mode_t mode)
{
	return __syscall(SYS_chmod, path, mode);
}

int fchmod(int fd, mode_t mode)
{
	return __syscall(SYS_fchmod, fd, mode);
}

ssize_t readlink(const char* path, char* buf, size_t size)
{
	return __syscall(SYS_readlink, path, buf, size);
}

int link(const char* target, const char* path)
{
	return __syscall(SYS_link, target, path);
}

int symlink(const char* target, const char* path)
{
	return __syscall(SYS_symlink, target, path);
}

