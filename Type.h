#ifndef __TYPE_H__
#define __TYPE_H__

#include <stddef.h>
#include <vector>
#include <string>
#include "RefCountObject.h"
#include "Enum.h"


enum TypeClass
{
	TYPE_VOID,
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRUCT,
	TYPE_ENUM,
	TYPE_POINTER,
	TYPE_ARRAY,
	TYPE_FUNCTION
};

enum CallingConvention
{
	CALLING_CONVENTION_DEFAULT,
	CALLING_CONVENTION_CDECL,
	CALLING_CONVENTION_STDCALL,
	CALLING_CONVENTION_FASTCALL
};


class Struct;
class OutputBlock;

class Type: public RefCountObject
{
	TypeClass m_class;
	size_t m_width, m_alignment;
	bool m_signed, m_const;
	Ref<Type> m_childType;
	CallingConvention m_callingConvention;
	std::vector< Ref<Type> > m_params;
	bool m_variableArguments;
	Ref<Struct> m_struct;
	Ref<Enum> m_enum;
	size_t m_elements;

	size_t m_serializationIndex;
	bool m_serializationIndexValid;
	static size_t m_nextSerializationIndex;
	static std::map< size_t, Ref<Type> > m_serializationMap;

	bool DeserializeInternal(InputBlock* input);

public:
	Type();
	Type(Type* type);

	Type* Duplicate(DuplicateContext& dup);

	TypeClass GetClass() const { return m_class; }
	size_t GetWidth() const;
	size_t GetAlignment() const;
	bool IsSigned() const { return m_signed; }
	bool IsConst() const { return m_const; }
	Type* GetChildType() const { return m_childType; }
	CallingConvention GetCallingConvention() const { return m_callingConvention; }
	const std::vector< Ref<Type> >& GetParams() const { return m_params; }
	bool HasVariableArguments() const { return m_variableArguments; }
	Struct* GetStruct() const { return m_struct; }
	Enum* GetEnum() const { return m_enum; }
	size_t GetElements() const { return m_elements; }

	void SetClass(TypeClass cls) { m_class = cls; }
	void SetWidth(size_t width) { m_width = width; }
	void SetAlignment(size_t align) { m_alignment = align; }
	void SetSigned(bool sign) { m_signed = sign; }
	void SetConst(bool cnst) { m_const = cnst; }
	void SetChildType(Type* type) { m_childType = type; }
	void SetCallingConvention(CallingConvention cc) { m_callingConvention = cc; }
	void SetParams(const std::vector< Ref<Type> >& params) { m_params = params; }
	void SetStruct(Struct* strct) { m_struct = strct; }
	void SetEnum(Enum* enm) { m_enum = enm; }
	void SetElements(size_t elem) { m_elements = elem; }

	bool operator==(const Type& type) const;
	bool operator!=(const Type& type) const { return !((*this) == type); }
	bool CanAssignTo(const Type& type) const;

	static Type* VoidType();
	static Type* BoolType();
	static Type* IntType(size_t width, bool sign);
	static Type* FloatType(size_t width);
	static Type* StructType(Struct* strct);
	static Type* EnumType(Enum* enm);
	static Type* PointerType(Type* type, size_t depth);
	static Type* ArrayType(Type* type, size_t elem);
	static Type* FunctionType(Type* returnValue, CallingConvention cc,
		const std::vector< std::pair< Ref<Type>, std::string > >& params);

	static Type* StructMemberType(Type* type, const std::string& name);

	void Serialize(OutputBlock* output);
	static Type* Deserialize(InputBlock* input);

#ifndef WIN32
	void Print();
#endif
};


size_t GetTargetPointerSize();
void SetTargetPointerSize(size_t size);


#endif

