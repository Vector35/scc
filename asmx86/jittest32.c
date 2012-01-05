#define X86_CODEGEN_ALWAYS_INLINE
#define X86_CODEGEN_DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "asmx86.h"


#define GETCODEPTR(buf, len) buf
#define ADVANCECODEPTR(buf, len) (buf += len)
#define EMIT(buf, op) X86_DYNALLOC_EMIT32(buf, GETCODEPTR, ADVANCECODEPTR, op)
#define EMIT_R(buf, op, a) X86_DYNALLOC_EMIT32_R(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_M(buf, op, a) X86_DYNALLOC_EMIT32_M(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a))
#define EMIT_I(buf, op, a) X86_DYNALLOC_EMIT32_I(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_P(buf, op, a) X86_DYNALLOC_EMIT32_P(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_T(buf, op, a) X86_DYNALLOC_EMIT32_T(buf, GETCODEPTR, ADVANCECODEPTR, op, a)
#define EMIT_RR(buf, op, a, b) X86_DYNALLOC_EMIT32_RR(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b)
#define EMIT_RM(buf, op, a, b) X86_DYNALLOC_EMIT32_RM(buf, GETCODEPTR, ADVANCECODEPTR, op, a, X86_MEM_INDEX(b))
#define EMIT_MR(buf, op, a, b) X86_DYNALLOC_EMIT32_MR(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b)
#define EMIT_RI(buf, op, a, b) X86_DYNALLOC_EMIT32_RI(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b)
#define EMIT_MI(buf, op, a, b) X86_DYNALLOC_EMIT32_MI(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b)
#define EMIT_RRR(buf, op, a, b, c) X86_DYNALLOC_EMIT32_RRR(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
#define EMIT_RRI(buf, op, a, b, c) X86_DYNALLOC_EMIT32_RRI(buf, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
#define EMIT_RMI(buf, op, a, b, c) X86_DYNALLOC_EMIT32_RMI(buf, GETCODEPTR, ADVANCECODEPTR, op, a, X86_MEM_INDEX(b), c)
#define EMIT_MRR(buf, op, a, b, c) X86_DYNALLOC_EMIT32_MRR(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b, c)
#define EMIT_MRI(buf, op, a, b, c) X86_DYNALLOC_EMIT32_MRI(buf, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b, c)
#define EMIT_SEG(buf, op, seg) X86_DYNALLOC_EMIT32_SEG(buf, GETCODEPTR, ADVANCECODEPTR, op, seg)
#define EMIT_SEG_M(buf, op, seg, a) X86_DYNALLOC_EMIT32_SEG_M(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a))
#define EMIT_SEG_RM(buf, op, seg, a, b) X86_DYNALLOC_EMIT32_SEG_RM(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, a, X86_MEM_INDEX(b))
#define EMIT_SEG_MR(buf, op, seg, a, b) X86_DYNALLOC_EMIT32_SEG_MR(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b)
#define EMIT_SEG_MI(buf, op, seg, a, b) X86_DYNALLOC_EMIT32_SEG_MI(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b)
#define EMIT_SEG_RMI(buf, op, seg, a, b, c) X86_DYNALLOC_EMIT32_SEG_RMI(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, a, X86_MEM_INDEX(b), c)
#define EMIT_SEG_MRR(buf, op, seg, a, b, c) X86_DYNALLOC_EMIT32_SEG_MRR(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b, c)
#define EMIT_SEG_MRI(buf, op, seg, a, b, c) X86_DYNALLOC_EMIT32_SEG_MRI(buf, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b, c)


int main()
{
	uint8* code = (uint8*)mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
	uint8* codeptr = code;

	EMIT_R(codeptr, push, REG_EBP);
	EMIT_RR(codeptr, mov_32, REG_EBP, REG_ESP);
	EMIT_R(codeptr, push, REG_EBX);
	EMIT_R(codeptr, push, REG_ESI);
	EMIT_R(codeptr, push, REG_EDI);
	EMIT_RM(codeptr, mov_32, REG_EAX, X86_MEM(REG_EBP, 8));
	EMIT_RM(codeptr, add_32, REG_EAX, X86_MEM(REG_EBP, 12));
	EMIT_RR(codeptr, xor_32, REG_EDX, REG_EDX);
	EMIT_RI(codeptr, mov_32, REG_ECX, 16);
	EMIT_R(codeptr, mul_32, REG_ECX);
	EMIT_RI(codeptr, shr_32, REG_EAX, 4);
	EMIT_RM(codeptr, mov_32, REG_ECX, X86_MEM(REG_EBP, 16));
	EMIT_MR(codeptr, lock_add_32, X86_MEM(REG_EBP, 20), REG_ECX);
	EMIT_MR(codeptr, xchg_32, X86_MEM(REG_EBP, 20), REG_EDX);
	EMIT_RR(codeptr, add_32, REG_EDX, REG_EAX);
	EMIT_RR(codeptr, xor_32, REG_EAX, REG_EAX);
	EMIT_RM(codeptr, lea_32, REG_ECX, X86_MEM(REG_EBP, 24));
	EMIT_RM(codeptr, or_16, REG_AX, X86_MEM(REG_ECX, 2));
	EMIT_RI(codeptr, shl_32, REG_EAX, 16);
	EMIT_RM(codeptr, mov_16, REG_AX, X86_MEM(REG_ECX, 0));
	EMIT_RM(codeptr, lea_32, REG_EDX, X86_MEM_INDEX(REG_EAX, REG_EDX, 1, 0));
	EMIT_RM(codeptr, add_32, REG_EDX, X86_MEM(REG_ECX, 4));
	EMIT_RM(codeptr, mov_32, REG_ECX, X86_MEM(REG_EBP, 32));
	EMIT_RM(codeptr, lea_32, REG_EAX, X86_MEM_INDEX(REG_EDX, REG_ECX, 1, 0));
	EMIT_R(codeptr, pop, REG_EDI);
	EMIT_R(codeptr, pop, REG_ESI);
	EMIT_R(codeptr, pop, REG_EBX);
	EMIT(codeptr, leave);
	EMIT(codeptr, retn);

	Instruction instr;
	char str[512];
	codeptr = code;
	while (true)
	{
		Disassemble32(codeptr, (uint32)codeptr, 15, &instr);
		FormatInstructionString(str, 512, "%a  %8b  %7i %o", codeptr, (uint32)codeptr, &instr);
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

