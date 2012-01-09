#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <string>
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
	void WriteInteger(int64_t i);
	void WriteString(const std::string& str);
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

