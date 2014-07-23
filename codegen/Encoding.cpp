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

#include "Encoding.h"

using namespace std;


EncodingField* EncodingField::CreateNormalField(const std::string& name, size_t width)
{
	EncodingField* field = new EncodingField;
	field->type = FIELD_NORMAL;
	field->name = name;
	field->width = width;
	field->start = 0;
	return field;
}


EncodingField* EncodingField::CreateDefaultValueField(const std::string& name, size_t width, int64_t value)
{
	EncodingField* field = new EncodingField;
	field->type = FIELD_DEFAULT_VALUE;
	field->name = name;
	field->width = width;
	field->start = 0;
	field->value = value;
	return field;
}


EncodingField* EncodingField::CreateFixedValueField(size_t width, int64_t value)
{
	EncodingField* field = new EncodingField;
	field->type = FIELD_FIXED_VALUE;
	field->width = width;
	field->start = 0;
	field->value = value;
	return field;
}


Encoding::Encoding(): m_width(0)
{
}


bool Encoding::IsFieldDefined(const string& name) const
{
	map<string, size_t>::const_iterator i = m_fieldsByName.find(name);
	return i != m_fieldsByName.end();
}


size_t Encoding::GetFieldIndex(const string& name) const
{
	map<string, size_t>::const_iterator i = m_fieldsByName.find(name);
	return i->second;
}


void Encoding::AddField(const EncodingField& field)
{
	for (vector<EncodingField>::iterator i = m_fields.begin(); i != m_fields.end(); i++)
		i->start += field.width;

	EncodingField copy = field;
	copy.start = 0;
	size_t idx = m_fields.size();
	m_fields.push_back(copy);

	if (field.type != FIELD_FIXED_VALUE)
		m_fieldsByName[field.name] = idx;

	m_width += field.width;
}

