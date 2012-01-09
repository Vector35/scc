#include "Variable.h"
#include "Struct.h"
#include "Output.h"

using namespace std;


map< int64_t, Ref<Variable> > Variable::m_serializationMapping;


Variable::Variable()
{
	m_class = VAR_TEMP;
	m_location.lineNumber = 0;
}


Variable::Variable(VariableClass cls, Type* type, const string& name)
{
	m_class = cls;
	m_type = type;
	m_name = name;
	m_location.lineNumber = 0;
}


Variable::Variable(size_t paramIndex, Type* type, const string& name)
{
	m_class = VAR_PARAM;
	m_paramIndex = paramIndex;
	m_type = type;
	m_name = name;
	m_location.lineNumber = 0;
}


void Variable::Serialize(OutputBlock* output)
{
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


bool Variable::Deserialize(InputBlock* input)
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


Variable* Variable::GetSerializationMapping(int64_t i)
{
	return m_serializationMapping[i];
}


void Variable::SetSerializationMapping(int64_t i, Variable* var)
{
	m_serializationMapping[i] = var;
}

