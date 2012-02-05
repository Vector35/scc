// Copyright (c) 2011-2012 Rusty Wagner
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

