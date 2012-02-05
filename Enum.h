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

#ifndef __ENUM_H__
#define __ENUM_H__

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include "RefCountObject.h"


struct EnumMember
{
	std::string name;
	uint32_t value;
	bool isDefault;
};


class OutputBlock;
class InputBlock;
class Struct;
struct DuplicateContext;

class Enum: public RefCountObject
{
	std::string m_name;
	std::vector<EnumMember> m_members;
	uint32_t m_nextValue;
	bool m_fullyDefined;

	size_t m_serializationIndex;
	bool m_serializationIndexValid;
	static size_t m_nextSerializationIndex;
	static std::map< size_t, Ref<Enum> > m_serializationMap;

public:
	Enum();

	Enum* Duplicate(DuplicateContext& dup);

	void ReplaceWith(Enum* e);

	bool IsFullyDefined() const { return m_fullyDefined; }
	void Complete() { m_fullyDefined = true; }

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	const std::vector<EnumMember>& GetMembers() const { return m_members; }
	uint32_t GetNextValue() const { return m_nextValue; }

	void AddMember(const std::string& name, uint32_t value);

	void Serialize(OutputBlock* output);
	static Enum* Deserialize(InputBlock* input);
};


#endif

