#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <string>
#include "ILBlock.h"
#include "Settings.h"


enum RelocationType
{
	CODE_RELOC_RELATIVE_8,
	CODE_RELOC_RELATIVE_32,
	CODE_RELOC_BASE_RELATIVE_8,
	CODE_RELOC_BASE_RELATIVE_32,
	CODE_RELOC_ABSOLUTE_32,
	CODE_RELOC_ABSOLUTE_64,
	DATA_RELOC_RELATIVE_8,
	DATA_RELOC_RELATIVE_32,
	DATA_RELOC_BASE_RELATIVE_8,
	DATA_RELOC_BASE_RELATIVE_32,
	DATA_RELOC_ABSOLUTE_32,
	DATA_RELOC_ABSOLUTE_64
};

struct Relocation
{
	RelocationType type;
	size_t offset, instruction, start, end;
	void (*overflow)(OutputBlock* out, size_t start, size_t offset);
	union
	{
		ILBlock* target;
		size_t dataOffset;
	};
};

struct OutputBlock
{
	void* code;
	size_t len, maxLen;
	std::vector<Relocation> relocs;

	void* PrepareWrite(size_t desiredLen);
	void FinishWrite(size_t written);
	void Write(const void* data, size_t len);
	void WriteInteger(int64_t i);
	void WriteString(const std::string& str);

	void ReplaceInstruction(size_t offset, size_t origLen, const void* newInstr, size_t newLen, size_t newRelocOffset);
};

struct InputBlock
{
	void* code;
	size_t len, offset;

	bool Read(void* data, size_t len);
	bool ReadInt8(int8_t& value);
	bool ReadInt16(int16_t& value);
	bool ReadInt32(int32_t& value);
	bool ReadInt64(int64_t& value);
	bool ReadUInt8(uint8_t& value);
	bool ReadUInt16(uint16_t& value);
	bool ReadUInt32(uint32_t& value);
	bool ReadUInt64(uint64_t& value);
	bool ReadNativeInteger(size_t& value);
	bool ReadString(std::string& str);
	bool ReadBool(bool& value);
};

struct RelocationReference
{
	OutputBlock* block;
	Relocation* reloc;
};


class Output
{
protected:
	Settings m_settings;

public:
	Output(const Settings& settings);
	virtual ~Output();

	virtual bool GenerateCode(Function* func) = 0;
};


#endif

