// Copyright (c) 2014 Rusty Wagner
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

#ifndef __ENCODING_H__
#define __ENCODING_H__

#include <string>
#include <map>
#include <vector>
#include "RefCountObject.h"


enum EncodingFieldType
{
	FIELD_NORMAL,
	FIELD_DEFAULT_VALUE,
	FIELD_FIXED_VALUE
};

struct EncodingField
{
	EncodingFieldType type;
	std::string name;
	uint32_t width, start;
	int64_t value;

	static EncodingField* CreateNormalField(const std::string& name, size_t width);
	static EncodingField* CreateDefaultValueField(const std::string& name, size_t width, int64_t value);
	static EncodingField* CreateFixedValueField(size_t width, int64_t value);
};

class Encoding: public RefCountObject
{
	std::vector<EncodingField> m_fields;
	std::map<std::string, size_t> m_fieldsByName;
	uint32_t m_width;

public:
	Encoding();

	uint32_t GetWidth() const { return m_width; }

	bool IsFieldDefined(const std::string& name) const;
	size_t GetFieldIndex(const std::string& name) const;
	void AddField(const EncodingField& field);

	const std::vector<EncodingField>& GetFields() const { return m_fields; }
};


#endif

