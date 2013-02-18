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

#include <stdio.h>
#include "Enum.h"
#include "Output.h"
#include "Struct.h"
#include "ParserState.h"

using namespace std;


size_t Enum::m_nextSerializationIndex = 0;
map< size_t, Ref<Enum> > Enum::m_serializationMap;


Enum::Enum()
{
	char tempName[64];
	sprintf(tempName, "<anonymous 0x%p>", this);
	m_name = tempName;

	m_nextValue = 0;
	m_fullyDefined = false;
	m_serializationIndexValid = false;
}


Enum* Enum::Duplicate(DuplicateContext& dup)
{
	if (dup.enums.find(this) != dup.enums.end())
		return dup.enums[this];

	Enum* e = new Enum();
	dup.enums[this] = e;

	e->m_name = m_name;
	e->m_members = m_members;
	e->m_nextValue = m_nextValue;
	e->m_fullyDefined = m_fullyDefined;
	return e;
}


void Enum::ReplaceWith(Enum* e)
{
	m_members = e->m_members;
	m_nextValue = e->m_nextValue;
	m_fullyDefined = e->m_fullyDefined;
}


void Enum::AddMember(const string& name, uint32_t value)
{
	EnumMember member;
	member.name = name;
	member.value = value;
	member.isDefault = false;
	m_members.push_back(member);

	m_nextValue = value + 1;
}


void Enum::Serialize(OutputBlock* output)
{
	if (m_serializationIndexValid)
	{
		output->WriteInteger(1);
		output->WriteInteger(m_serializationIndex);
		return;
	}

	m_serializationIndexValid = true;
	m_serializationIndex = m_nextSerializationIndex++;
	output->WriteInteger(0);
	output->WriteInteger(m_serializationIndex);

	output->WriteString(m_name);

	output->WriteInteger(m_members.size());
	for (vector<EnumMember>::iterator i = m_members.begin(); i != m_members.end(); i++)
	{
		output->WriteString(i->name);
		output->WriteInteger(i->value);
	}

	output->WriteInteger(m_fullyDefined ? 1 : 0);
}


Enum* Enum::Deserialize(InputBlock* input)
{
	bool existingEnum;
	size_t i;
	if (!input->ReadBool(existingEnum))
		return NULL;
	if (!input->ReadNativeInteger(i))
		return NULL;

	if (existingEnum)
		return m_serializationMap[i];

	Enum* result = new Enum();
	m_serializationMap[i] = result;

	if (!input->ReadString(result->m_name))
		return NULL;

	size_t memberCount;
	if (!input->ReadNativeInteger(memberCount))
		return NULL;
	for (size_t j = 0; j < memberCount; j++)
	{
		EnumMember member;
		member.isDefault = false;

		if (!input->ReadString(member.name))
			return NULL;
		if (!input->ReadUInt32(member.value))
			return NULL;

		result->m_members.push_back(member);
	}

	if (!input->ReadBool(result->m_fullyDefined))
		return NULL;

	return result;
}

