// Copyright (c) 2015 Rusty Wagner
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
	__syscall(SYS_terminate, result);
}

int transmit(int fd, const void* buf, size_t len, size_t* result)
{
	__syscall(SYS_transmit, fd, buf, len, result);
}

int receive(int fd, void* buf, size_t len, size_t* result)
{
	__syscall(SYS_receive, fd, buf, len, result);
}

int fdwait(int nfd, fd_set* readfds, fd_set* writefds, const struct timeval* timeout, int* readyfds)
{
	__syscall(SYS_fdwait, nfd, readfds, writefds, timeout, readyfds);
}

int allocate(size_t len, int execute, void** addr)
{
	__syscall(SYS_allocate, len, execute, addr);
}

int deallocate(void* addr, size_t len)
{
	__syscall(SYS_deallocate, addr, len);
}

int random(void* buf, size_t len, size_t* result)
{
	__syscall(SYS_random, buf, len, result);
}

ssize_t read(int fd, void* buf, size_t count)
{
	size_t result;
	int err = receive(fd, buf, count, &result);
	if (err)
		return -err;
	return result;
}

ssize_t write(int fd, const void* buf, size_t count)
{
	size_t result;
	int err = transmit(fd, buf, count, &result);
	if (err)
		return -err;
	return result;
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

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds, struct timeval* timeout)
{
	int result;
	int err = fdwait(nfds, readfds, writefds, timeout, &result);
	if (err)
		return -err;
	return result;
}

void* malloc(size_t len)
{
	size_t finalLen = len + sizeof(size_t);
	void* result;
	allocate((finalLen + 4095) & (~4095), 1, &result);
	*(size_t*)result = len;
	return (void*)((size_t)result + sizeof(size_t));
}

void free(void* ptr)
{
	if (ptr == NULL)
		return;
	void* block = (void*)((size_t)ptr - sizeof(size_t));
	size_t len = *(size_t*)block;
	size_t fullLen = len + sizeof(size_t);
	deallocate(block, (fullLen + 4095) & (~4095));
}

char* strdup(const char* str)
{
	char* dest = (char*)malloc(strlen(str) + 1);
	strcpy(dest, str);
	return dest;
}

ssize_t send(int fd, const void* buf, size_t n, int flags)
{
	return write(fd, buf, n);
}

ssize_t recv(int fd, void* buf, size_t n, int flags)
{
	return read(fd, buf, n);
}

ssize_t send_all(int fd, const void* buf, size_t n, int flags)
{
	size_t offset = 0;
	while (offset < n)
	{
		ssize_t result = write(fd, (const void*)((size_t)buf + offset), n - offset);
		if (result < 0)
			return result;
		offset += result;
	}
	return offset;
}

ssize_t recv_all(int fd, void* buf, size_t n, int flags)
{
	size_t offset = 0;
	while (offset < n)
	{
		ssize_t result = read(fd, (void*)((size_t)buf + offset), n - offset);
		if (result < 0)
			return result;
		offset += result;
	}
	return offset;
}

ssize_t send_string(int fd, const char* str)
{
	return send_all(fd, str, strlen(str), 0);
}

