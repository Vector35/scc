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

void* malloc(size_t len)
{
	size_t finalLen = len + sizeof(size_t);
	void* result = mmap(NULL, (finalLen + 4095) & (~4095), PROT_READ | PROT_WRITE | PROT_EXEC,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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
	munmap(block, (fullLen + 4095) & (~4095));
}

char* strdup(const char* str)
{
	char* dest = (char*)malloc(strlen(str) + 1);
	strcpy(dest, str);
	return dest;
}

int mprotect(void* addr, size_t len, int prot)
{
	return __syscall(SYS_mprotect, addr, len, prot);
}

