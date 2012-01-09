#include "Variable.h"
#include "Struct.h"
#include "Output.h"
#include "ParserState.h"

using namespace std;


size_t Variable::m_nextSerializationIndex;
map< size_t, Ref<Variable> > Variable::m_serializationMap;


Variable::Variable()
{
	m_class = VAR_TEMP;
	m_location.lineNumber = 0;
	m_serializationIndexValid = false;
}


Variable::Variable(VariableClass cls, Type* type, const string& name)
{
	m_class = cls;
	m_type = type;
	m_name = name;
	m_location.lineNumber = 0;
	m_serializationIndexValid = false;
}


Variable::Variable(size_t paramIndex, Type* type, const string& name)
{
	m_class = VAR_PARAM;
	m_paramIndex = paramIndex;
	m_type = type;
	m_name = name;
	m_location.lineNumber = 0;
	m_serializationIndexValid = false;
}


Variable* Variable::Duplicate(DuplicateContext& dup)
{
	if (dup.vars.find(this) != dup.vars.end())
		return dup.vars[this];

	Variable* var = new Variable();
	dup.vars[this] = var;

	var->m_class = m_class;
	var->m_paramIndex = m_paramIndex;
	var->m_type = m_type->Duplicate(dup);
	var->m_name = m_name;
	var->m_location = m_location;

	return var;
}


void Variable::Serialize(OutputBlock* output)
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
	output->WriteInteger(m_paramIndex);
	m_type->Serialize(output);
	output->WriteString(m_name);

	if ((m_class != VAR_TEMP) && (m_class != VAR_LOCAL) && (m_class != VAR_PARAM))
	{
		output->WriteString(m_location.fileName);
		output->WriteInteger(m_location.lineNumber);
	}
}


bool Variable::DeserializeInternal(InputBlock* input)
{
	uint32_t cls;
	if (!input->ReadUInt32(cls))
		return false;
	m_class = (VariableClass)cls;

	if (!input->ReadNativeInteger(m_paramIndex))
		return false;
	m_type = Type::Deserialize(input);
	if (!m_type)
		return false;
	if (!input->ReadString(m_name))
		return false;

	if ((m_class != VAR_TEMP) && (m_class != VAR_LOCAL) && (m_class != VAR_PARAM))
	{
		if (!input->ReadString(m_location.fileName))
			return false;
		if (!input->ReadInt32(m_location.lineNumber))
			return false;
	}

	return true;
}


Variable* Variable::Deserialize(InputBlock* input)
{
	bool existingVar;
	size_t i;
	if (!input->ReadBool(existingVar))
		return NULL;
	if (!input->ReadNativeInteger(i))
		return NULL;

	if (existingVar)
		return m_serializationMap[i];

	Variable* var = new Variable();
	m_serializationMap[i] = var;
	if (var->DeserializeInternal(input))
		return var;
	return NULL;
}

