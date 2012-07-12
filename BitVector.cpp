#include <string.h>
#include "BitVector.h"


BitVector::BitVector(): m_data(NULL), m_size(0)
{
}


BitVector::BitVector(const BitVector& v)
{
	if (v.m_size == 0)
	{
		m_data = NULL;
		m_size = 0;
	}
	else
	{
		m_data = new uint32_t[(v.m_size + 31) / 32];
		m_size = v.m_size;
		memcpy(m_data, v.m_data, (m_size + 7) / 8);
	}
}


BitVector::~BitVector()
{
	if (m_data)
		delete[] m_data;
}


BitVector& BitVector::operator=(const BitVector& v)
{
	if (v.m_size != m_size)
		Reset(v.m_size, false);
	memcpy(m_data, v.m_data, (m_size + 7) / 8);
	return *this;
}


bool BitVector::operator==(const BitVector& v)
{
	if (v.m_size != m_size)
		return false;
	return memcmp(m_data, v.m_data, (m_size + 7) / 8) == 0;
}


bool BitVector::operator!=(const BitVector& v)
{
	return !operator==(v);
}


void BitVector::Reset(size_t size, bool value)
{
	if (m_size != size)
	{
		if (m_data)
			delete[] m_data;
		m_data = new uint32_t[(size + 31) / 32];
		m_size = size;
	}

	memset(m_data, value ? 0xff : 0, (size + 7) / 8);
	if ((m_size & 31) != 0)
		m_data[(m_size - 1) / 32] &= (1 << (m_size & 31)) - 1;
}


bool BitVector::GetBit(size_t bit) const
{
	return (m_data[bit / 32] & (1 << (bit & 31))) != 0;
}


void BitVector::SetBit(size_t bit, bool value)
{
	if (value)
		m_data[bit / 32] |= 1 << (bit & 31);
	else
		m_data[bit / 32] &= ~(1 << (bit & 31));
}


size_t BitVector::GetSize() const
{
	return m_size;
}


void BitVector::Union(const BitVector& v)
{
	for (size_t i = 0; i < (m_size + 31) / 32; i++)
		m_data[i] |= v.m_data[i];
}


void BitVector::Intersection(const BitVector& v)
{
	for (size_t i = 0; i < (m_size + 31) / 32; i++)
		m_data[i] &= v.m_data[i];
}


void BitVector::Difference(const BitVector& v)
{
	for (size_t i = 0; i < (m_size + 31) / 32; i++)
		m_data[i] &= ~v.m_data[i];
}

