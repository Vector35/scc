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

#ifndef __ASMQUARK_H__
#define __ASMQUARK_H__

#define QUARK_SR      0
#define QUARK_SWAPR0  1
#define QUARK_PGBASE  4
#define QUARK_PGADDR  8
#define QUARK_PGFLAG  9
#define QUARK_SYSNUM  12
#define QUARK_SYSIP   16
#define QUARK_INTIP   17
#define QUARK_PGIP    18
#define QUARK_INVIP   19
#define QUARK_DIVIP   20
#define QUARK_BREAKIP 21

#define QUARK_COND_LT   0
#define QUARK_COND_LE   1
#define QUARK_COND_GE   2
#define QUARK_COND_GT   3
#define QUARK_COND_EQ   4
#define QUARK_COND_NE   5
#define QUARK_COND_BTNZ 6
#define QUARK_COND_BTZ  7
#define QUARK_COND_NAN  6
#define QUARK_COND_INF  7
#define QUARK_COND(c, i) (((c) & 7) | (((i) & 3) << 3))

#define QUARK_ALWAYS      0
#define QUARK_NEVER       1
#define QUARK_IF_FALSE(n) ((((n) & 3) << 1) | 8)
#define QUARK_IF_TRUE(n)  ((((n) & 3) << 1) | 9)

#define __QUARK_PREFIX(n) __cgquark_ ## n
#define __QUARK_NAME(n, t) __cgquark_ ## n ## _ ## t

#define QUARK_EMIT(op) __QUARK_PREFIX(op) ()
#define QUARK_EMIT_1(op, a) __QUARK_PREFIX(op) (a)
#define QUARK_EMIT_1I(op, a) __QUARK_NAME(op, i) (a)
#define QUARK_EMIT_1R(op, a) __QUARK_NAME(op, r) (a)
#define QUARK_EMIT_2(op, a, b) __QUARK_PREFIX(op) (a, b)
#define QUARK_EMIT_2I(op, a, b) __QUARK_NAME(op, i) (a, b)
#define QUARK_EMIT_2F(op, a, b) __QUARK_NAME(op, f) (a, b)
#define QUARK_EMIT_2R(op, a, b, s) __QUARK_NAME(op, r) (a, b, s)
#define QUARK_EMIT_3(op, a, b, c) __QUARK_PREFIX(op) (a, b, c)
#define QUARK_EMIT_3I(op, a, b, c) __QUARK_NAME(op, i) (a, b, c)
#define QUARK_EMIT_3F(op, a, b, c) __QUARK_NAME(op, f) (a, b, c)
#define QUARK_EMIT_3R(op, a, b, c, s) __QUARK_NAME(op, r) (a, b, c, s)
#define QUARK_EMIT_4(op, a, b, c, d) __QUARK_PREFIX(op) (a, b, c, d)

#define QUARK_EMIT_COND(op, cc) (__QUARK_PREFIX(op) () | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_1(op, cc, a) (__QUARK_PREFIX(op) (a) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_1I(op, cc, a) (__QUARK_NAME(op, i) (a) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_1R(op, cc, a) (__QUARK_NAME(op, r) (a) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_2(op, cc, a, b) (__QUARK_PREFIX(op) (a, b) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_2I(op, cc, a, b) (__QUARK_NAME(op, i) (a, b) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_2F(op, cc, a, b) (__QUARK_NAME(op, f) (a, b) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_2R(op, cc, a, b, s) (__QUARK_NAME(op, r) (a, b, s) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_3(op, cc, a, b, c) (__QUARK_PREFIX(op) (a, b, c) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_3I(op, cc, a, b, c) (__QUARK_NAME(op, i) (a, b, c) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_3F(op, cc, a, b, c) (__QUARK_NAME(op, f) (a, b, c) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_3R(op, cc, a, b, c, s) (__QUARK_NAME(op, r) (a, b, c, s) | (((cc) & 15) << 28))
#define QUARK_EMIT_COND_4(op, cc, a, b, c, d) (__QUARK_PREFIX(op) (a, b, c, d) | (((cc) & 15) << 28))

