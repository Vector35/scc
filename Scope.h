// Copyright (c) 2011-2012 Rusty Wagner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

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

	Scope* Duplicate(DuplicateContext& dup);

	Scope* GetParent() const { return m_parent; }
	Scope* GetRoot() const { return m_root; }
	const std::vector< Ref<Variable> >& GetVariables() { return m_vars; }

	bool IsVariableDefined(const std::string& name) const;
	bool IsVariableDefinedInCurrentScope(const std::string& name) const;
	Variable* GetVariable(const std::string& name) const;
	Variable* DefineVariable(VariableClass cls, Type* type, const std::string& name);
	void DefineVariable(Variable* var);

	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
};


#endif

