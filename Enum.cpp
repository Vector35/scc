#include <stdio.h>
#include "Enum.h"

using namespace std;


Enum::Enum()
{
	char tempName[64];
	sprintf(tempName, "<anonymous 0x%p>", this);
	m_name = tempName;

	m_nextValue = 0;
	m_fullyDefined = false;
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

