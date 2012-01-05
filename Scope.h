#ifndef __SCOPE_H__
#define __SCOPE_H__

#include <vector>
#include <string>
#include <map>
#include "Type.h"
#include "Variable.h"


class Scope
{
	Scope* m_root;
	Scope* m_parent;
	std::vector< Ref<Variable> > m_vars;
	std::map< std::string, Ref<Variable> > m_currentScopeVars;

public:
	Scope(Scope* parent, bool newContext);

	Scope* GetParent() const { return m_parent; }
	Scope* GetRoot() const { return m_root; }
	const std::vector< Ref<Variable> >& GetVariables() { return m_vars; }

	bool IsVariableDefined(const std::string& name) const;
	bool IsVariableDefinedInCurrentScope(const std::string& name) const;
	Variable* GetVariable(const std::string& name) const;
	Variable* DefineVariable(VariableClass cls, Type* type, const std::string& name);
	void DefineVariable(Variable* var);
};


#endif

