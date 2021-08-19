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

void rc4_init(rc4_state_t* state, void* key, size_t keylen)
{
	uint32_t i, j;
	for (i = 0; i < 256; i++)
		state->s[i] = (uint8_t)i;

	for (i = j = 0; i < 256; i++)
	{
		j = (j + ((uint8_t*)key)[i % keylen] + state->s[i]) & 255;

		uint8_t t = state->s[i];
		state->s[i] = state->s[j];
		state->s[j] = t;
	}

	state->i = 0;
	state->j = 0;
}

uint8_t rc4_output(rc4_state_t* state)
{
	uint8_t i = state->i;
	uint8_t j = state->j;

	i++;
	j += state->s[i];

	state->i = i;
	state->j = j;

	uint8_t t = state->s[j];
	state->s[j] = state->s[i];
	state->s[i] = t;

	return state->s[(t + state->s[j]) & 255];
}

void rc4_crypt(rc4_state_t* state, void* data, size_t len)
{
	for (size_t i = 0; i < len; i++)
		((uint8_t*)data)[i] ^= rc4_output(state);
}
