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

#ifndef __STRUCT_H__
#define __STRUCT_H__

#include <string>
#include <vector>
#include <map>
#include "RefCountObject.h"
#include "Type.h"


struct StructMember
{
	Ref<Type> type;
	std::string name;
	size_t offset;
};


class ParserState;

class Struct: public RefCountObject
{
	std::string m_name;
	std::vector<StructMember> m_members;
	std::map<std::string, StructMember> m_membersByName;
	size_t m_width, m_alignment;
	bool m_union;
	bool m_fullyDefined;

	size_t m_serializationIndex;
	bool m_serializationIndexValid;
	static size_t m_nextSerializationIndex;
	static std::map< size_t, Ref<Struct> > m_serializationMap;

public:
	Struct();
	Struct(bool isUnion);

	Struct* Duplicate(DuplicateContext& dup);

	void ReplaceWith(Struct* s);

	bool IsFullyDefined() const { return m_fullyDefined; }
	void Complete();

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	const std::vector<StructMember>& GetMembers() const { return m_members; }
	bool HasMember(const std::string& name) const { return m_membersByName.find(name) != m_membersByName.end(); }
	StructMember GetMember(ParserState* state, const std::string& name) const;
	const StructMember* GetMember(const std::string& name) const;

	size_t GetWidth() const { return m_width; }
	size_t GetAlignment() const { return m_alignment; }
	bool IsUnion() const { return m_union; }

	void AddMember(ParserState* state, Type* type, const std::string& name);
	void CopyMembers(ParserState* state, Struct* s);

	void Serialize(OutputBlock* output);
	static Struct* Deserialize(InputBlock* input);
};


#endif

