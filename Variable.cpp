#include "Variable.h"
#include "Struct.h"

using namespace std;


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

