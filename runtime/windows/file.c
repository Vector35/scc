// Copyright (c) 2011-2014 Rusty Wagner
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

int open(const char* file, uint32_t flags, int mode)
{
	uint32_t access = flags & 0xf0000000;
	uint32_t share = flags & 0xf;
	uint32_t disposition = OPEN_EXISTING;
	uint32_t attr = FILE_ATTRIBUTE_NORMAL;
	if (flags & O_CREAT)
	{
		if (flags & O_EXCL)
			disposition = CREATE_NEW;
		else
			disposition = CREATE_ALWAYS;
	}
	else if (flags & O_TRUNC)
		disposition = TRUNCATE_EXISTING;
	return (int)CreateFileA(file, access, share, NULL, disposition, attr, NULL);
}

int close(int fd)
{
	CloseHandle((HANDLE)fd);
}

ssize_t read(int fd, void* buf, size_t count)
{
	uint32_t result;
	if (!ReadFile((HANDLE)fd, buf, (uint32_t)count, &result, NULL))
		return -1;
	return result;
}

ssize_t write(int fd, const void* buf, size_t count)
{
	uint32_t result;
	if (!WriteFile((HANDLE)fd, buf, (uint32_t)count, &result, NULL))
		return -1;
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
	fputs(str, stdout);
	return fputs("\r\n", stdout);
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
	if (SetCurrentDirectoryA(path))
		return 0;
	return -1;
}

char* getcwd(char* buf, size_t size)
{
	if (!GetCurrentDirectoryA((uint32_t)size, buf))
		return NULL;
	return buf;
}

int rmdir(const char* path)
{
	if (RemoveDirectoryA(path))
		return 0;
	return -1;
}

int mkdir(const char* path, mode_t mode)
{
	if (CreateDirectoryA(path, NULL))
		return 0;
	return -1;
}

int unlink(const char* path)
{
	if (DeleteFileA(path))
		return 0;
	return -1;
}

int rename(const char* oldpath, const char* newpath)
{
	if (MoveFileA(oldpath, newpath))
		return 0;
	return -1;
}

size_t lseek(int fd, ssize_t offset, int whence)
{
	return SetFilePointer((HANDLE)fd, (int32_t)offset, NULL, whence);
}

int ftruncate(int fd, ssize_t length)
{
	lseek(fd, length, SEEK_SET);
	if (SetEndOfFile((HANDLE)fd))
		return 0;
	return -1;
}

int pipe(int* fds)
{
	HANDLE read, write;
	if (!CreatePipe(&read, &write, NULL, 0))
		return -1;
	fds[0] = (int)read;
	fds[1] = (int)write;
	return 0;
}
