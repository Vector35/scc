#ifndef __VARIABLE_H__
#define __VARIABLE_H__

#include "Type.h"
#include "Expr.h"


enum VariableClass
{
	VAR_TEMP,
	VAR_LOCAL,
	VAR_PARAM,
	VAR_GLOBAL,
	VAR_FILE_SCOPE,
	VAR_EXTERN
};

class Variable: public RefCountObject
{
	VariableClass m_class;
	size_t m_paramIndex;
	Ref<Type> m_type;
	std::string m_name;
	Location m_location;

public:
	Variable(VariableClass cls, Type* type, const std::string& name);
	Variable(size_t paramIndex, Type* type, const std::string& name);

	VariableClass GetClass() const { return m_class; }
	bool IsParameter() const { return m_class == VAR_PARAM; }
	bool IsTempVariable() const { return m_class == VAR_TEMP; }
	bool IsLocalScope() const { return m_class == VAR_FILE_SCOPE; }
	bool IsGlobal() const { return (m_class == VAR_GLOBAL) || (m_class == VAR_FILE_SCOPE) || (m_class == VAR_EXTERN); }
	bool IsExternal() const { return m_class == VAR_EXTERN; }
	size_t GetParameterIndex() const { return m_paramIndex; }
	Type* GetType() const { return m_type; }
	const std::string& GetName() const { return m_name; }

	const Location& GetLocation() const { return m_location; }
	void SetLocation(const Location& loc) { m_location = loc; }
};


#endif

