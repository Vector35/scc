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

typedef struct
{
	uint8_t* modulus;
	size_t modulusSize;
	uint8_t* exponent;
	size_t exponentSize;
} RsaContext;


void Add(uint8_t* result, uint8_t* value, size_t size)
{
	uint8_t carry = 0;
	for (ssize_t i = size; i >= 0; i--)
	{
		uint16_t sum = ((uint16_t)result[i]) + ((uint16_t)value[i]) + (uint16_t)carry;
		carry = (uint8_t)(sum >> 8);
		result[i] = (uint8_t)sum;
	}
}


void Sub(uint8_t* result, uint8_t* value, size_t size)
{
	uint8_t borrow = 0;
	for (ssize_t i = size; i >= 0; i--)
	{
		uint16_t diff = ((uint16_t)result[i]) - (((uint16_t)value[i]) + (uint16_t)borrow);
		borrow = (uint8_t)(0x100 - (diff >> 8));
		result[i] = (uint8_t)diff;
	}
}


void Mod(uint8_t* result, uint8_t* mod, size_t size)
{
	for (ssize_t i = size; i >= 0; i--)
	{
		if (result[i] < mod[i])
			return;
		if (result[i] > mod[i])
			break;
	}

	Sub(result, mod, size);
}


void ModMult(uint8_t* result, uint8_t* a, uint8_t* b, uint8_t* mod, size_t size)
{
	memset(result, 0, size);

	uint8_t* bitAdd = (uint8_t*)alloca(size);
	memcpy(bitAdd, a, size);

	for (size_t i = 0; i < (size * 8); i++)
	{
		if (b[i / 8] & (1 << (i & 7)))
		{
			Add(result, bitAdd, size);
			Mod(result, mod, size);
		}

		Add(bitAdd, bitAdd, size);
		Mod(bitAdd, mod, size);
	}
}


void RsaCipherBlock(RsaContext* rsa, void* output, void* message, size_t len)
{
	size_t blockSize = rsa->modulusSize;

	uint8_t* result = (uint8_t*)output;
	memset(result, 0, blockSize);
	result[0] = 1;

	uint8_t* exponentMultiplier = (uint8_t*)alloca(blockSize);
	exponentMultiplier[blockSize - 1] = 0;
	exponentMultiplier[blockSize - 2] = 2;
	exponentMultiplier[len] = 0;
	for (size_t i = len + 1; i < (blockSize - 2); i++)
		exponentMultiplier[len] = 0;//(uint8_t)((rand() % 255) + 1);
	memcpy(exponentMultiplier, message, len);

	uint8_t* multResult = (uint8_t*)alloca(blockSize);

	for (size_t i = 0; i < (rsa->exponentSize * 8); i++)
	{
		if (rsa->exponent[i / 8] & (1 << (i & 7)))
		{
			ModMult(multResult, result, exponentMultiplier, rsa->modulus, blockSize);
			memcpy(result, multResult, blockSize);
		}

		ModMult(multResult, exponentMultiplier, exponentMultiplier, rsa->modulus, blockSize);
		memcpy(exponentMultiplier, multResult, blockSize);
	}
}


void main()
{
}

