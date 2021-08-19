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

#ifndef __LIBC__MATH_H__
#define __LIBC__MATH_H__

#define DIV_MOD_DECL(size) \
	uint##size##_t __udiv##size(uint##size##_t a, uint##size##_t b); \
	uint##size##_t __umod##size(uint##size##_t a, uint##size##_t b); \
	int##size##_t __sdiv##size(int##size##_t a, int##size##_t b); \
	int##size##_t __smod##size(int##size##_t a, int##size##_t b);

DIV_MOD_DECL(8)
DIV_MOD_DECL(16)
DIV_MOD_DECL(32)
DIV_MOD_DECL(64)

uint64_t __shl64(uint64_t a, uint8_t count);
uint64_t __shr64(uint64_t a, uint8_t count);
int64_t __sar64(int64_t a, uint8_t count);
uint16_t __byteswap16(uint16_t a);
uint32_t __byteswap32(uint32_t a);
uint64_t __byteswap64(uint64_t a);

#endif
