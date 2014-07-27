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

#ifndef __LIBC__LINUX_MEMORY_H__
#define __LIBC__LINUX_MEMORY_H__

#define PROT_READ         1
#define PROT_WRITE        2
#define PROT_EXEC         4
#define PROT_NONE         0
#define PROT_GROWSDOWN    0x1000000
#define PROT_GROWSUP      0x2000000

#define MAP_SHARED        1
#define MAP_PRIVATE       2
#define MAP_COPY          2
#define MAP_TYPE          0xf
#define MAP_FIXED         0x10
#define MAP_UNINITIALIZED 0x4000000

#endif

