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

#ifndef __LIBC__STRING_H__
#define __LIBC__STRING_H__

#include "runtime/vararg.h"

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t len);
size_t __strlen(const char* str);
int strcmp(const char* a, const char* b);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, char ch);
char* strrchr(const char* str, char ch);

void* __memcpy(void* dest, const void* src, size_t len);
void* __memset(void* dest, char ch, size_t len);

int sprintf(char* out, const char* fmt, ...);
int snprintf(char* out, size_t len, const char* fmt, ...);
int vsprintf(char* out, const char* fmt, va_list va);
int vsnprintf(char* out, size_t len, const char* fmt, va_list va);
int __vaprintf(void (*output)(void* ctxt, char ch), void* ctxt, const char* fmt, va_list va);

int atoi(const char* str);

#endif

