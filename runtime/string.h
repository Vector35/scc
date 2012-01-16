#ifndef __LIBC__STRING_H__
#define __LIBC__STRING_H__

#include "runtime/vararg.h"

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t len);
size_t strlen(const char* str);

int sprintf(char* out, const char* fmt, ...);
int snprintf(char* out, size_t len, const char* fmt, ...);
int vsprintf(char* out, const char* fmt, va_list va);
int vsnprintf(char* out, size_t len, const char* fmt, va_list va);
int __vaprintf(void (*output)(void* ctxt, char ch), void* ctxt, const char* fmt, va_list va);

int atoi(const char* str);

#endif

