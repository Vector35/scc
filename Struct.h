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

