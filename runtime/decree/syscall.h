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

#define SYS_terminate  1
#define SYS_transmit   2
#define SYS_receive    3
#define SYS_fdwait     4
#define SYS_allocate   5
#define SYS_deallocate 6
#define SYS_random     7

#define EBADF  1
#define EFAULT 2
#define EINVAL 3
#define ENOMEM 4
#define ENOSYS 5
#define EPIPE  6

#define FD_SETSIZE  1024
#define FD_ZERO(fd) memset(fd, 0, sizeof(fd_set))
#define FD_SET(d, set) (set)->fds_bits[(d) >> 5] |= (uint32_t)(1 << ((d) & 31))
#define FD_CLR(d, set) (set)->fds_bits[(d) >> 5] &= (uint32_t)~(1 << ((d) & 31))
#define FD_ISSET(d, set) ((set)->fds_bits[(d) >> 5] & (1 << ((d) & 31)))

struct timeval
{
	int tv_sec;
	int tv_usec;
};

typedef struct
{
	uint32_t fds_bits[FD_SETSIZE / sizeof(uint32_t)];
} fd_set;

void exit(int result) __noreturn;
#define terminate exit

int transmit(int fd, const void* buf, size_t len, size_t* result);
int receive(int fd, void* buf, size_t len, size_t* result);
int fdwait(int nfd, fd_set* readfds, fd_set* writefds, const struct timeval* timeout, int* readyfds);
int allocate(size_t len, int execute, void** addr);
int deallocate(void* addr, size_t len);
int random(void* buf, size_t len, size_t* result);

#define stdin  (FILE*)0
#define stdout (FILE*)1
#define stderr (FILE*)2
#define fdopen(fd) ((FILE*)(fd))

#define EOF    -1

typedef void* FILE;

ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);

int fputc(int ch, FILE* fp);
int fputs(const char* str, FILE* fp);
int puts(const char* str);

int fgetc(FILE* fp);
char* fgets(char* result, int size, FILE* fp);

int printf(const char* fmt, ...);
int fprintf(FILE* fp, const char* fmt, ...);
int vprintf(const char* fmt, va_list va);
int vfprintf(FILE* fp, const char* fmt, va_list va);

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* errorfds, struct timeval* timeout);

void* malloc(size_t len);
void free(void* ptr);
char* strdup(const char* str);

ssize_t send(int fd, const void* buf, size_t n, int flags);
ssize_t recv(int fd, void* buf, size_t n, int flags);
ssize_t send_all(int fd, const void* buf, size_t n, int flags);
ssize_t recv_all(int fd, void* buf, size_t n, int flags);
ssize_t send_string(int fd, const char* str);

