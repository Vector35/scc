#define X86_SYMINSTR_PREFIX X64Sym
#define OUTPUT64

#ifdef WIN32
	#define GETCODEPTR(buf, len)     ((uint8_t*)out->PrepareWrite(len))
	#define ADVANCECODEPTR(buf, len) (out->FinishWrite(len))
	#define EMIT(op)                 X86_DYNALLOC_EMIT64(out, GETCODEPTR, ADVANCECODEPTR, op)
	#define EMIT_R(op, a)            X86_DYNALLOC_EMIT64_R(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_M(op, a)            X86_DYNALLOC_EMIT64_M(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_I(op, a)            X86_DYNALLOC_EMIT64_I(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_II(op, a, b)        X86_DYNALLOC_EMIT64_II(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_P(op, a)            X86_DYNALLOC_EMIT64_P(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_T(op, a)            X86_DYNALLOC_EMIT64_T(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_RR(op, a, b)        X86_DYNALLOC_EMIT64_RR(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_RM(op, a, b)        X86_DYNALLOC_EMIT64_RM(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_MR(op, a, b)        X86_DYNALLOC_EMIT64_MR(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_RI(op, a, b)        X86_DYNALLOC_EMIT64_RI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_MI(op, a, b)        X86_DYNALLOC_EMIT64_MI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_RRR(op, a, b, c) \
		X86_DYNALLOC_EMIT64_RRR(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_RRI(op, a, b, c) \
		X86_DYNALLOC_EMIT64_RRI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_RMI(op, a, b, c) \
		X86_DYNALLOC_EMIT64_RMI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_MRR(op, a, b, c) \
		X86_DYNALLOC_EMIT64_MRR(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_MRI(op, a, b, c) \
		X86_DYNALLOC_EMIT64_MRI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_SEG(op, seg) X86_DYNALLOC_EMIT64_SEG(out, GETCODEPTR, ADVANCECODEPTR, op, seg)
	#define EMIT_SEG_M(op, seg, a) \
		X86_DYNALLOC_EMIT64_SEG_M(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a)
	#define EMIT_SEG_RM(op, seg, a, b) \
		X86_DYNALLOC_EMIT64_SEG_RM(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, b)
	#define EMIT_SEG_MR(op, seg, a, b) \
		X86_DYNALLOC_EMIT64_SEG_MR(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, b)
	#define EMIT_SEG_MI(op, seg, a, b) \
		X86_DYNALLOC_EMIT64_SEG_MI(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, b)
	#define EMIT_SEG_RMI(op, seg, a, b, c) \
		X86_DYNALLOC_EMIT64_SEG_RMI(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, b, c)
	#define EMIT_SEG_MRR(op, seg, a, b, c) \
		X86_DYNALLOC_EMIT64_SEG_MRR(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, b, c)
	#define EMIT_SEG_MRI(op, seg, a, b, c) \
		X86_DYNALLOC_EMIT64_SEG_MRI(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, b, c)
#else
	#define GETCODEPTR(buf, len)     ((uint8_t*)out->PrepareWrite(len))
	#define ADVANCECODEPTR(buf, len) (out->FinishWrite(len))
	#define EMIT(op)                 X86_DYNALLOC_EMIT64(out, GETCODEPTR, ADVANCECODEPTR, op)
	#define EMIT_R(op, a)            X86_DYNALLOC_EMIT64_R(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_M(op, a)            X86_DYNALLOC_EMIT64_M(out, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a))
	#define EMIT_I(op, a)            X86_DYNALLOC_EMIT64_I(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_II(op, a, b)        X86_DYNALLOC_EMIT64_II(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_P(op, a)            X86_DYNALLOC_EMIT64_P(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_T(op, a)            X86_DYNALLOC_EMIT64_T(out, GETCODEPTR, ADVANCECODEPTR, op, a)
	#define EMIT_RR(op, a, b)        X86_DYNALLOC_EMIT64_RR(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_RM(op, a, b) \
		X86_DYNALLOC_EMIT64_RM(out, GETCODEPTR, ADVANCECODEPTR, op, a, X86_MEM_INDEX(b))
	#define EMIT_MR(op, a, b) \
		X86_DYNALLOC_EMIT64_MR(out, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b)
	#define EMIT_RI(op, a, b) X86_DYNALLOC_EMIT64_RI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b)
	#define EMIT_MI(op, a, b) \
		X86_DYNALLOC_EMIT64_MI(out, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b)
	#define EMIT_RRR(op, a, b, c) \
		X86_DYNALLOC_EMIT64_RRR(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_RRI(op, a, b, c) \
		X86_DYNALLOC_EMIT64_RRI(out, GETCODEPTR, ADVANCECODEPTR, op, a, b, c)
	#define EMIT_RMI(op, a, b, c) \
		X86_DYNALLOC_EMIT64_RMI(out, GETCODEPTR, ADVANCECODEPTR, op, a, X86_MEM_INDEX(b), c)
	#define EMIT_MRR(op, a, b, c) \
		X86_DYNALLOC_EMIT64_MRR(out, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b, c)
	#define EMIT_MRI(op, a, b, c) \
		X86_DYNALLOC_EMIT64_MRI(out, GETCODEPTR, ADVANCECODEPTR, op, X86_MEM_INDEX(a), b, c)
	#define EMIT_SEG(op, seg) X86_DYNALLOC_EMIT64_SEG(out, GETCODEPTR, ADVANCECODEPTR, op, seg)
	#define EMIT_SEG_M(op, seg, a) \
		X86_DYNALLOC_EMIT64_SEG_M(out, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a))
	#define EMIT_SEG_RM(op, seg, a, b) \
		X86_DYNALLOC_EMIT64_SEG_RM(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, X86_MEM_INDEX(b))
	#define EMIT_SEG_MR(op, seg, a, b) \
		X86_DYNALLOC_EMIT64_SEG_MR(out, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b)
	#define EMIT_SEG_MI(op, seg, a, b) \
		X86_DYNALLOC_EMIT64_SEG_MI(out, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b)
	#define EMIT_SEG_RMI(op, seg, a, b, c) \
		X86_DYNALLOC_EMIT64_SEG_RMI(out, GETCODEPTR, ADVANCECODEPTR, op, seg, a, X86_MEM_INDEX(b), c)
	#define EMIT_SEG_MRR(op, seg, a, b, c) \
		X86_DYNALLOC_EMIT64_SEG_MRR(out, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b, c)
	#define EMIT_SEG_MRI(op, seg, a, b, c) \
		X86_DYNALLOC_EMIT64_SEG_MRI(out, GETCODEPTR, ADVANCECODEPTR, op, seg, X86_MEM_INDEX(a), b, c)
#endif

#define DEFAULT_STACK_POINTER REG_RSP
#define DEFAULT_FRAME_POINTER REG_RBP
#define DEFAULT_BASE_POINTER  NONE

#include "X86SymInstrCommon.cpp"
