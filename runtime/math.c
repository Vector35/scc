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

#ifdef BIG_ENDIAN
#define LOW_PART 1
#define HIGH_PART 0
#else
#define LOW_PART 0
#define HIGH_PART 1
#endif

static uint64_t __udivmod64(uint64_t a, uint64_t b, uint64_t* remainder)
{
	uint64_t quotient = 0;
	uint64_t bit = 1;

	if (b == 0)
	{
		// Force divide by zero
		return (uint32_t)a / (uint32_t)b;
	}

	while ((int64_t)b >= 0)
	{
		b <<= 1;
		bit <<= 1;
	}

	while (bit)
	{
		if (b <= a)
		{
			a -= b;
			quotient += bit;
		}
		b >>= 1;
		bit >>= 1;
	}

	*remainder = a;
	return quotient;
}

uint64_t __udiv64(uint64_t a, uint64_t b)
{
	uint64_t remainder;
	return __udivmod64(a, b, &remainder);
}

uint64_t __umod64(uint64_t a, uint64_t b)
{
	uint64_t remainder;
	__udivmod64(a, b, &remainder);
	return remainder;
}

int64_t __sdiv64(int64_t a, int64_t b)
{
	bool negative = false;
	if (a < 0)
	{
		a = -a;
		negative = true;
	}
	if (b < 0)
	{
		b = -b;
		negative = !negative;
	}

	int64_t result = __udiv64(a, b);
	if (negative)
		result = -result;
	return result;
}

int64_t __smod64(int64_t a, int64_t b)
{
	bool negative = false;
	if (a < 0)
	{
		a = -a;
		negative = true;
	}
	if (b < 0)
	{
		b = -b;
	}

	int64_t result = __umod64(a, b);
	if (negative)
		result = -result;
	return result;
}

uint64_t __shl64(uint64_t a, uint8_t count)
{
	uint32_t* parts = (uint32_t*)&a;
	count &= 63;
	if (count >= 32)
	{
		parts[HIGH_PART] = parts[LOW_PART] << (count - 32);
		parts[LOW_PART] = 0;
	}
	else
	{
		parts[HIGH_PART] = (parts[HIGH_PART] << count) | (parts[LOW_PART] >> (32 - count));
		parts[LOW_PART] <<= count;
	}
	return a;
}

uint64_t __shr64(uint64_t a, uint8_t count)
{
	uint32_t* parts = (uint32_t*)&a;
	count &= 63;
	if (count >= 32)
	{
		parts[LOW_PART] = parts[HIGH_PART] >> (count - 32);
		parts[HIGH_PART] = 0;
	}
	else
	{
		parts[LOW_PART] = (parts[LOW_PART] >> count) | (parts[HIGH_PART] << (32 - count));
		parts[HIGH_PART] >>= count;
	}
	return a;
}

int64_t __sar64(int64_t a, uint8_t count)
{
	int32_t* parts = (int32_t*)&a;
	count &= 63;
	if (count >= 32)
	{
		parts[LOW_PART] = parts[HIGH_PART] >> (count - 32);
		parts[HIGH_PART] >>= 31;
	}
	else
	{
		parts[LOW_PART] = (((uint32_t)parts[LOW_PART]) >> count) | (((uint32_t)parts[HIGH_PART]) << (32 - count));
		parts[HIGH_PART] >>= count;
	}
	return a;
}

uint16_t __byteswap16(uint16_t a)
{
	return (a >> 8) | (a << 8);
}

uint32_t __byteswap32(uint32_t a)
{
	return (a >> 24) | ((a >> 8) & 0xff00) | ((a << 8) & 0xff0000) | (a << 24);
}

uint64_t __byteswap64(uint64_t a)
{
	uint32_t* parts = (uint32_t*)&a;
	uint32_t s0 = __byteswap32(parts[1]);
	uint32_t s1 = __byteswap32(parts[0]);
	parts[0] = s0;
	parts[1] = s1;
	return a;
}

