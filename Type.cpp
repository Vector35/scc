#include <stdio.h>
#include "Type.h"
#include "Struct.h"
#include "Enum.h"
#include "Output.h"
#include "ParserState.h"

using namespace std;


size_t g_targetPointerSize = 4;

size_t Type::m_nextSerializationIndex = 0;
map< size_t, Ref<Type> > Type::m_serializationMap;


Type::Type()
{
	m_class = TYPE_VOID;
	m_width = 0;
	m_alignment = 1;
	m_signed = false;
	m_const = false;
	m_callingConvention = CALLING_CONVENTION_DEFAULT;
	m_elements = 0;
	m_serializationIndexValid = false;
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
	m_variableArguments = type->m_variableArguments;
	m_struct = type->m_struct;
	m_enum = type->m_enum;
	m_elements = type->m_elements;
	m_serializationIndexValid = false;
}


Type* Type::Duplicate(DuplicateContext& dup)
{
	if (dup.types.find(this) != dup.types.end())
		return dup.types[this];

	Type* type = new Type(this);
	dup.types[this] = type;

	if (type->m_struct)
		type->m_struct = type->m_struct->Duplicate(dup);
	if (type->m_enum)
		type->m_enum = type->m_enum->Duplicate(dup);
	if (type->m_childType)
		type->m_childType = type->m_childType->Duplicate(dup);

	for (vector< Ref<Type> >::iterator i = type->m_params.begin(); i != type->m_params.end(); i++)
		*i = (*i)->Duplicate(dup);

	return type;
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
	if (GetWidth() != type.GetWidth())
		return false;

	switch (m_class)
	{
	case TYPE_INT:
		return m_signed == type.m_signed;
	case TYPE_STRUCT:
		return m_struct->GetName() == type.m_struct->GetName();
	case TYPE_ENUM:
		return m_enum->GetName() == type.m_enum->GetName();
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
		if (m_variableArguments != type.m_variableArguments)
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


bool Type::CanAssignTo(const Type& type) const
{
	if ((m_class != type.m_class) && (!((m_class == TYPE_ARRAY) && (type.m_class == TYPE_POINTER))) &&
		(!((m_class == TYPE_ENUM) && (type.m_class == TYPE_INT))))
		return false;

	switch (m_class)
	{
	case TYPE_STRUCT:
		return m_struct == type.m_struct;
	case TYPE_ENUM:
		if (type.m_class == TYPE_INT)
			return true;
		return m_enum == type.m_enum;
	case TYPE_POINTER:
		if (type.m_childType->m_class == TYPE_VOID)
			return true;
		return (*m_childType) == (*type.m_childType);
	case TYPE_ARRAY:
		if (type.m_class == TYPE_POINTER)
		{
			if (type.m_childType->m_class == TYPE_VOID)
				return true;
			return (*m_childType) == (*type.m_childType);
		}
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
		if (m_variableArguments != type.m_variableArguments)
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
	type->m_variableArguments = false;

	for (vector< pair< Ref<Type>, string > >::const_iterator i = params.begin(); i != params.end(); i++)
	{
		if (i->second == "...")
			type->m_variableArguments = true;
		else
			type->m_params.push_back(i->first);
	}

	return type;
}


Type* Type::StructMemberType(Type* type, const string& name)
{
	Struct* s = new Struct();
	s->AddMember(NULL, type, name);
	return Type::StructType(s);
}


void Type::Serialize(OutputBlock* output)
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

	output->WriteInteger(m_class);
	output->WriteInteger(m_width);
	output->WriteInteger(m_alignment);
	output->WriteInteger(m_const ? 1 : 0);

	switch (m_class)
	{
	case TYPE_INT:
		output->WriteInteger(m_signed ? 1 : 0);
		break;
	case TYPE_STRUCT:
		m_struct->Serialize(output);
		break;
	case TYPE_ENUM:
		m_enum->Serialize(output);
		break;
	case TYPE_POINTER:
		m_childType->Serialize(output);
		break;
	case TYPE_ARRAY:
		m_childType->Serialize(output);
		output->WriteInteger(m_elements);
		break;
	case TYPE_FUNCTION:
		m_childType->Serialize(output);
		output->WriteInteger(m_callingConvention);
		output->WriteInteger(m_params.size());
		for (vector< Ref<Type> >::iterator i = m_params.begin(); i != m_params.end(); i++)
			(*i)->Serialize(output);
		output->WriteInteger(m_variableArguments ? 1 : 0);
		break;
	default:
		break;
	}
}


bool Type::DeserializeInternal(InputBlock* input)
{
	uint32_t cls;
	if (!input->ReadUInt32(cls))
		return false;
	m_class = (TypeClass)cls;

	if (!input->ReadNativeInteger(m_width))
		return false;
	if (!input->ReadNativeInteger(m_alignment))
		return false;
	if (!input->ReadBool(m_const))
		return false;

	uint32_t convention;
	size_t paramCount;
	switch (m_class)
	{
	case TYPE_INT:
		if (!input->ReadBool(m_signed))
			return false;
		break;
	case TYPE_STRUCT:
		m_struct = Struct::Deserialize(input);
		if (!m_struct)
			return false;
		break;
	case TYPE_ENUM:
		m_enum = Enum::Deserialize(input);
		if (!m_enum)
			return false;
		break;
	case TYPE_POINTER:
		m_childType = Type::Deserialize(input);
		if (!m_childType)
			return false;
		break;
	case TYPE_ARRAY:
		m_childType = Type::Deserialize(input);
		if (!m_childType)
			return false;
		if (!input->ReadNativeInteger(m_elements))
			return false;
		break;
	case TYPE_FUNCTION:
		m_childType = Type::Deserialize(input);
		if (!m_childType)
			return false;
		if (!input->ReadUInt32(convention))
			return false;
		m_callingConvention = (CallingConvention)convention;
		if (!input->ReadNativeInteger(paramCount))
			return false;
		for (size_t i = 0; i < paramCount; i++)
		{
			Type* type = Type::Deserialize(input);
			if (!type)
				return false;
			m_params.push_back(type);
		}
		if (!input->ReadBool(m_variableArguments))
			return false;
		break;
	default:
		break;
	}

	return true;
}


Type* Type::Deserialize(InputBlock* input)
{
	bool existingType;
	size_t i;
	if (!input->ReadBool(existingType))
		return NULL;
	if (!input->ReadNativeInteger(i))
		return NULL;

	if (existingType)
		return m_serializationMap[i];

	Type* type = new Type();
	m_serializationMap[i] = type;
	if (type->DeserializeInternal(input))
		return type;
	return NULL;
}


#ifndef WIN32
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
			fprintf(stderr, "int%d_t", (int)(m_width * 8));
		else
			fprintf(stderr, "uint%d_t", (int)(m_width * 8));
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
		fprintf(stderr, "[%d]", (int)m_elements);
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
		if (m_variableArguments)
			fprintf(stderr, ", ...");
		fprintf(stderr, ")");
		break;
	default:
		fprintf(stderr, "<invalid_type>");
		break;
	}
}
#endif


size_t GetTargetPointerSize()
{
	return g_targetPointerSize;
}


void SetTargetPointerSize(size_t size)
{
	g_targetPointerSize = size;
}

