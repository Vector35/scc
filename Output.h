#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "ILBlock.h"
#include "Settings.h"
#include "TreeNode.h"
#include <string>
#include <vector>


enum RelocationType
{
	CODE_RELOC_RELATIVE_8,
	CODE_RELOC_RELATIVE_32,
	CODE_RELOC_RELATIVE_32_FIELD,
	CODE_RELOC_RELATIVE_64_SPLIT_FIELD,
	CODE_RELOC_RELATIVE_CUSTOM_FIELD,
	CODE_RELOC_BASE_RELATIVE_8,
	CODE_RELOC_BASE_RELATIVE_32,
	CODE_RELOC_ABSOLUTE_32,
	CODE_RELOC_ABSOLUTE_64,
	DATA_RELOC_RELATIVE_8,
	DATA_RELOC_RELATIVE_32,
	DATA_RELOC_RELATIVE_32_FIELD,
	DATA_RELOC_RELATIVE_64_SPLIT_FIELD,
	DATA_RELOC_RELATIVE_CUSTOM_FIELD,
	DATA_RELOC_BASE_RELATIVE_8,
	DATA_RELOC_BASE_RELATIVE_32,
	DATA_RELOC_ABSOLUTE_32,
	DATA_RELOC_ABSOLUTE_64,
};

struct Relocation
{
	RelocationType type;
	size_t offset, instruction, start, end;
	size_t bitOffset, bitSize, bitShift;
	size_t secondBitOffset, secondBitSize, secondBitShift;
	uint32_t extra;
	void (*overflow)(OutputBlock* out, Relocation& reloc);
	int64_t (*read)(OutputBlock* out, Relocation& reloc);
	void (*write)(OutputBlock* out, Relocation& reloc, int64_t diff);
	bool (*valid)(OutputBlock* out, Relocation& reloc, int64_t diff);
	union
	{
		ILBlock* target;
		size_t dataOffset;
	};
};

struct OutputBlock
{
	void* code;
	size_t len, maxLen, randomLen;
	std::vector<Relocation> relocs;
	bool bigEndian;

	void* PrepareWrite(size_t desiredLen);
	void FinishWrite(size_t written);
	void Write(const void* data, size_t len);
	void WriteInteger(int64_t i);
	void WriteString(const std::string& str);

	void WriteInt8(int8_t value);
	void WriteInt16(int16_t value);
	void WriteInt32(int32_t value);
	void WriteInt64(int64_t value);
	void WriteUInt8(uint8_t value);
	void WriteUInt16(uint16_t value);
	void WriteUInt32(uint32_t value);
	void WriteUInt64(uint64_t value);

	void WriteFloat(float value);
	void WriteDouble(double value);

	int8_t ReadOffsetInt8(size_t ofs);
	int16_t ReadOffsetInt16(size_t ofs);
	int32_t ReadOffsetInt32(size_t ofs);
	int64_t ReadOffsetInt64(size_t ofs);
	uint8_t ReadOffsetUInt8(size_t ofs);
	uint16_t ReadOffsetUInt16(size_t ofs);
	uint32_t ReadOffsetUInt32(size_t ofs);
	uint64_t ReadOffsetUInt64(size_t ofs);
	void WriteOffsetInt8(size_t ofs, int8_t value);
	void WriteOffsetInt16(size_t ofs, int16_t value);
	void WriteOffsetInt32(size_t ofs, int32_t value);
	void WriteOffsetInt64(size_t ofs, int64_t value);
	void WriteOffsetUInt8(size_t ofs, uint8_t value);
	void WriteOffsetUInt16(size_t ofs, uint16_t value);
	void WriteOffsetUInt32(size_t ofs, uint32_t value);
	void WriteOffsetUInt64(size_t ofs, uint64_t value);

	void ReplaceInstruction(
	    size_t offset, size_t origLen, const void* newInstr, size_t newLen, size_t newRelocOffset);
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


class TreeBlock;

class Output
{
 protected:
	Settings m_settings;
	Function* m_startFunc;

 public:
	Output(const Settings& settings, Function* startFunc);
	virtual ~Output();

	virtual bool GenerateCode(Function* func) = 0;

	virtual TreeNode* GenerateCall(TreeBlock* block, TreeNode* func, CallingConvention conv,
	    size_t fixedParams, const std::vector<Ref<TreeNode>>& params, TreeNodeType resultType) = 0;
	virtual TreeNode* GenerateSyscall(TreeBlock* block, TreeNode* num,
	    const std::vector<Ref<TreeNode>>& params, TreeNodeType resultType) = 0;
};


uint16_t BE16(uint16_t val);
uint32_t BE32(uint32_t val);
uint64_t BE64(uint64_t val);


#endif
