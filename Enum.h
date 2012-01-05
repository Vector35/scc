#ifndef __ENUM_H__
#define __ENUM_H__

#include <string>
#include <vector>
#include <stdint.h>
#include "RefCountObject.h"


struct EnumMember
{
	std::string name;
	uint32_t value;
	bool isDefault;
};


class Enum: public RefCountObject
{
	std::string m_name;
	std::vector<EnumMember> m_members;
	uint32_t m_nextValue;
	bool m_fullyDefined;

public:
	Enum();

	void ReplaceWith(Enum* e);

	bool IsFullyDefined() const { return m_fullyDefined; }
	void Complete() { m_fullyDefined = true; }

	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

	const std::vector<EnumMember>& GetMembers() const { return m_members; }
	uint32_t GetNextValue() const { return m_nextValue; }

	void AddMember(const std::string& name, uint32_t value);
};


#endif

