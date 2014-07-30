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

#include <stdio.h>
#include "ParserState.h"
#include "Output.h"

using namespace std;


ParserState::ParserState(const Settings& settings, const string& name, void* scanner):
	m_fileName(name), m_scanner(scanner), m_settings(settings)
{
	m_initExpression = BasicExpr(EXPR_SEQUENCE);
	m_globalScope = new Scope(NULL, true);
	m_currentScope = m_globalScope;
	m_errors = 0;
	m_line = 1;
}


ParserState::ParserState(ParserState* parent, const std::string& name, void* scanner):
	m_fileName(name), m_scanner(scanner), m_settings(parent->m_settings)
{
	DuplicateContext dup;
	for (map< string, Ref<Type> >::iterator i = parent->m_types.begin(); i != parent->m_types.end(); i++)
		m_types[i->first] = i->second->Duplicate(dup);
	for (map< string, Ref<Type> >::iterator i = parent->m_structTypes.begin(); i != parent->m_structTypes.end(); i++)
		m_structTypes[i->first] = i->second->Duplicate(dup);
	for (map< string, Ref<Type> >::iterator i = parent->m_unionTypes.begin(); i != parent->m_unionTypes.end(); i++)
		m_unionTypes[i->first] = i->second->Duplicate(dup);
	for (map< string, Ref<Type> >::iterator i = parent->m_enumTypes.begin(); i != parent->m_enumTypes.end(); i++)
		m_enumTypes[i->first] = i->second->Duplicate(dup);
	for (map< string, Ref<Function> >::iterator i = parent->m_functions.begin(); i != parent->m_functions.end(); i++)
		m_functions[i->first] = i->second->Duplicate(dup);
	m_initExpression = parent->m_initExpression->Duplicate(dup);
	m_globalScope = parent->m_globalScope->Duplicate(dup);
	m_enumMembers = parent->m_enumMembers;
	m_currentScope = m_globalScope;
	m_errors = 0;
	m_line = 1;
}


ParserState::~ParserState()
{
}


Location ParserState::GetLocation()
{
	Location loc;
	loc.fileName = GetFileName();
	loc.lineNumber = GetLineNumber();
	return loc;
}


string ParserState::ProcessEscapedString(const string& str)
{
	string result;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != '\\')
		{
			result += string(1, str[i]);
			continue;
		}

		i++;
		if (i >= str.length())
			break;

		if (str[i] == 'x')
		{
			i++;
			if (i >= str.length())
			{
				result += string(1, 'x');
				break;
			}

			char ch = 0;
			if ((str[i] >= '0') && (str[i] <= '9'))
				ch = str[i] - '0';
			else if ((str[i] >= 'A') && (str[i] <= 'F'))
				ch = str[i] - 'A' + 10;
			else if ((str[i] >= 'a') && (str[i] <= 'f'))
				ch = str[i] - 'a' + 10;
			else
			{
				result += string(1, 'x');
				result += string(1, str[i]);
				continue;
			}

			i++;
			if (i >= str.length())
			{
				result += string(1, 'x');
				result += string(1, str[i - 1]);
				break;
			}

			ch <<= 4;
			if ((str[i] >= '0') && (str[i] <= '9'))
				ch |= str[i] - '0';
			else if ((str[i] >= 'A') && (str[i] <= 'F'))
				ch |= str[i] - 'A' + 10;
			else if ((str[i] >= 'a') && (str[i] <= 'f'))
				ch |= str[i] - 'a' + 10;
			else
			{
				result += string(1, 'x');
				result += string(1, str[i - 1]);
				result += string(1, str[i]);
				continue;
			}

			result += string(1, ch);
			continue;
		}

		if (((i + 2) < str.length()) && (str[i] >= '0') && (str[i] <= '7') && (str[i + 1] >= '0') && (str[i + 1] <= '7') &&
			(str[i + 2] >= '0') && (str[i + 2] <= '7'))
		{
			char ch = (char)(str[i] - '0') << 6;
			ch |= (char)(str[i + 1] - '0') << 3;
			ch |= (char)(str[i + 2] - '0');
			result += string(1, ch);
			i += 2;
			continue;
		}

		switch (str[i])
		{
		case 'r':
			result += string(1, '\r');
			break;
		case 'n':
			result += string(1, '\n');
			break;
		case 't':
			result += string(1, '\t');
			break;
		default:
			result += string(1, str[i]);
			break;
		}
	}

	return result;
}


