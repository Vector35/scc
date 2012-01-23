#ifndef __LIBC__FILE_H__
#define __LIBC__FILE_H__

#define stdin  (FILE*)0
#define stdout (FILE*)1
#define stderr (FILE*)2

#define EOF    -1

typedef void* FILE;

int open(const char* file, int flags, int mode);
int close(int fd);
int dup(int fd);
int dup2(int oldFd, int newFd);

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

#endif

