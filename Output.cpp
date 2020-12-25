#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Output.h"
#include "Struct.h"
#include "Variable.h"

using namespace std;


void* OutputBlock::PrepareWrite(size_t desiredLen)
{
	if ((len + desiredLen) > maxLen)
	{
		maxLen = ((len + desiredLen) * 3) / 2;
		void* newBuf = malloc(maxLen);
		if (!newBuf)
			return nullptr;

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


void OutputBlock::Write(const void* data, size_t len)
{
	void* out = PrepareWrite(len);
	memcpy(out, data, len);
	FinishWrite(len);
}


void OutputBlock::WriteInteger(int64_t i)
{
	// Integers are written in a packed form where the high bit
	// of each byte is a continuation indicator, and bit 6
	// of the first byte is a sign bit.  Integer is stored in
	// little endian byte order.
	uint8_t byte;
	bool neg = false;

	if (i < 0)
	{
		neg = true;
		i = -i;
	}

	byte = i & 0x3f;
	if (neg)
		byte |= 0x40;
	if (i >= 0x40)
		byte |= 0x80;
	Write(&byte, 1);
	i >>= 6;

	while (i != 0)
	{
		byte = i & 0x7f;
		if (i >= 0x80)
			byte |= 0x80;
		Write(&byte, 1);
		i >>= 7;
	}
}


void OutputBlock::WriteString(const string& str)
{
	WriteInteger(str.size());
	Write(str.c_str(), str.size());
}


void OutputBlock::ReplaceInstruction(size_t offset, size_t origLen, const void* newInstr, size_t newLen, size_t newRelocOffset)
{
	// Ensure there is enough space in the buffer
	if (newLen > origLen)
		PrepareWrite(newLen - origLen);

	// Move code after the instruction into place
	memmove((void*)((size_t)code + offset + newLen), (void*)((size_t)code + offset + origLen),
		(len - (offset + origLen)));

	// Copy in the new instruction
	memcpy((void*)((size_t)code + offset), newInstr, newLen);
	len += newLen - origLen;

	// Adjust relocation offsets
	for (vector<Relocation>::iterator i = relocs.begin(); i != relocs.end(); i++)
	{
		if (i->offset < offset)
			continue;

		if (i->offset < (offset + origLen))
		{
			// This instruction's relocation
			i->offset = offset + newRelocOffset;
			i->start += newLen - origLen;
			i->end += newLen - origLen;
		}
		else
		{
			// Relocation is after current instruction
			i->instruction += newLen - origLen;
			i->start += newLen - origLen;
			i->end += newLen - origLen;
			i->offset += newLen - origLen;
		}
	}

	// Check for block-internal relocations
	for (vector<Relocation>::iterator i = relocs.begin(); i != relocs.end(); i++)
	{
		switch (i->type)
		{
		case CODE_RELOC_RELATIVE_8:
		case CODE_RELOC_BASE_RELATIVE_8:
			if ((!i->target) && ((offset >= i->start) && (offset < i->end)))
			{
				*(int8_t*)((size_t)code + i->offset) += newLen - origLen;
				i->end += newLen - origLen;
			}
			break;
		case CODE_RELOC_RELATIVE_32:
		case CODE_RELOC_BASE_RELATIVE_32:
		case CODE_RELOC_ABSOLUTE_32:
			if ((!i->target) && ((offset >= i->start) && (offset < i->end)))
			{
				*(int32_t*)((size_t)code + i->offset) += newLen - origLen;
				i->end += newLen - origLen;
			}
			break;
		case CODE_RELOC_ABSOLUTE_64:
			if ((!i->target) && ((offset >= i->start) && (offset < i->end)))
			{
				*(int64_t*)((size_t)code + i->offset) += newLen - origLen;
				i->end += newLen - origLen;
			}
			break;
		default:
			break;
		}
	}
}


void OutputBlock::WriteInt8(int8_t value)
{
	Write(&value, sizeof(value));
}


void OutputBlock::WriteInt16(int16_t value)
{
	if (bigEndian)
		value = BE16(value);
	Write(&value, sizeof(value));
}


void OutputBlock::WriteInt32(int32_t value)
{
	if (bigEndian)
		value = BE32(value);
	Write(&value, sizeof(value));
}


void OutputBlock::WriteInt64(int64_t value)
{
	if (bigEndian)
		value = BE64(value);
	Write(&value, sizeof(value));
}


void OutputBlock::WriteUInt8(uint8_t value)
{
	Write(&value, sizeof(value));
}


void OutputBlock::WriteUInt16(uint16_t value)
{
	if (bigEndian)
		value = BE16(value);
	Write(&value, sizeof(value));
}


void OutputBlock::WriteUInt32(uint32_t value)
{
	if (bigEndian)
		value = BE32(value);
	Write(&value, sizeof(value));
}


void OutputBlock::WriteUInt64(uint64_t value)
{
	if (bigEndian)
		value = BE64(value);
	Write(&value, sizeof(value));
}


void OutputBlock::WriteFloat(float value)
{
	union
	{
		float floatValue;
		uint32_t intValue;
	} u;
	u.floatValue = value;
	WriteUInt32(u.intValue);
}


void OutputBlock::WriteDouble(double value)
{
	union
	{
		double floatValue;
		uint32_t intValue;
	} u;
	u.floatValue = value;
	WriteUInt64(u.intValue);
}


int8_t OutputBlock::ReadOffsetInt8(size_t ofs)
{
	int8_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	return value;
}


int16_t OutputBlock::ReadOffsetInt16(size_t ofs)
{
	int16_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	if (bigEndian)
		return BE16(value);
	return value;
}


int32_t OutputBlock::ReadOffsetInt32(size_t ofs)
{
	int32_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	if (bigEndian)
		return BE32(value);
	return value;
}


int64_t OutputBlock::ReadOffsetInt64(size_t ofs)
{
	int64_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	if (bigEndian)
		return BE64(value);
	return value;
}


uint8_t OutputBlock::ReadOffsetUInt8(size_t ofs)
{
	uint8_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	return value;
}


uint16_t OutputBlock::ReadOffsetUInt16(size_t ofs)
{
	uint16_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	if (bigEndian)
		return BE16(value);
	return value;
}


uint32_t OutputBlock::ReadOffsetUInt32(size_t ofs)
{
	uint32_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	if (bigEndian)
		return BE32(value);
	return value;
}


uint64_t OutputBlock::ReadOffsetUInt64(size_t ofs)
{
	uint64_t value;
	memcpy(&value, (char*)code + ofs, sizeof(value));
	if (bigEndian)
		return BE64(value);
	return value;
}


void OutputBlock::WriteOffsetInt8(size_t ofs, int8_t value)
{
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetInt16(size_t ofs, int16_t value)
{
	if (bigEndian)
		value = BE16(value);
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetInt32(size_t ofs, int32_t value)
{
	if (bigEndian)
		value = BE32(value);
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetInt64(size_t ofs, int64_t value)
{
	if (bigEndian)
		value = BE64(value);
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetUInt8(size_t ofs, uint8_t value)
{
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetUInt16(size_t ofs, uint16_t value)
{
	if (bigEndian)
		value = BE16(value);
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetUInt32(size_t ofs, uint32_t value)
{
	if (bigEndian)
		value = BE32(value);
	memcpy((char*)code + ofs, &value, sizeof(value));
}


void OutputBlock::WriteOffsetUInt64(size_t ofs, uint64_t value)
{
	if (bigEndian)
		value = BE64(value);
	memcpy((char*)code + ofs, &value, sizeof(value));
}


bool InputBlock::Read(void* data, size_t readLen)
{
	if ((offset + readLen) > len)
		return false;
	memcpy(data, (void*)((size_t)code + offset), readLen);
	offset += readLen;
	return true;
}


bool InputBlock::ReadInt8(int8_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (int8_t)i;
	return true;
}


bool InputBlock::ReadInt16(int16_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (int16_t)i;
	return true;
}


bool InputBlock::ReadInt32(int32_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (int32_t)i;
	return true;
}


bool InputBlock::ReadInt64(int64_t& value)
{
	uint8_t byte;
	bool neg;

	if (!Read(&byte, 1))
		return false;
	neg = (byte & 0x40) != 0;
	value = byte & 0x3f;

	size_t shift = 6;
	while (byte & 0x80)
	{
		if (!Read(&byte, 1))
			return false;

		value |= ((int64_t)(byte & 0x7f)) << shift;
		shift += 7;
	}

	if (neg)
		value = -value;
	return true;
}


bool InputBlock::ReadUInt8(uint8_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (uint8_t)i;
	return true;
}


bool InputBlock::ReadUInt16(uint16_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (uint16_t)i;
	return true;
}


bool InputBlock::ReadUInt32(uint32_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (uint32_t)i;
	return true;
}


bool InputBlock::ReadUInt64(uint64_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (uint64_t)i;
	return true;
}


bool InputBlock::ReadNativeInteger(size_t& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = (size_t)i;
	return true;
}


bool InputBlock::ReadString(std::string& str)
{
	size_t size;
	if (!ReadNativeInteger(size))
		return false;

	char* data = new char[size];
	if (!Read(data, size))
	{
		delete[] data;
		return false;
	}

	str = string(data, size);
	delete[] data;
	return true;
}


bool InputBlock::ReadBool(bool& value)
{
	int64_t i;
	if (!ReadInt64(i))
		return false;
	value = i != 0;
	return true;
}


Output::Output(const Settings& settings, Function* startFunc): m_settings(settings), m_startFunc(startFunc)
{
}


Output::~Output()
{
}


uint16_t BE16(uint16_t value)
{
	return (value >> 8) | (value << 8);
}


uint32_t BE32(uint32_t value)
{
	return ((value >> 24) & 0xff) | ((value >> 8) & 0xff00) | ((value << 8) & 0xff0000) | (value << 24);
}


uint64_t BE64(uint64_t value)
{
	return ((value >> 56) & 0xff) | ((value >> 40) & 0xff00) | ((value >> 24) & 0xff0000) |
		((value >> 8) & 0xff000000) | ((value << 8) & 0xff00000000LL) | ((value << 24) & 0xff0000000000LL) |
		((value << 40) & 0xff000000000000LL) | (value << 56);
}

