#define X86_CODEGEN_ALWAYS_INLINE
#define X86_CODEGEN_DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "asmx86.h"


#define GETCODEPTR(buf, len) buf
#define ADVANCECODEPTR(buf, len) (buf += len)
#define EMIT(buf, op) X86_DYNALLOC_EMIT64(buf, GETCODEPTR, ADVANCECODEPTR, op)
#define EMIT_R(buf, op, a) X86_DYNALLOC_EMIT64_R(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_M(buf, op, a) X86_DYNALLOC_EMIT64_M(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a))
#define EMIT_I(buf, op, a) X86_DYNALLOC_EMIT64_I(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_P(buf, op, a) X86_DYNALLOC_EMIT64_P(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_T(buf, op, a) X86_DYNALLOC_EMIT64_T(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_RR(buf, op, a, b) X86_DYNALLOC_EMIT64_RR(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b)
#define EMIT_RM(buf, op, a, b) X86_DYNALLOC_EMIT64_RM(buf, GETCODEPTR, ADVANCECODEPTR, op, a, X86_MEM_INDEX(b))
#define EMIT_MR(buf, op, a, b) X86_DYNALLOC_EMIT64_MR(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b)
#define EMIT_RI(buf, op, a, b) X86_DYNALLOC_EMIT64_RI(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b)
#define EMIT_MI(buf, op, a, b) X86_DYNALLOC_EMIT64_MI(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b)
#define EMIT_RRR(buf, op, a, b, c) X86_DYNALLOC_EMIT64_RRR(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
#define EMIT_RRI(buf, op, a, b, c) X86_DYNALLOC_EMIT64_RRI(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
#define EMIT_RMI(buf, op, a, b, c) X86_DYNALLOC_EMIT64_RMI(buf, GETCODEPTR, ADVANCECODEPTR, op, a, X86_MEM_INDEX(b), c)
#define EMIT_MRR(buf, op, a, b, c) X86_DYNALLOC_EMIT64_MRR(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b, c)
#define EMIT_MRI(buf, op, a, b, c) X86_DYNALLOC_EMIT64_MRI(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b, c)
#define EMIT_SEG(buf, op, seg) X86_DYNALLOC_EMIT64_SEG(buf, GETCODEPTR, ADVANCECODEPTR, op, seg)
#define EMIT_SEG_M(buf, op, seg, a) X86_DYNALLOC_EMIT64_SEG_M(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a))
#define EMIT_SEG_RM(buf, op, seg, a, b) X86_DYNALLOC_EMIT64_SEG_RM(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, a, X86_MEM_INDEX(b))
#define EMIT_SEG_MR(buf, op, seg, a, b) X86_DYNALLOC_EMIT64_SEG_MR(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b)
#define EMIT_SEG_MI(buf, op, seg, a, b) X86_DYNALLOC_EMIT64_SEG_MI(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b)
#define EMIT_SEG_RMI(buf, op, seg, a, b, c) X86_DYNALLOC_EMIT64_SEG_RMI(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, a, X86_MEM_INDEX(b), c)
#define EMIT_SEG_MRR(buf, op, seg, a, b, c) X86_DYNALLOC_EMIT64_SEG_MRR(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b, c)
#define EMIT_SEG_MRI(buf, op, seg, a, b, c) X86_DYNALLOC_EMIT64_SEG_MRI(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b, c)


int main()
{
	uint8* code = (uint8*)mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
	uint8* codeptr = code;

	EMIT_R(codeptr, push, REG_RBP);
	EMIT_RR(codeptr, mov_64, REG_RBP, REG_RSP);
	EMIT_R(codeptr, push, REG_RBX);
	EMIT_R(codeptr, push, REG_R12);
	EMIT_R(codeptr, push, REG_R13);
	EMIT_R(codeptr, push, REG_R14);
	EMIT_R(codeptr, push, REG_R15);
	EMIT_RR(codeptr, mov_32, REG_EAX, REG_EDI);
	EMIT_RR(codeptr, add_32, REG_EAX, REG_ESI);
	EMIT_RR(codeptr, xor_32, REG_R15D, REG_R15D);
	EMIT_RR(codeptr, xchg_64, REG_RDX, REG_R15);
	EMIT_RR(codeptr, xchg_64, REG_RCX, REG_R14);
	EMIT_RI(codeptr, mov_32, REG_ECX, 16);
	EMIT_R(codeptr, mul_32, REG_ECX);
	EMIT_RI(codeptr, shr_32, REG_EAX, 4);
	EMIT_RR(codeptr, mov_32, REG_ECX, REG_R15D);
	EMIT_RR(codeptr, add_32, REG_R14D, REG_ECX);
	EMIT_RR(codeptr, xchg_32, REG_R14D, REG_EDX);
	EMIT_RR(codeptr, add_32, REG_EDX, REG_EAX);
	EMIT_RR(codeptr, xor_32, REG_EAX, REG_EAX);
	EMIT_RR(codeptr, mov_64, REG_RSI, REG_R8);
	EMIT_RI(codeptr, shr_64, REG_RSI, 16);
	EMIT_RR(codeptr, mov_64, REG_RBX, REG_RSI);
	EMIT_RR(codeptr, or_8, REG_AL, REG_SIL);
	EMIT_RR(codeptr, or_8, REG_AH, REG_BH);
	EMIT_RI(codeptr, shl_32, REG_EAX, 16);
	EMIT_RR(codeptr, mov_16, REG_AX, REG_R8W);
	EMIT_RM(codeptr, lea_32, REG_EDX, X86_MEM_INDEX(REG_RAX, REG_RDX, 1, 0));
	EMIT_RR(codeptr, add_32, REG_EDX, REG_R9D);
	EMIT_RM(codeptr, lea_64, REG_R13, X86_MEM(REG_RBP, 16));
	EMIT_RM(codeptr, mov_32, REG_ECX, X86_MEM(REG_R13, 0));
	EMIT_RM(codeptr, lea_32, REG_EAX, X86_MEM_INDEX(REG_RDX, REG_RCX, 1, 0));
	EMIT_R(codeptr, pop, REG_R15);
	EMIT_R(codeptr, pop, REG_R14);
	EMIT_R(codeptr, pop, REG_R13);
	EMIT_R(codeptr, pop, REG_R12);
	EMIT_R(codeptr, pop, REG_RBX);
	EMIT(codeptr, leave);
	EMIT(codeptr, retn);

	Instruction instr;
	char str[512];
	codeptr = code;
	while (true)
	{
		Disassemble64(codeptr, (uint64)codeptr, 15, &instr);
		FormatInstructionString(str, 512, "%a  %8b  %7i %o", codeptr, (uint64)codeptr, &instr);
		printf("%s\n", str);
		codeptr += instr.length;
		if (instr.operation == RETN)
			break;
	}

	int result = ((int (*)(int, int, int, int, int, int, int))code) (42, 777, 11, 22, 33, 44, 55);

	if (result == (42 + 777 + 11 + 22 + 33 + 44 + 55))
		printf("Code passed\n");
	else
		printf("Code failed\n");

	return 0;
}

