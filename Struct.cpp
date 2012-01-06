#include <stdio.h>
#include "Struct.h"
#include "ParserState.h"

using namespace std;


Struct::Struct()
{
	char tempName[64];
	sprintf(tempName, "<anonymous 0x%p>", this);
	m_name = tempName;

	m_width = 0;
	m_alignment = 1;
	m_union = false;
	m_fullyDefined = false;
}


Struct::Struct(bool isUnion)
{
	char tempName[64];
	sprintf(tempName, "<anonymous 0x%p>", this);
	m_name = tempName;

	m_width = 0;
	m_alignment = 1;
	m_union = isUnion;
	m_fullyDefined = false;
}


void Struct::ReplaceWith(Struct* s)
{
	m_members = s->m_members;
	m_membersByName = s->m_membersByName;
	m_width = s->m_width;
	m_alignment = s->m_alignment;
	m_union = s->m_union;
	m_fullyDefined = s->m_fullyDefined;
}


void Struct::Complete()
{
	// Ensure alignment of entire structure
	if ((m_width & (m_alignment - 1)) != 0)
		m_width += m_alignment - (m_width & (m_alignment - 1));

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

	if (m_membersByName.find(name) != m_membersByName.end())
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
		if ((m_width & (type->GetAlignment() - 1)) != 0)
		{
			// Needs alignment before adding this member
			m_width += type->GetAlignment() - (m_width & (type->GetAlignment() - 1));
		}

		member.offset = m_width;
		m_width += type->GetWidth();
	}

	// Entire structure's alignment is the largest alignment requirement
	// of the individual elements
	if (type->GetAlignment() > m_alignment)
		m_alignment = type->GetAlignment();

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
				fprintf(stderr, "%s:%d: error: member '%s' is already defined\n", state->GetFileName().c_str(),
					state->GetLineNumber(), i->name.c_str());
			}

			m_members.push_back(*i);
			m_membersByName[i->name] = *i;
		}
	}
	else
	{
		if ((m_width & (s->GetAlignment() - 1)) != 0)
		{
			// Needs alignment before adding these members
			m_width += s->GetAlignment() - (m_width & (s->GetAlignment() - 1));
		}

		for (vector<StructMember>::iterator i = s->m_members.begin(); i != s->m_members.end(); i++)
		{
			if (m_membersByName.find(i->name) != m_membersByName.end())
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: member '%s' is already defined\n", state->GetFileName().c_str(),
					state->GetLineNumber(), i->name.c_str());
			}

			// Add members to end of structure
			StructMember member = *i;
			member.offset += m_width;

			m_members.push_back(member);
			m_membersByName[i->name] = member;
		}

		m_width += s->GetWidth();
	}

	// Entire structure's alignment is the largest alignment requirement
	// of the individual elements
	if (s->GetAlignment() > m_alignment)
		m_alignment = s->GetAlignment();
}

