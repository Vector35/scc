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

#include "runtime/string.h"
#include "runtime/posix/process.h"

#ifdef __32BIT

#define SP 0
#define LR 30
#define IP 31

#define STACK_SIZE 16384
#define MAX_PARAM_SIZE 64

#define IMM5(i) (((i) & 0x1f) | (((i) & 0x10) ? 0xffffffe0 : 0))
#define IMM11(i) (((i) & 0x7ff) | (((i) & 0x400) ? 0xfffff800 : 0))
#define IMM17(i) (((i) & 0x1ffff) | (((i) & 0x10000) ? 0xfffe0000 : 0))
#define IMM22(i) (((i) & 0x3fffff) | (((i) & 0x200000) ? 0xffc00000 : 0))
#define ROL(i, n) ((n) ? (((i) << (n)) | ((i) >> (32 - (n)))) : (i))
#define ROR(i, n) ((n) ? (((i) >> (n)) | ((i) << (32 - (n)))) : (i))

#define SMALLIMM(i) ((i) & 0x400)
#define LARGEIMM(i) ((i) & 0x800)

void quark_exec(void* buf, ...) __noreturn
{
	uint32_t r[32];
	uint8_t cc[4];

	// Allocate a VM stack and initialize registers
	void* stack = alloca(STACK_SIZE);
	r[SP] = (uint32_t)stack + STACK_SIZE;
	r[IP] = (uint32_t)buf;

	// Copy parameters to main() in the VM
	memcpy((void*)(r[SP] - MAX_PARAM_SIZE), (void*)(((size_t)&buf) + sizeof(void*)), MAX_PARAM_SIZE);
	r[SP] = r[SP] - MAX_PARAM_SIZE;

	while (true)
	{
		// Fetch and decode the instruction
		uint32_t instr = *(uint32_t*)(r[IP]);
		uint32_t cond = instr >> 28;
		uint32_t op = (instr >> 22) & 0x3f;
		uint32_t a = (instr >> 17) & 31;
		uint32_t b = (instr >> 12) & 31;
		uint32_t c = (instr >> 5) & 31;
		uint32_t d = instr & 31;
		uint32_t cval;
		uint32_t addr, result;
		uint64_t result64;

		r[IP] += 4;

		if (LARGEIMM(instr))
			cval = IMM11(instr);
		else if (SMALLIMM(instr))
			cval = ROL(IMM5(instr), d);
		else
			cval = r[c] << d;

		// Check for conditional instructions
		if (cond & 8)
		{
			// Condition bit check enabled
			if (cc[(cond >> 1) & 3] != (cond & 1))
				continue;
		}
		else if (cond & 1)
		{
			// Never execute condition
			continue;
		}

		// Dispatch instruction, only user mode instructions are present as this is
		// a user level emulation.
		// TODO: Add floating point when the compiler supports it properly.
		switch (op)
		{
		case 0x00: // ldb
			r[a] = *(uint8_t*)(r[b] + cval);
			break;
		case 0x01: // ldh
			r[a] = *(uint16_t*)(r[b] + cval);
			break;
		case 0x02: // ldw
			r[a] = *(uint32_t*)(r[b] + cval);
			break;
		case 0x03: // ldmw
			addr = r[b] + cval;
			for (uint32_t i = a; i < 31; i++)
				r[i] = *(uint32_t*)(addr + ((i - a) * 4));
			break;
		case 0x04: // stb
			*(uint8_t*)(r[b] + cval) = (uint8_t)r[a];
			break;
		case 0x05: // sth
			*(uint16_t*)(r[b] + cval) = (uint16_t)r[a];
			break;
		case 0x06: // stw
			*(uint32_t*)(r[b] + cval) = r[a];
			break;
		case 0x07: // stmw
			addr = r[b] + cval;
			for (uint32_t i = a; i < 31; i++)
				*(uint32_t*)(addr + ((i - a) * 4)) = r[i];
			break;
		case 0x08: // ldbu
			addr = r[b] + cval;
			r[b] = addr + 1;
			r[a] = *(uint8_t*)addr;
			break;
		case 0x09: // ldhu
			addr = r[b] + cval;
			r[b] = addr + 2;
			r[a] = *(uint16_t*)addr;
			break;
		case 0x0a: // ldwu
			addr = r[b] + cval;
			r[b] = addr + 4;
			r[a] = *(uint32_t*)addr;
			break;
		case 0x0b: // ldmwu
			addr = r[b] + cval;
			r[b] = addr + ((31 - a) * 4);
			for (uint32_t i = a; i < 31; i++)
				r[i] = *(uint32_t*)(addr + ((i - a) * 4));
			break;
		case 0x0c: // stbu
			addr = r[b] + cval;
			*(uint8_t*)addr = (uint8_t)r[a];
			r[b] = addr;
			break;
		case 0x0d: // sthu
			addr = r[b] + cval;
			*(uint16_t*)addr = (uint16_t)r[a];
			r[b] = addr;
			break;
		case 0x0e: // stwu
			addr = r[b] + cval;
			*(uint32_t*)addr = r[a];
			r[b] = addr;
			break;
		case 0x0f: // stmwu
			addr = r[b] + cval;
			for (uint32_t i = a; i < 31; i++)
				*(uint32_t*)(addr + ((i - a) * 4)) = r[i];
			r[b] = addr;
			break;
		case 0x10: // ldsxb
			r[a] = (int32_t)*(int8_t*)(r[b] + cval);
			break;
		case 0x11: // ldsxh
			r[a] = (int32_t)*(int16_t*)(r[b] + cval);
			break;
		case 0x12: // ldsxbu
			addr = r[b] + cval;
			r[b] = addr + 1;
			r[a] = (int32_t)*(int8_t*)addr;
			break;
		case 0x13: // ldsxhu
			addr = r[b] + cval;
			r[b] = addr + 2;
			r[a] = (int32_t)*(int16_t*)addr;
			break;
		case 0x14: // ldi
			r[a] = IMM17(instr);
			break;
		case 0x15: // ldih
			r[a] = (r[a] & 0xffff) | ((instr & 0xffff) << 16);
			break;
		case 0x16: // jmp
			r[IP] += IMM22(instr) << 2;
			break;
		case 0x17: // call
			r[LR] = r[IP];
			r[IP] += IMM22(instr) << 2;
			break;
		case 0x18: // add
			r[a] = r[b] + cval;
			break;
		case 0x19: // sub
			r[a] = r[b] - cval;
			break;
		case 0x1a: // addx
			result64 = ((uint64_t)r[b] + (uint64_t)cval + (uint64_t)cc[3]);
			r[a] = (uint32_t)result64;
			cc[3] = ((uint8_t)(result64 >> 32)) & 1;
			break;
		case 0x1b: // subx
			result64 = ((uint64_t)r[b] - ((uint64_t)cval + (uint64_t)cc[3]));
			r[a] = (uint32_t)result64;
			cc[3] = ((uint8_t)(result64 >> 32)) & 1;
			break;
		case 0x1c: // mulx
			result64 = ((uint64_t)r[b] * (uint64_t)r[c]);
			r[a] = (uint32_t)result64;
			r[d] = (uint32_t)(result64 >> 32);
			break;
		case 0x1d: // imulx
			result64 = ((int64_t)(int32_t)r[b] * (int64_t)(int32_t)r[c]);
			r[a] = (uint32_t)result64;
			r[d] = (uint32_t)(result64 >> 32);
			break;
		case 0x1e: // mul
			r[a] = r[b] * cval;
			break;
		case 0x1f: // integer group
			switch (b)
			{
			case 0x00: // mov
				r[a] = cval;
				break;
			case 0x01: // xchg
				result = r[a];
				r[a] = r[c];
				r[c] = result;
				break;
			case 0x02: // sxb
				r[a] = (int32_t)(int8_t)r[c];
				break;
			case 0x03: // sxh
				r[a] = (int32_t)(int16_t)r[c];
				break;
			case 0x04: // swaph
				r[a] = __byteswap((uint16_t)r[c]);
				break;
			case 0x05: // swapw
				r[a] = __byteswap(r[c]);
				break;
			case 0x06: // call
				addr = r[a];
				r[LR] = r[IP];
				r[IP] = addr;
				break;
			case 0x08: // neg
				r[a] = -(int32_t)r[c];
				break;
			case 0x09: // not
				r[a] = ~r[c];
				break;
			case 0x0a: // zxb
				r[a] = (uint32_t)(uint8_t)r[c];
				break;
			case 0x0b: // zxh
				r[a] = (uint32_t)(uint16_t)r[c];
				break;
			case 0x0e: // ldcr
				r[a] = ((uint32_t)cc[3] << 24) | ((uint32_t)cc[2] << 16) | ((uint32_t)cc[1] << 8) | (uint32_t)cc[0];
				break;
			case 0x0f: // stcr
				cc[0] = (uint8_t)(r[a] & 1);
				cc[1] = (uint8_t)((r[a] >> 8) & 1);
				cc[2] = (uint8_t)((r[a] >> 16) & 1);
				cc[3] = (uint8_t)((r[a] >> 24) & 1);
				break;
			case 0x10: // syscall
				r[1] = __syscall2(r[2], r[a], r[1], r[2], r[3], r[4], r[5], r[6]);
				break;
			case 0x18: // setcc
				cc[a & 3] = 1;
				break;
			case 0x19: // clrcc
				cc[a & 3] = 0;
				break;
			case 0x1a: // notcc
				cc[a & 3] = cc[c & 3] ^ 1;
				break;
			case 0x1b: // movcc
				cc[a & 3] = cc[c & 3];
				break;
			case 0x1c: // andcc
				cc[a & 3] = cc[c & 3] & cc[d & 3];
				break;
			case 0x1d: // orcc
				cc[a & 3] = cc[c & 3] | cc[d & 3];
				break;
			case 0x1e: // xorcc
				cc[a & 3] = cc[c & 3] ^ cc[d & 3];
				break;
			default:
				break;
			}
			break;
		case 0x20: // div
			r[a] = r[b] / cval;
			break;
		case 0x21: // idiv
			r[a] = ((int32_t)r[b]) / ((int32_t)cval);
			break;
		case 0x22: // mod
			r[a] = r[b] % cval;
			break;
		case 0x23: // imod
			r[a] = ((int32_t)r[b]) % ((int32_t)cval);
			break;
		case 0x24: // and
			r[a] = r[b] & cval;
			break;
		case 0x25: // or
			r[a] = r[b] | cval;
			break;
		case 0x26: // xor
			r[a] = r[b] ^ cval;
			break;
		case 0x27: // sar
			r[a] = ((int32_t)r[b]) >> cval;
			break;
		case 0x28: // shl
			r[a] = r[b] << cval;
			break;
		case 0x29: // shr
			r[a] = r[b] >> cval;
			break;
		case 0x2a: // rol
			r[a] = ROL(r[b], cval);
			break;
		case 0x2b: // ror
			r[a] = ROR(r[b], cval);
			break;
		case 0x2c: // syscall
			r[1] = __syscall2(r[2], IMM22(instr), r[1], r[2], r[3], r[4], r[5], r[6]);
			break;
		case 0x2d: // cmp
			switch (b & 7)
			{
			case 0: // less
				cc[b >> 3] = (uint8_t)((r[a] < cval) ? 1 : 0);
				break;
			case 1: // less or equal
				cc[b >> 3] = (uint8_t)((r[a] <= cval) ? 1 : 0);
				break;
			case 2: // greater or equal
				cc[b >> 3] = (uint8_t)((r[a] >= cval) ? 1 : 0);
				break;
			case 3: // greater
				cc[b >> 3] = (uint8_t)((r[a] > cval) ? 1 : 0);
				break;
			case 4: // equal
				cc[b >> 3] = (uint8_t)((r[a] == cval) ? 1 : 0);
				break;
			case 5: // not equal
				cc[b >> 3] = (uint8_t)((r[a] != cval) ? 1 : 0);
				break;
			case 6: // bit test nonzero
				cc[b >> 3] = (uint8_t)((r[a] & cval) ? 1 : 0);
				break;
			case 7: // bit test zero
				cc[b >> 3] = (uint8_t)((r[a] & cval) ? 0 : 1);
				break;
			default:
				break;
			}
			break;
		case 0x2e: // icmp
			switch (b & 7)
			{
			case 0: // less
				cc[b >> 3] = (uint8_t)((((int32_t)r[a]) < ((int32_t)cval)) ? 1 : 0);
				break;
			case 1: // less or equal
				cc[b >> 3] = (uint8_t)((((int32_t)r[a]) <= ((int32_t)cval)) ? 1 : 0);
				break;
			case 2: // greater or equal
				cc[b >> 3] = (uint8_t)((((int32_t)r[a]) >= ((int32_t)cval)) ? 1 : 0);
				break;
			case 3: // greater
				cc[b >> 3] = (uint8_t)((((int32_t)r[a]) > ((int32_t)cval)) ? 1 : 0);
				break;
			case 4: // equal
				cc[b >> 3] = (uint8_t)((r[a] == cval) ? 1 : 0);
				break;
			case 5: // not equal
				cc[b >> 3] = (uint8_t)((r[a] != cval) ? 1 : 0);
				break;
			case 6: // bit test nonzero
				cc[b >> 3] = (uint8_t)((r[a] & cval) ? 1 : 0);
				break;
			case 7: // bit test zero
				cc[b >> 3] = (uint8_t)((r[a] & cval) ? 0 : 1);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

#endif