#define __QUARK_IMM11(op, a, b, i) ((((op) & 63) << 22) | (((a) & 31) << 17) | (((b) & 31) << 12) | __QUARK_PREFIX(imm_encoding)(i))
#define __QUARK_IMM17(op, a, i) ((((op) & 63) << 22) | (((a) & 31) << 17) | (i & 0x1ffff))
#define __QUARK_IMM22(op, i) ((((op) & 63) << 22) | (i & 0x3fffff))
#define __QUARK_INSTR(op, a, b, c, d) ((((op) & 63) << 22) | (((a) & 31) << 17) | (((b) & 31) << 12) | (((c) & 31) << 5) | ((d) & 31))

static uint32_t __inline __QUARK_PREFIX(imm_encoding) (int32_t imm)
{
	int32_t i;
	// Check if immediate can be represented as a signed 11-bit integer
	if ((imm >= -0x400) && (imm <= 0x3ff))
		return (1 << 11) | (imm & 0x7ff);
	// Check if immediate can be represented as a rotated 5-bit integer
	for (i = 1; i < 32; i++)
	{
		int32_t rotated = ((imm >> i) & 0xffffffff) | (imm << (32 - i));
		if ((rotated >= -0x20) && (imm <= 0x1f))
			return (1 << 10) | ((rotated & 0x1f) << 5) | (i & 0x1f);
	}
	abort();
}

