#ifndef __BITVECTOR_H__
#define __BITVECTOR_H__

#include <stddef.h>
#include <stdint.h>


class BitVector
{
	uint32_t* m_data;
	size_t m_size;

 public:
	BitVector();
	BitVector(const BitVector& v);
	~BitVector();

	BitVector& operator=(const BitVector& v);
	bool operator==(const BitVector& v);
	bool operator!=(const BitVector& v);

	void Reset(size_t size, bool value);

	bool GetBit(size_t bit) const;
	void SetBit(size_t bit, bool value);

	size_t GetSize() const;

	void Union(const BitVector& v);
	void Intersection(const BitVector& v);
	void Difference(const BitVector& v);
};


#endif
