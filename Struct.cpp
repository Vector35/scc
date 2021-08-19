#include "Struct.h"
#include "Output.h"
#include "ParserState.h"
#include <stdio.h>

using namespace std;


size_t Struct::m_nextSerializationIndex = 0;
map<size_t, Ref<Struct>> Struct::m_serializationMap;


Struct::Struct()
{
	char tempName[64];
	sprintf(tempName, "<anonymous 0x%p>", this);
	m_name = tempName;

	m_width = 0;
	m_alignment = 1;
	m_union = false;
	m_fullyDefined = false;
	m_packed = false;
	m_serializationIndexValid = false;
}


Struct::Struct(bool isUnion, bool packed)
{
	char tempName[64];
	sprintf(tempName, "<anonymous 0x%p>", this);
	m_name = tempName;

	m_width = 0;
	m_alignment = 1;
	m_union = isUnion;
	m_fullyDefined = false;
	m_packed = packed;
	m_serializationIndexValid = false;
}


Struct* Struct::Duplicate(DuplicateContext& dup)
{
	if (dup.structs.find(this) != dup.structs.end())
		return dup.structs[this];

	Struct* s = new Struct();
	dup.structs[this] = s;

	s->m_name = m_name;
	s->m_width = m_width;
	s->m_alignment = m_alignment;
	s->m_union = m_union;
	s->m_fullyDefined = m_fullyDefined;
	s->m_packed = m_packed;

	for (vector<StructMember>::iterator i = m_members.begin(); i != m_members.end(); i++)
	{
		StructMember member;
		member.type = i->type->Duplicate(dup);
		member.name = i->name;
		member.offset = i->offset;
		s->m_members.push_back(member);
		s->m_membersByName[i->name] = member;
	}

	return s;
}


void Struct::ReplaceWith(Struct* s)
{
	m_members = s->m_members;
	m_membersByName = s->m_membersByName;
	m_width = s->m_width;
	m_alignment = s->m_alignment;
	m_union = s->m_union;
	m_fullyDefined = s->m_fullyDefined;
	m_packed = s->m_packed;
}


void Struct::Complete()
{
	if (!m_packed)
	{
		// Ensure alignment of entire structure
		if ((m_width & (m_alignment - 1)) != 0)
			m_width += m_alignment - (m_width & (m_alignment - 1));
	}

	m_fullyDefined = true;
}


StructMember Struct::GetMember(ParserState* state, const std::string& name) const
{
	map<string, StructMember>::const_iterator i = m_membersByName.find(name);
	if (i == m_membersByName.end())
	{
		state->Error();
		fprintf(stderr, "%s:%d: error: member '%s' is not defined\n", state->GetFileName().c_str(),
		    state->GetLineNumber(), name.c_str());

		StructMember member;
		member.type = Type::VoidType();
		member.name = name;
		member.offset = 0;
		return member;
	}

	return i->second;
}


const StructMember* Struct::GetMember(const string& name) const
{
	map<string, StructMember>::const_iterator i = m_membersByName.find(name);
	if (i == m_membersByName.end())
		return NULL;
	return &i->second;
}


void Struct::AddMember(ParserState* state, Type* type, const string& name)
{
	StructMember member;
	member.type = type;
	member.name = name;

	if (state && (m_membersByName.find(name) != m_membersByName.end()))
	{
		state->Error();
		fprintf(stderr, "%s:%d: error: member '%s' is already defined\n", state->GetFileName().c_str(),
		    state->GetLineNumber(), name.c_str());
	}

	if (m_union)
	{
		member.offset = 0;
		if (type->GetWidth() > m_width)
			m_width = type->GetWidth();
	}
	else
	{
		if ((!m_packed) && ((m_width & (type->GetAlignment() - 1)) != 0))
		{
			// Needs alignment before adding this member
			m_width += type->GetAlignment() - (m_width & (type->GetAlignment() - 1));
		}

		member.offset = m_width;
		m_width += type->GetWidth();
	}

	if (!m_packed)
	{
		// Entire structure's alignment is the largest alignment requirement
		// of the individual elements
		if (type->GetAlignment() > m_alignment)
			m_alignment = type->GetAlignment();
	}

	m_members.push_back(member);
	m_membersByName[name] = member;
}


void Struct::CopyMembers(ParserState* state, Struct* s)
{
	if (m_union)
	{
		if (s->GetWidth() > m_width)
			m_width = s->GetWidth();

		for (vector<StructMember>::iterator i = s->m_members.begin(); i != s->m_members.end(); i++)
		{
			if (m_membersByName.find(i->name) != m_membersByName.end())
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: member '%s' is already defined\n",
				    state->GetFileName().c_str(), state->GetLineNumber(), i->name.c_str());
			}

			m_members.push_back(*i);
			m_membersByName[i->name] = *i;
		}
	}
	else
	{
		if ((!m_packed) && ((m_width & (s->GetAlignment() - 1)) != 0))
		{
			// Needs alignment before adding these members
			m_width += s->GetAlignment() - (m_width & (s->GetAlignment() - 1));
		}

		for (vector<StructMember>::iterator i = s->m_members.begin(); i != s->m_members.end(); i++)
		{
			if (m_membersByName.find(i->name) != m_membersByName.end())
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: member '%s' is already defined\n",
				    state->GetFileName().c_str(), state->GetLineNumber(), i->name.c_str());
			}

			// Add members to end of structure
			StructMember member = *i;
			member.offset += m_width;

			m_members.push_back(member);
			m_membersByName[i->name] = member;
		}

		m_width += s->GetWidth();
	}

	if (!m_packed)
	{
		// Entire structure's alignment is the largest alignment requirement
		// of the individual elements
		if (s->GetAlignment() > m_alignment)
			m_alignment = s->GetAlignment();
	}
}


void Struct::Serialize(OutputBlock* output)
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
	for (vector<StructMember>::iterator i = m_members.begin(); i != m_members.end(); i++)
	{
		i->type->Serialize(output);
		output->WriteString(i->name);
		output->WriteInteger(i->offset);
	}

	output->WriteInteger(m_width);
	output->WriteInteger(m_alignment);
	output->WriteInteger(m_union ? 1 : 0);
	output->WriteInteger(m_fullyDefined ? 1 : 0);
	output->WriteInteger(m_packed ? 1 : 0);
}


Struct* Struct::Deserialize(InputBlock* input)
{
	bool existingStruct;
	size_t i;
	if (!input->ReadBool(existingStruct))
		return NULL;
	if (!input->ReadNativeInteger(i))
		return NULL;

	if (existingStruct)
		return m_serializationMap[i];

	Struct* result = new Struct();
	m_serializationMap[i] = result;

	if (!input->ReadString(result->m_name))
		return NULL;

	size_t memberCount;
	if (!input->ReadNativeInteger(memberCount))
		return NULL;
	for (size_t j = 0; j < memberCount; j++)
	{
		StructMember member;
		member.type = Type::Deserialize(input);
		if (!member.type)
			return NULL;
		if (!input->ReadString(member.name))
			return NULL;
		if (!input->ReadNativeInteger(member.offset))
			return NULL;
		result->m_members.push_back(member);
		result->m_membersByName[member.name] = member;
	}

	if (!input->ReadNativeInteger(result->m_width))
		return NULL;
	if (!input->ReadNativeInteger(result->m_alignment))
		return NULL;
	if (!input->ReadBool(result->m_union))
		return NULL;
	if (!input->ReadBool(result->m_fullyDefined))
		return NULL;
	if (!input->ReadBool(result->m_packed))
		return NULL;

	return result;
}
