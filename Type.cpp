#include <stdio.h>
#include "Type.h"
#include "Struct.h"
#include "Enum.h"

using namespace std;


size_t g_targetPointerSize = 4;


Type::Type()
{
	m_class = TYPE_VOID;
	m_width = 0;
	m_alignment = 1;
	m_signed = false;
	m_const = false;
	m_callingConvention = CALLING_CONVENTION_DEFAULT;
	m_elements = 0;
}


Type::Type(Type* type)
{
	m_class = type->m_class;
	m_width = type->m_width;
	m_alignment = type->m_alignment;
	m_signed = type->m_signed;
	m_const = type->m_const;
	m_childType = type->m_childType;
	m_callingConvention = type->m_callingConvention;
	m_params = type->m_params;
	m_struct = type->m_struct;
	m_enum = type->m_enum;
	m_elements = type->m_elements;
}


size_t Type::GetWidth() const
{
	if (m_class == TYPE_STRUCT)
		return m_struct->GetWidth();
	if (m_class == TYPE_ARRAY)
		return m_childType->GetWidth() * m_elements;
	return m_width;
}


size_t Type::GetAlignment() const
{
	if (m_class == TYPE_STRUCT)
		return m_struct->GetAlignment();
	if (m_class == TYPE_ARRAY)
		return m_childType->GetAlignment();
	return m_alignment;
}


bool Type::operator==(const Type& type) const
{
	if (m_class != type.m_class)
		return false;
	if (m_const != type.m_const)
		return false;
	if (GetWidth() != type.GetWidth())
		return false;

	switch (m_class)
	{
	case TYPE_INT:
		return m_signed == type.m_signed;
	case TYPE_STRUCT:
		return m_struct == type.m_struct;
	case TYPE_ENUM:
		return m_enum == type.m_enum;
	case TYPE_POINTER:
		return (*m_childType) == (*type.m_childType);
	case TYPE_ARRAY:
		if ((*m_childType) != (*type.m_childType))
			return false;
		return m_elements == type.m_elements;
	case TYPE_FUNCTION:
		if ((*m_childType) != (*type.m_childType))
			return false;
		if (m_callingConvention != type.m_callingConvention)
			return false;
		if (m_params.size() != type.m_params.size())
			return false;
		for (size_t i = 0; i < m_params.size(); i++)
		{
			if ((*m_params[i]) != (*type.m_params[i]))
				return false;
		}
		return true;
	default:
		return true;
	}
}


Type* Type::VoidType()
{
	return new Type();
}


Type* Type::BoolType()
{
	Type* type = new Type();
	type->m_class = TYPE_BOOL;
	type->m_width = 1;
	type->m_alignment = 1;
	return type;
}


Type* Type::IntType(size_t width, bool sign)
{
	Type* type = new Type();
	type->m_class = TYPE_INT;
	type->m_width = width;
	type->m_alignment = width;
	type->m_signed = sign;
	return type;
}


Type* Type::FloatType(size_t width)
{
	Type* type = new Type();
	type->m_class = TYPE_FLOAT;
	type->m_width = width;
	type->m_alignment = width;
	type->m_signed = true;
	return type;
}


Type* Type::StructType(Struct* strct)
{
	Type* type = new Type();
	type->m_class = TYPE_STRUCT;
	type->m_struct = strct;
	type->m_alignment = strct->GetAlignment();
	type->m_width = strct->GetWidth();
	return type;
}


Type* Type::EnumType(Enum* enm)
{
	Type* type = new Type();
	type->m_class = TYPE_ENUM;
	type->m_enum = enm;
	type->m_alignment = 4;
	type->m_width = 4;
	return type;
}


Type* Type::PointerType(Type* childType, size_t depth)
{
	Type* type = childType;
	for (size_t i = 0; i < depth; i++)
	{
		Type* newType = new Type();
		newType->m_class = TYPE_POINTER;
		newType->m_childType = type;
		newType->m_width = GetTargetPointerSize();
		newType->m_alignment = GetTargetPointerSize();
		type = newType;
	}
	return type;
}


Type* Type::ArrayType(Type* childType, size_t elem)
{
	Type* type = new Type();
	type->m_class = TYPE_ARRAY;
	type->m_childType = childType;
	type->m_elements = elem;
	type->m_width = childType->GetWidth() * elem;
	type->m_alignment = childType->m_alignment;
	return type;
}


Type* Type::FunctionType(Type* returnValue, CallingConvention cc, const vector< pair< Ref<Type>, string > >& params)
{
	Type* type = new Type();
	type->m_class = TYPE_FUNCTION;
	type->m_childType = returnValue;
	type->m_callingConvention = cc;
	type->m_width = GetTargetPointerSize();
	type->m_alignment = GetTargetPointerSize();

	for (vector< pair< Ref<Type>, string > >::const_iterator i = params.begin(); i != params.end(); i++)
		type->m_params.push_back(i->first);

	return type;
}


Type* Type::StructMemberType(Type* type, const string& name)
{
	Struct* s = new Struct();
	s->AddMember(NULL, type, name);
	return Type::StructType(s);
}


void Type::Print()
{
	if (m_const)
		fprintf(stderr, "const ");

	switch (m_class)
	{
	case TYPE_VOID:
		fprintf(stderr, "void");
		break;
	case TYPE_BOOL:
		fprintf(stderr, "bool");
		break;
	case TYPE_INT:
		if (m_signed)
			fprintf(stderr, "int%ld_t", m_width * 8);
		else
			fprintf(stderr, "uint%ld_t", m_width * 8);
		break;
	case TYPE_FLOAT:
		if (m_width == 4)
			fprintf(stderr, "float");
		else if (m_width == 8)
			fprintf(stderr, "double");
		else
			fprintf(stderr, "unspecified_float");
		break;
	case TYPE_STRUCT:
		fprintf(stderr, "%s %s", m_struct->IsUnion() ? "union" : "struct", m_struct->GetName().c_str());
		break;
	case TYPE_ENUM:
		fprintf(stderr, "enum %s", m_enum->GetName().c_str());
		break;
	case TYPE_POINTER:
		m_childType->Print();
		fprintf(stderr, "*");
		break;
	case TYPE_ARRAY:
		m_childType->Print();
		fprintf(stderr, "[%ld]", m_elements);
		break;
	case TYPE_FUNCTION:
		m_childType->Print();
		fprintf(stderr, " (");
		switch (m_callingConvention)
		{
		case CALLING_CONVENTION_CDECL:
			fprintf(stderr, "__cdecl");
			break;
		case CALLING_CONVENTION_STDCALL:
			fprintf(stderr, "__stdcall");
			break;
		case CALLING_CONVENTION_FASTCALL:
			fprintf(stderr, "__fastcall");
			break;
		default:
			break;
		}
		fprintf(stderr, "*)(");
		for (size_t i = 0; i < m_params.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_params[i]->Print();
		}
		fprintf(stderr, ")");
		break;
	default:
		fprintf(stderr, "<invalid_type>");
		break;
	}
}


size_t GetTargetPointerSize()
{
	return g_targetPointerSize;
}


void SetTargetPointerSize(size_t size)
{
	g_targetPointerSize = size;
}

