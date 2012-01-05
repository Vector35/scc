#include "Scope.h"
#include "Struct.h"

using namespace std;


Scope::Scope(Scope* parent, bool newContext)
{
	m_parent = parent;
	m_root = ((!parent) || newContext) ? this : parent->m_root;
}


bool Scope::IsVariableDefined(const string& name) const
{
	map< string, Ref<Variable> >::const_iterator i = m_currentScopeVars.find(name);
	if (i != m_currentScopeVars.end())
		return true;
	if (!m_parent)
		return false;
	return m_parent->IsVariableDefined(name);
}


bool Scope::IsVariableDefinedInCurrentScope(const string& name) const
{
	map< string, Ref<Variable> >::const_iterator i = m_currentScopeVars.find(name);
	return i != m_currentScopeVars.end();
}


Variable* Scope::GetVariable(const string& name) const
{
	map< string, Ref<Variable> >::const_iterator i = m_currentScopeVars.find(name);
	if (i != m_currentScopeVars.end())
		return i->second;
	if (!m_parent)
		return NULL;
	return m_parent->GetVariable(name);
}


Variable* Scope::DefineVariable(VariableClass cls, Type* type, const string& name)
{
	Variable* var = new Variable(cls, type, name);

	if (cls == VAR_FILE_SCOPE)
	{
		// Static variable, always add to global list (but still in current scope for namespace reasons)
		Scope* topmost = m_root;
		while (topmost->m_parent)
			topmost = topmost->m_parent;
		topmost->m_vars.push_back(var);
	}
	else
	{
		// Normal variable, add to current context
		m_root->m_vars.push_back(var);
	}

	m_currentScopeVars[name] = var;
	return var;
}


void Scope::DefineVariable(Variable* var)
{
	m_root->m_vars.push_back(var);
	m_currentScopeVars[var->GetName()] = var;
}

