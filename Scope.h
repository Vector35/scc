#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "Type.h"
#include "Variable.h"
#include <map>
#include <string>
#include <vector>


class Scope
{
	Scope* m_root;
	Scope* m_parent;
	std::vector<Ref<Variable>> m_vars;
	std::map<std::string, Ref<Variable>> m_currentScopeVars;

 public:
	Scope(Scope* parent, bool newContext);

	Scope* Duplicate(DuplicateContext& dup);

	Scope* GetParent() const { return m_parent; }
	Scope* GetRoot() const { return m_root; }
	const std::vector<Ref<Variable>>& GetVariables() { return m_vars; }

	bool IsVariableDefined(const std::string& name) const;
	bool IsVariableDefinedInCurrentScope(const std::string& name) const;
	Variable* GetVariable(const std::string& name) const;
	Variable* DefineVariable(VariableClass cls, Type* type, const std::string& name);
	void DefineVariable(Variable* var);

	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
};


#endif
