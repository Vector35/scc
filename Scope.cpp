#include "Scope.h"
#include "Output.h"
#include "Struct.h"

using namespace std;


Scope::Scope(Scope* parent, bool newContext)
{
	m_parent = parent;
	m_root = ((!parent) || newContext) ? this : parent->m_root;
}


Scope* Scope::Duplicate(DuplicateContext& dup)
{
	Scope* scope = new Scope(NULL, true);

	for (vector<Ref<Variable>>::iterator i = m_vars.begin(); i != m_vars.end(); i++)
		scope->m_vars.push_back((*i)->Duplicate(dup));

	for (map<string, Ref<Variable>>::iterator i = m_currentScopeVars.begin();
	     i != m_currentScopeVars.end(); i++)
		scope->m_currentScopeVars[i->first] = i->second->Duplicate(dup);

	return scope;
}


bool Scope::IsVariableDefined(const string& name) const
{
	map<string, Ref<Variable>>::const_iterator i = m_currentScopeVars.find(name);
	if (i != m_currentScopeVars.end())
		return true;
	if (!m_parent)
		return false;
	return m_parent->IsVariableDefined(name);
}


bool Scope::IsVariableDefinedInCurrentScope(const string& name) const
{
	map<string, Ref<Variable>>::const_iterator i = m_currentScopeVars.find(name);
	return i != m_currentScopeVars.end();
}


Variable* Scope::GetVariable(const string& name) const
{
	map<string, Ref<Variable>>::const_iterator i = m_currentScopeVars.find(name);
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


void Scope::Serialize(OutputBlock* output)
{
	output->WriteInteger(m_vars.size());
	for (vector<Ref<Variable>>::iterator i = m_vars.begin(); i != m_vars.end(); i++)
		(*i)->Serialize(output);

	output->WriteInteger(m_currentScopeVars.size());
	for (map<string, Ref<Variable>>::iterator i = m_currentScopeVars.begin();
	     i != m_currentScopeVars.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}
}


bool Scope::Deserialize(InputBlock* input)
{
	size_t varCount;
	if (!input->ReadNativeInteger(varCount))
		return false;
	for (size_t i = 0; i < varCount; i++)
	{
		Variable* var = Variable::Deserialize(input);
		if (!var)
			return false;
		m_vars.push_back(var);
	}

	if (!input->ReadNativeInteger(varCount))
		return false;
	for (size_t i = 0; i < varCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Variable* var = Variable::Deserialize(input);
		if (!var)
			return false;

		m_currentScopeVars[name] = var;
	}

	return true;
}
