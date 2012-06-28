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

int8_t add_two_8(int8_t a, int8_t b)
{
	return a + b;
}

int16_t add_two_16(int16_t a, int16_t b)
{
	return a + b;
}

int32_t add_two_32(int32_t a, int32_t b)
{
	return a + b;
}

int64_t add_two_64(int64_t a, int64_t b)
{
	return a + b;
}

int8_t add_var_8(int8_t a, ...)
{
	int8_t result = a;
	int8_t val;
	va_list va;
	va_start(va, a);
	do
	{
		val = va_arg(va, int8_t);
		result += val;
	} while (val != 0);
	return result;
}

int16_t add_var_16(int16_t a, ...)
{
	int16_t result = a;
	int16_t val;
	va_list va;
	va_start(va, a);
	do
	{
		val = va_arg(va, int16_t);
		result += val;
	} while (val != 0);
	return result;
}

int32_t add_var_32(int32_t a, ...)
{
	int32_t result = a;
	int32_t val;
	va_list va;
	va_start(va, a);
	do
	{
		val = va_arg(va, int32_t);
		result += val;
	} while (val != 0);
	return result;
}

int64_t add_var_64(int64_t a, ...)
{
	int64_t result = a;
	int64_t val;
	va_list va;
	va_start(va, a);
	do
	{
		val = va_arg(va, int64_t);
		result += val;
	} while (val != 0);
	return result;
}

void show_8(int8_t a)
{
	printf("%d\n", a);
}

void show_16(int16_t a)
{
	printf("%d\n", a);
}

void show_32(int32_t a)
{
	printf("%d\n", a);
}

void show_64(int64_t a)
{
	uint32_t reduce = (uint32_t)a;
	reduce ^= (uint32_t)(a >> 32);
	printf("%d\n", reduce);
}

void const_inline_1(int a)
{
	int x = 42;
	if (a == 0)
		printf("zero\n");
	else
		printf("nonzero\n");
}

void const_inline_2(int a)
{
	int x = 42;
	if (a == 0)
		printf("zero\n");
	else
		printf("nonzero\n");
}

int const_inline_3(int a, int b)
{
	int x = 42;
	int y = 17;
	return a + b;
}

void test_const_inline(void)
{
	const_inline_1(0);
	const_inline_2(17);

	int a = const_inline_3(1, 2);
	printf("%d\n", a);
}

void test_syscall_error_return(void)
{
	int result = open("Obj/this_file_must_not_exist", O_RDONLY, 0);
	if (result == -ENOENT)
		printf("ENOENT\n");
	else
		printf("%d\n", result);
}

int main()
{
	show_8(add_two_8(17, 42));
	show_16(add_two_16(14859, 3928));
	show_32(add_two_32(283211239, 321821391));
	show_64(add_two_64(248294892348923894, 2348234832984983294));
	show_8(add_var_8(17, 42, -8, -1, 29, 0));
	show_16(add_var_16(14859, 3928, -13123, 2838, 0));
	show_32(add_var_32(283211239, 321821391, -1238123, 82347, -23423489, 0));
	show_64(add_var_64((int64_t)248294892348923894, (int64_t)2348234832984983294, (int64_t)-2349832892498398,
		(int64_t)3284984, (int64_t)0));
	test_const_inline();
	test_syscall_error_return();
	return 0;
}

