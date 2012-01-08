#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "ILBlock.h"
#include "Settings.h"


enum RelocationType
{
	RELOC_RELATIVE_8,
	RELOC_RELATIVE_32
};

struct Relocation
{
	RelocationType type;
	size_t offset;
	ILBlock* target;
};

struct OutputBlock
{
	void* code;
	size_t len, maxLen;
	std::vector<Relocation> relocs;

	void* PrepareWrite(size_t desiredLen);
	void FinishWrite(size_t written);
	void Write(const void* data, size_t len);
};


class Output
{
protected:
	Settings m_settings;

public:
	Output(const Settings& settings);
	virtual ~Output();

	virtual bool GenerateCode(Function* func, bool finalPass) = 0;
};


#endif