static uint32_t __inline __QUARK_NAME(ldb, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(0, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldb, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(0, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldh, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(1, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldh, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(1, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldw, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(2, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldw, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(2, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldmw, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(3, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldmw, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(3, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(stb, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(4, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stb, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(4, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(sth, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(5, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(sth, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(5, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stw, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(6, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stw, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(6, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stmw, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(7, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stmw, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(7, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(ldbu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(8, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldbu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(8, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldhu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(9, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldhu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(9, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldwu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(10, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldwu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(10, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldmwu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(11, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldmwu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(11, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(stbu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(12, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stbu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(12, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(sthu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(13, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(sthu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(13, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stwu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(14, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stwu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(14, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stmwu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(15, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stmwu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(15, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(ldsxb, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(16, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldsxb, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(16, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldsxh, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(17, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldsxh, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(17, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldsxbu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(18, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldsxbu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(18, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldsxhu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(19, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldsxhu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(19, ra, rb, rc, s); }

static uint32_t __inline __QUARK_PREFIX(ldi) (int ra, int32_t imm) { return __QUARK_IMM17(20, ra, imm); }
static uint32_t __inline __QUARK_PREFIX(ldih) (int ra, int32_t imm) { return __QUARK_IMM17(21, ra, imm); }
static uint32_t __inline __QUARK_PREFIX(jmp) (int32_t imm) { return __QUARK_IMM22(22, imm); }
static uint32_t __inline __QUARK_NAME(call, i) (int32_t imm) { return __QUARK_IMM22(23, imm); }

static uint32_t __inline __QUARK_NAME(add, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(24, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(add, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(24, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(sub, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(25, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(sub, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(25, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(addx, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(26, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(addx, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(26, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(subx, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(27, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(subx, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(27, ra, rb, rc, s); }
static uint32_t __inline __QUARK_PREFIX(mulx) (int rd, int ra, int rb, int rc) { return __QUARK_INSTR(28, ra, rb, rc, rd); }
static uint32_t __inline __QUARK_PREFIX(imulx) (int rd, int ra, int rb, int rc) { return __QUARK_INSTR(29, ra, rb, rc, rd); }
static uint32_t __inline __QUARK_NAME(mul, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(30, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(mul, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(30, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(mov, i) (int ra, int32_t imm) { return __QUARK_IMM11(31, ra, 0, imm); }
static uint32_t __inline __QUARK_NAME(mov, r) (int ra, int rb, int s) { return __QUARK_INSTR(31, ra, 0, rb, s); }
static uint32_t __inline __QUARK_PREFIX(xchg) (int ra, int rb) { return __QUARK_INSTR(31, ra, 1, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(sxb) (int ra, int rb) { return __QUARK_INSTR(31, ra, 2, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(sxh) (int ra, int rb) { return __QUARK_INSTR(31, ra, 3, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(swaph) (int ra, int rb) { return __QUARK_INSTR(31, ra, 4, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(swapw) (int ra, int rb) { return __QUARK_INSTR(31, ra, 5, rb, 0); }
static uint32_t __inline __QUARK_NAME(call, r) (int ra) { return __QUARK_INSTR(31, ra, 6, 0, 0); }
static uint32_t __inline __QUARK_PREFIX(neg) (int ra, int rb) { return __QUARK_INSTR(31, ra, 8, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(not) (int ra, int rb) { return __QUARK_INSTR(31, ra, 9, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(zxb) (int ra, int rb) { return __QUARK_INSTR(31, ra, 10, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(zxh) (int ra, int rb) { return __QUARK_INSTR(31, ra, 11, rb, 0); }

static uint32_t __inline __QUARK_PREFIX(ldsr) (int ra, int rb) { return __QUARK_INSTR(31, ra, 12, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(stsr) (int ra, int rb) { return __QUARK_INSTR(31, ra, 13, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(ldcr) (int ra) { return __QUARK_INSTR(31, ra, 14, 0, 0); }
static uint32_t __inline __QUARK_PREFIX(stcr) (int ra) { return __QUARK_INSTR(31, ra, 15, 0, 0); }

static uint32_t __inline __QUARK_NAME(syscall, r) (int ra) { return __QUARK_INSTR(31, ra, 16, 0, 0); }
static uint32_t __inline __QUARK_PREFIX(iret) () { return __QUARK_INSTR(31, 0, 17, 0, 0); }
static uint32_t __inline __QUARK_NAME(invpg, i) (int ra, int32_t imm) { return __QUARK_IMM11(31, ra, 18, imm); }
static uint32_t __inline __QUARK_NAME(invpg, r) (int ra, int rb, int s) { return __QUARK_INSTR(31, ra, 18, rb, s); }
static uint32_t __inline __QUARK_PREFIX(invall) () { return __QUARK_INSTR(31, 0, 19, 0, 0); }

static uint32_t __inline __QUARK_PREFIX(setcc) (int a) { return __QUARK_INSTR(31, a, 24, 0, 0); }
static uint32_t __inline __QUARK_PREFIX(clrcc) (int a) { return __QUARK_INSTR(31, a, 25, 0, 0); }
static uint32_t __inline __QUARK_PREFIX(notcc) (int a, int b) { return __QUARK_INSTR(31, a, 26, b, 0); }
static uint32_t __inline __QUARK_PREFIX(movcc) (int a, int b) { return __QUARK_INSTR(31, a, 27, b, 0); }
static uint32_t __inline __QUARK_PREFIX(andcc) (int a, int b, int c) { return __QUARK_INSTR(31, a, 28, b, c); }
static uint32_t __inline __QUARK_PREFIX(orcc) (int a, int b, int c) { return __QUARK_INSTR(31, a, 29, b, c); }
static uint32_t __inline __QUARK_PREFIX(xorcc) (int a, int b, int c) { return __QUARK_INSTR(31, a, 30, b, c); }
static uint32_t __inline __QUARK_PREFIX(bp) () { return __QUARK_INSTR(31, 31, 31, 31, 31); }

static uint32_t __inline __QUARK_NAME(div, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(32, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(div, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(32, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(idiv, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(33, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(idiv, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(33, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(mod, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(34, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(mod, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(34, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(imod, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(35, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(imod, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(35, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(and, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(36, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(and, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(36, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(or, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(37, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(or, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(37, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(xor, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(38, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(xor, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(38, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(sar, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(39, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(sar, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(39, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(shl, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(40, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(shl, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(40, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(shr, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(41, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(shr, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(41, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(rol, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(42, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(rol, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(42, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ror, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(43, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ror, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(43, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(syscall, i) (int32_t imm) { return __QUARK_IMM17(44, 0, imm); }

static uint32_t __inline __QUARK_NAME(cmp, i) (int cond, int ra, int32_t imm) { return __QUARK_IMM11(45, ra, cond, imm); }
static uint32_t __inline __QUARK_NAME(cmp, r) (int cond, int ra, int rb, int s) { return __QUARK_INSTR(45, ra, cond, rb, s); }
static uint32_t __inline __QUARK_NAME(icmp, i) (int cond, int ra, int32_t imm) { return __QUARK_IMM11(46, ra, cond, imm); }
static uint32_t __inline __QUARK_NAME(icmp, r) (int cond, int ra, int rb, int s) { return __QUARK_INSTR(46, ra, cond, rb, s); }
static uint32_t __inline __QUARK_NAME(fcmp, i) (int cond, int ra, int32_t imm) { return __QUARK_IMM11(47, ra, cond, imm); }
static uint32_t __inline __QUARK_NAME(fcmp, f) (int cond, int ra, int rb) { return __QUARK_INSTR(47, ra, cond, rb, 0); }

static uint32_t __inline __QUARK_NAME(ldfs, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(48, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldfs, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(48, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldfd, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(49, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldfd, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(49, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stfs, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(50, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stfs, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(50, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stfd, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(51, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stfd, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(51, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(ldfsu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(52, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldfsu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(52, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(ldfdu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(53, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(ldfdu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(53, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stfsu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(54, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stfsu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(54, ra, rb, rc, s); }
static uint32_t __inline __QUARK_NAME(stfdu, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(55, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(stfdu, r) (int ra, int rb, int rc, int s) { return __QUARK_INSTR(55, ra, rb, rc, s); }

static uint32_t __inline __QUARK_NAME(fadd, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(56, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(fadd, f) (int ra, int rb, int rc) { return __QUARK_INSTR(56, ra, rb, rc, 0); }
static uint32_t __inline __QUARK_NAME(fsub, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(57, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(fsub, f) (int ra, int rb, int rc) { return __QUARK_INSTR(57, ra, rb, rc, 0); }
static uint32_t __inline __QUARK_NAME(fmul, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(58, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(fmul, f) (int ra, int rb, int rc) { return __QUARK_INSTR(58, ra, rb, rc, 0); }
static uint32_t __inline __QUARK_NAME(fdiv, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(59, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(fdiv, f) (int ra, int rb, int rc) { return __QUARK_INSTR(59, ra, rb, rc, 0); }
static uint32_t __inline __QUARK_NAME(fmod, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(60, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(fmod, f) (int ra, int rb, int rc) { return __QUARK_INSTR(60, ra, rb, rc, 0); }
static uint32_t __inline __QUARK_NAME(fpow, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(61, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(fpow, f) (int ra, int rb, int rc) { return __QUARK_INSTR(61, ra, rb, rc, 0); }
static uint32_t __inline __QUARK_NAME(flog, i) (int ra, int rb, int32_t imm) { return __QUARK_IMM11(62, ra, rb, imm); }
static uint32_t __inline __QUARK_NAME(flog, f) (int ra, int rb, int rc) { return __QUARK_INSTR(62, ra, rb, rc, 0); }

static uint32_t __inline __QUARK_NAME(ldfi, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 0, imm); }
static uint32_t __inline __QUARK_NAME(ldfi, r) (int ra, int rb, int s) { return __QUARK_INSTR(63, ra, 0, rb, s); }
static uint32_t __inline __QUARK_PREFIX(stfi) (int ra, int rb) { return __QUARK_INSTR(63, ra, 1, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fldpi) (int ra) { return __QUARK_INSTR(63, ra, 2, 0, 0); }
static uint32_t __inline __QUARK_PREFIX(flde) (int ra) { return __QUARK_INSTR(63, ra, 3, 0, 0); }
static uint32_t __inline __QUARK_NAME(fex, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 4, imm); }
static uint32_t __inline __QUARK_NAME(fex, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 4, rb, 0); }
static uint32_t __inline __QUARK_NAME(f2x, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 5, imm); }
static uint32_t __inline __QUARK_NAME(f2x, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 5, rb, 0); }
static uint32_t __inline __QUARK_NAME(f10x, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 6, imm); }
static uint32_t __inline __QUARK_NAME(f10x, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 6, rb, 0); }
static uint32_t __inline __QUARK_NAME(fsqrt, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 8, imm); }
static uint32_t __inline __QUARK_NAME(fsqrt, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 8, rb, 0); }
static uint32_t __inline __QUARK_NAME(frcp, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 9, imm); }
static uint32_t __inline __QUARK_NAME(frcp, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 9, rb, 0); }
static uint32_t __inline __QUARK_NAME(frsqrt, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 10, imm); }
static uint32_t __inline __QUARK_NAME(frsqrt, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 10, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fneg) (int ra, int rb) { return __QUARK_INSTR(63, ra, 11, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fsin) (int ra, int rb) { return __QUARK_INSTR(63, ra, 12, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fcos) (int ra, int rb) { return __QUARK_INSTR(63, ra, 13, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(ftan) (int ra, int rb) { return __QUARK_INSTR(63, ra, 14, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(ffloor) (int ra, int rb) { return __QUARK_INSTR(63, ra, 15, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fasin) (int ra, int rb) { return __QUARK_INSTR(63, ra, 16, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(facos) (int ra, int rb) { return __QUARK_INSTR(63, ra, 17, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fatan) (int ra, int rb) { return __QUARK_INSTR(63, ra, 18, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fceil) (int ra, int rb) { return __QUARK_INSTR(63, ra, 19, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fsinh) (int ra, int rb) { return __QUARK_INSTR(63, ra, 20, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fcosh) (int ra, int rb) { return __QUARK_INSTR(63, ra, 21, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(ftanh) (int ra, int rb) { return __QUARK_INSTR(63, ra, 22, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fround) (int ra, int rb) { return __QUARK_INSTR(63, ra, 23, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fasinh) (int ra, int rb) { return __QUARK_INSTR(63, ra, 24, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(facosh) (int ra, int rb) { return __QUARK_INSTR(63, ra, 25, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fatanh) (int ra, int rb) { return __QUARK_INSTR(63, ra, 26, rb, 0); }
static uint32_t __inline __QUARK_PREFIX(fabs) (int ra, int rb) { return __QUARK_INSTR(63, ra, 27, rb, 0); }
static uint32_t __inline __QUARK_NAME(fln, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 28, imm); }
static uint32_t __inline __QUARK_NAME(fln, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 28, rb, 0); }
static uint32_t __inline __QUARK_NAME(flog2, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 29, imm); }
static uint32_t __inline __QUARK_NAME(flog2, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 29, rb, 0); }
static uint32_t __inline __QUARK_NAME(flog10, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 30, imm); }
static uint32_t __inline __QUARK_NAME(flog10, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 30, rb, 0); }
static uint32_t __inline __QUARK_NAME(fmov, i) (int ra, int32_t imm) { return __QUARK_IMM11(63, ra, 31, imm); }
static uint32_t __inline __QUARK_NAME(fmov, f) (int ra, int rb) { return __QUARK_INSTR(63, ra, 31, rb, 0); }

#endif

