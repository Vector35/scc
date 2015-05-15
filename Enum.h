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


struct OutputBlock;
struct InputBlock;
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