int64_t ParserState::CharStringToValue(const string& str)
{
	int64_t value = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		value <<= 8;
		value |= (int64_t)((uint8_t)str[i]);
	}
	return value;
}


void ParserState::DefineType(const string& name, Type* type)
{
	if (IsTypeDefined(name))
	{
		Error();
		fprintf(stderr, "%s:%d: error: type '%s' already defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
	}

	if (type->GetClass() == TYPE_STRUCT)
		type->GetStruct()->SetName(name);
	else if (type->GetClass() == TYPE_ENUM)
		type->GetEnum()->SetName(name);

	m_types[name] = type;
}


Type* ParserState::GetType(const string& name)
{
	map< string, Ref<Type> >::const_iterator i = m_types.find(name);
	if (i == m_types.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: type '%s' is not defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return Type::VoidType();
	}
	return i->second;
}


void ParserState::DefineStructType(const string& name, Type* type)
{
	type->GetStruct()->SetName(name);

	if (m_structTypes.find(name) != m_structTypes.end())
	{
		if (!type->GetStruct()->IsFullyDefined())
		{
			// This definition is just a forward declaration, ignore it
			type->SetStruct(m_structTypes[name]->GetStruct());
			return;
		}

		if (m_structTypes[name]->GetStruct()->IsFullyDefined())
		{
			// Redefining a completed structure
			Error();
			fprintf(stderr, "%s:%d: error: structure '%s' already defined\n", GetFileName().c_str(),
				GetLineNumber(), name.c_str());
		}

		// Update structure contents, ensuring that any references to the old object
		// will get the updated contents
		m_structTypes[name]->GetStruct()->ReplaceWith(type->GetStruct());
		type->SetStruct(m_structTypes[name]->GetStruct());
		return;
	}

	m_structTypes[name] = type;
}


void ParserState::DefineUnionType(const string& name, Type* type)
{
	type->GetStruct()->SetName(name);

	if (m_unionTypes.find(name) != m_unionTypes.end())
	{
		if (!type->GetStruct()->IsFullyDefined())
		{
			// This definition is just a forward declaration, ignore it
			type->SetStruct(m_unionTypes[name]->GetStruct());
			return;
		}

		if (m_unionTypes[name]->GetStruct()->IsFullyDefined())
		{
			// Redefining a completed union
			Error();
			fprintf(stderr, "%s:%d: error: union '%s' already defined\n", GetFileName().c_str(),
				GetLineNumber(), name.c_str());
		}

		// Update union contents, ensuring that any references to the old object
		// will get the updated contents
		m_unionTypes[name]->GetStruct()->ReplaceWith(type->GetStruct());
		type->SetStruct(m_unionTypes[name]->GetStruct());
		return;
	}

	m_unionTypes[name] = type;
}


void ParserState::DefineEnumType(const string& name, Type* type)
{
	type->GetEnum()->SetName(name);

	if (m_enumTypes.find(name) != m_enumTypes.end())
	{
		if (!type->GetEnum()->IsFullyDefined())
		{
			// This definition is just a forward declaration, ignore it
			type->SetEnum(m_enumTypes[name]->GetEnum());
			return;
		}

		if (m_enumTypes[name]->GetEnum()->IsFullyDefined())
		{
			// Redefining a completed enumeration
			Error();
			fprintf(stderr, "%s:%d: error: enumeration '%s' already defined\n", GetFileName().c_str(),
				GetLineNumber(), name.c_str());
		}

		// Update enumeration contents, ensuring that any references to the old object
		// will get the updated contents
		m_enumTypes[name]->GetEnum()->ReplaceWith(type->GetEnum());
		type->SetEnum(m_enumTypes[name]->GetEnum());
		return;
	}

	m_enumTypes[name] = type;
}


Type* ParserState::GetStructType(const string& name)
{
	map< string, Ref<Type> >::const_iterator i = m_structTypes.find(name);
	if (i == m_structTypes.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: structure '%s' is not defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return Type::VoidType();
	}
	return i->second;
}


Type* ParserState::GetUnionType(const string& name)
{
	map< string, Ref<Type> >::const_iterator i = m_unionTypes.find(name);
	if (i == m_unionTypes.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: union '%s' is not defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return Type::VoidType();
	}
	return i->second;
}


Type* ParserState::GetEnumType(const string& name)
{
	map< string, Ref<Type> >::const_iterator i = m_enumTypes.find(name);
	if (i == m_enumTypes.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: enumeration '%s' is not defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return Type::VoidType();
	}
	return i->second;
}


void ParserState::AddEnumMember(const string& name, uint32_t value)
{
	if (m_enumMembers.find(name) != m_enumMembers.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: identifier '%s' already defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
	}

	m_enumMembers[name] = value;
}


uint32_t ParserState::GetEnumMemberValue(const string& name)
{
	map<string, uint32_t>::const_iterator i = m_enumMembers.find(name);
	if (i == m_enumMembers.end())
	{
		Error();
		fprintf(stderr, "%s:%d: error: identifier '%s' not defined\n", GetFileName().c_str(),
			GetLineNumber(), name.c_str());
		return 0;
	}
	return i->second;
}


void ParserState::BeginFunctionScope(const FunctionInfo& func)
{
	m_currentScope = new Scope(m_currentScope, true);

	for (size_t i = 0; i < func.params.size(); i++)
	{
		if (func.params[i].second.size() == 0)
			continue;
		if (func.params[i].second == "...")
			continue;

		m_currentScope->DefineVariable(new Variable(i, func.params[i].first, func.params[i].second));
	}
}


void ParserState::PushScope()
{
	m_currentScope = new Scope(m_currentScope, false);
}


void ParserState::PopScope()
{
	m_currentScope = m_currentScope->GetParent();
}


Expr* ParserState::DeclareVariable(Type* type, const VarInitInfo& info, bool isStatic, bool isExtern)
{
	if (info.array)
		type = Type::ArrayType(type, info.elements);

	if (m_currentScope->IsVariableDefinedInCurrentScope(info.name))
	{
		if (!m_currentScope->GetVariable(info.name)->IsExternal())
		{
			Error();
			fprintf(stderr, "%s:%d: error: variable '%s' already defined\n",
				info.location.fileName.c_str(), info.location.lineNumber, info.name.c_str());
		}
	}

	if ((m_currentScope == m_globalScope) && (m_functions.find(info.name) != m_functions.end()))
	{
		Error();
		fprintf(stderr, "%s:%d: error: identifier '%s' already defined\n",
			info.location.fileName.c_str(), info.location.lineNumber, info.name.c_str());
	}

	Variable* var;
	if (isExtern)
		var = m_currentScope->DefineVariable(VAR_EXTERN, type, info.name);
	else if ((m_currentScope == m_globalScope) && isStatic)
		var = m_currentScope->DefineVariable(VAR_FILE_SCOPE, type, info.name);
	else if ((m_currentScope == m_globalScope) && (!isStatic))
		var = m_currentScope->DefineVariable(VAR_GLOBAL, type, info.name);
	else if (isStatic)
		var = m_currentScope->DefineVariable(VAR_FILE_SCOPE, type, info.name);
	else
		var = m_currentScope->DefineVariable(VAR_LOCAL, type, info.name);
	var->SetLocation(info.location);

	if (!info.initialized)
		return new Expr(info.location, EXPR_SEQUENCE);
	return Expr::BinaryExpr(info.location, EXPR_INIT_ASSIGN, Expr::VariableExpr(info.location, var), info.value);
}


void ParserState::DefineFunction(FunctionInfo& func, Expr* body, bool isLocalScope)
{
	map< string, Ref<Function> >::iterator i = m_functions.find(func.name);
	if (i != m_functions.end())
	{
		if (!i->second->IsCompatible(func))
		{
			Error();
			fprintf(stderr, "%s:%d: error: function definition for '%s' not compatible with existing declaration\n",
				func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
			return;
		}

		if (i->second->IsFullyDefined())
		{
			Error();
			fprintf(stderr, "%s:%d: error: function '%s' already defined\n",
				func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
		}

		if (isLocalScope != i->second->IsLocalScope())
		{
			Error();
			fprintf(stderr, "%s:%d: error: function definition for '%s' not compatible with existing declaration\n",
				func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
			return;
		}

		i->second->SetVariables(m_currentScope->GetRoot()->GetVariables());
		i->second->SetBody(body);
		i->second->SetLocation(func.location);
		return;
	}

	if (m_globalScope->IsVariableDefined(func.name))
	{
		Error();
		fprintf(stderr, "%s:%d: error: identifier '%s' already defined\n",
			func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
	}

	m_functions[func.name] = new Function(func, m_currentScope->GetRoot()->GetVariables(), body, isLocalScope);
}


void ParserState::DefineFunctionPrototype(FunctionInfo& func, bool isLocalScope)
{
	map< string, Ref<Function> >::iterator i = m_functions.find(func.name);
	if (i != m_functions.end())
	{
		if (!i->second->IsCompatible(func))
		{
			Error();
			fprintf(stderr, "%s:%d: error: prototype for '%s' not compatible with existing declaration\n",
				func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
		}

		if (isLocalScope != i->second->IsLocalScope())
		{
			Error();
			fprintf(stderr, "%s:%d: error: prototype for '%s' not compatible with existing declaration\n",
				func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
			return;
		}
		return;
	}

	if (m_globalScope->IsVariableDefined(func.name))
	{
		Error();
		fprintf(stderr, "%s:%d: error: identifier '%s' already defined\n",
			func.location.fileName.c_str(), func.location.lineNumber, func.name.c_str());
	}

	m_functions[func.name] = new Function(func, isLocalScope);
}


Expr* ParserState::ResolveIdentifierExpr(const string& name)
{
	if (m_currentScope->IsVariableDefined(name))
		return VariableExpr(m_currentScope->GetVariable(name));

	map< string, Ref<Function> >::iterator i = m_functions.find(name);
	if (i != m_functions.end())
		return FunctionExpr(i->second);

	Error();
	fprintf(stderr, "%s:%d: error: identifier '%s' not defined\n", GetFileName().c_str(), GetLineNumber(), name.c_str());
	return IntExpr(0);
}


void ParserState::AddInitExpression(Expr* expr)
{
	if (expr->GetClass() == EXPR_SEQUENCE)
		m_initExpression->CopyChildren(expr);
	else
		m_initExpression->AddChild(expr);
}


bool ParserState::HasIntrinsicStrlen()
{
	return m_settings.architecture == ARCH_X86;
}


bool ParserState::HasIntrinsicMemcpy()
{
	return m_settings.architecture == ARCH_X86;
}


bool ParserState::HasIntrinsicMemset()
{
	return m_settings.architecture == ARCH_X86;
}


bool ParserState::HasIntrinsicDivide64()
{
	return m_settings.preferredBits == 64;
}


bool ParserState::HasIntrinsicShift64()
{
	return m_settings.architecture == ARCH_X86;
}


bool ParserState::HasIntrinsicPow()
{
	return true;
}


bool ParserState::HasIntrinsicByteSwap()
{
	if (m_settings.architecture == ARCH_MIPS)
		return false;
	return true;
}


bool ParserState::HasIntrinsicDivide()
{
	if (m_settings.architecture == ARCH_ARM)
		return false;
	return true;
}


bool ParserState::IsValidFloatImmediate(double value)
{
	if (m_settings.architecture == ARCH_QUARK)
	{
		if (((double)(int32_t)value) == value)
		{
			int32_t i = (int32_t)value;
			if (i < -1024)
				return false;
			if (i > 1023)
				return false;
			return true;
		}
	}

	return false;
}


Variable* ParserState::GetFloatImmediateVariable(Type* type, double value)
{
	if (type->GetWidth() == 4)
	{
		map<float, Variable*>::iterator i = m_floatImmed.find((float)value);
		if (i != m_floatImmed.end())
			return i->second;

		char name[32];
		sprintf(name, "@%ff", value);
		Variable* var = m_globalScope->DefineVariable(VAR_FILE_SCOPE, type, name);
		var->GetData().WriteFloat((float)value);
		return var;
	}
	else
	{
		map<double, Variable*>::iterator i = m_doubleImmed.find((double)value);
		if (i != m_doubleImmed.end())
			return i->second;

		char name[32];
		sprintf(name, "@%f", value);
		Variable* var = m_globalScope->DefineVariable(VAR_FILE_SCOPE, type, name);
		var->GetData().WriteDouble(value);
		return var;
	}
}


void ParserState::Serialize(OutputBlock* output)
{
	output->WriteInteger(m_types.size());
	for (map< string, Ref<Type> >::iterator i = m_types.begin(); i != m_types.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	output->WriteInteger(m_structTypes.size());
	for (map< string, Ref<Type> >::iterator i = m_structTypes.begin(); i != m_structTypes.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	output->WriteInteger(m_unionTypes.size());
	for (map< string, Ref<Type> >::iterator i = m_unionTypes.begin(); i != m_unionTypes.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	output->WriteInteger(m_enumTypes.size());
	for (map< string, Ref<Type> >::iterator i = m_enumTypes.begin(); i != m_enumTypes.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	output->WriteInteger(m_enumMembers.size());
	for (map<string, uint32_t>::iterator i = m_enumMembers.begin(); i != m_enumMembers.end(); i++)
	{
		output->WriteString(i->first);
		output->WriteInteger(i->second);
	}

	output->WriteInteger(m_functions.size());
	for (map< string, Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
	{
		output->WriteString(i->first);
		i->second->Serialize(output);
	}

	m_initExpression->Serialize(output);
	m_globalScope->Serialize(output);
}


bool ParserState::Deserialize(InputBlock* input)
{
	size_t typeCount;
	if (!input->ReadNativeInteger(typeCount))
		return false;
	for (size_t i = 0; i < typeCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Type* type = Type::Deserialize(input);
		if (!type)
			return false;

		m_types[name] = type;
	}

	if (!input->ReadNativeInteger(typeCount))
		return false;
	for (size_t i = 0; i < typeCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Type* type = Type::Deserialize(input);
		if (!type)
			return false;

		m_structTypes[name] = type;
	}

	if (!input->ReadNativeInteger(typeCount))
		return false;
	for (size_t i = 0; i < typeCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Type* type = Type::Deserialize(input);
		if (!type)
			return false;

		m_unionTypes[name] = type;
	}

	if (!input->ReadNativeInteger(typeCount))
		return false;
	for (size_t i = 0; i < typeCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Type* type = Type::Deserialize(input);
		if (!type)
			return false;

		m_enumTypes[name] = type;
	}

	size_t memberCount;
	if (!input->ReadNativeInteger(memberCount))
		return false;
	for (size_t i = 0; i < memberCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		uint32_t value;
		if (!input->ReadUInt32(value))
			return false;

		m_enumMembers[name] = value;
	}

	size_t functionCount;
	if (!input->ReadNativeInteger(functionCount))
		return false;
	for (size_t i = 0; i < functionCount; i++)
	{
		string name;
		if (!input->ReadString(name))
			return false;

		Function* func = Function::Deserialize(input);
		if (!func)
			return false;

		m_functions[name] = func;
	}

	m_initExpression = Expr::Deserialize(input);
	if (!m_initExpression)
		return false;

	if (!m_globalScope->Deserialize(input))
		return false;

	return true;
}


#ifndef WIN32
void ParserState::Print()
{
	fprintf(stderr, "Global variables:\n");
	for (vector< Ref<Variable> >::const_iterator i = m_globalScope->GetVariables().begin();
		i != m_globalScope->GetVariables().end(); i++)
	{
		(*i)->GetType()->Print();
		fprintf(stderr, " %s\n", (*i)->GetName().c_str());
	}

	fprintf(stderr, "\nInitializers:\n");
	m_initExpression->Print(0);

	fprintf(stderr, "\n\nFunctions:\n");
	for (map< string, Ref<Function> >::iterator i = m_functions.begin(); i != m_functions.end(); i++)
		i->second->Print();
}
#endif

