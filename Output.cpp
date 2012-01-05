#include "Output.h"
#include "Struct.h"


void* OutputBlock::PrepareWrite(size_t desiredLen)
{
	if ((len + desiredLen) > maxLen)
	{
		void* newBuf = malloc(((len + desiredLen) * 3) / 2);
		memcpy(newBuf, code, len);
		free(code);
		code = newBuf;
	}

	return (void*)((size_t)code + len);
}


void OutputBlock::FinishWrite(size_t written)
{
	len += written;
}


Output::Output()
{
}


Output::~Output()
{
}

