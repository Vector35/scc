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
#include <stdlib.h>
#include "ILBlock.h"
#include "Variable.h"
#include "Function.h"
#include "Struct.h"
#include "Output.h"

using namespace std;


map<size_t, ILBlock*> ILBlock::m_serializationMapping;
stack< map<size_t, ILBlock*> > ILBlock::m_savedSerializationMappings;

	
ILParameter::ILParameter()
{
	cls = ILPARAM_VOID;
	integerValue = 0;
	type = ILTYPE_VOID;
}


ILParameter::ILParameter(bool b)
{
	cls = ILPARAM_BOOL;
	boolValue = b;
	type = ILTYPE_INT8;
}


ILParameter::ILParameter(ILParameterType t, int64_t i)
{
	cls = ILPARAM_INT;
	integerValue = i;
	type = t;
}


ILParameter::ILParameter(Type* t, int64_t i)
{
	cls = ILPARAM_INT;
	integerValue = i;
	type = ReduceType(t);
}


ILParameter::ILParameter(ILParameterType t, double f)
{
	cls = ILPARAM_FLOAT;
	floatValue = f;
	type = t;
}


ILParameter::ILParameter(Type* t, double f)
{
	cls = ILPARAM_FLOAT;
	floatValue = f;
	type = ReduceType(t);
}


ILParameter::ILParameter(const string& str)
{
	cls = ILPARAM_STRING;
	stringValue = str;
	type = (GetTargetPointerSize() == 4) ? ILTYPE_INT32 : ILTYPE_INT64;
}


ILParameter::ILParameter(Struct* s, const string& name)
{
	if (!s)
		__asm__ __volatile__("int3");
	cls = ILPARAM_FIELD;
	structure = s;
	stringValue = name;
	type = ILTYPE_VOID;
}


ILParameter::ILParameter(Variable* var)
{
	cls = ILPARAM_VAR;
	variable = var;
	type = ReduceType(var->GetType());
}


ILParameter::ILParameter(Function* func)
{
	cls = ILPARAM_FUNC;
	function = func;
	type = (GetTargetPointerSize() == 4) ? ILTYPE_INT32 : ILTYPE_INT64;
}


ILParameter::ILParameter(ILBlock* b)
{
	cls = ILPARAM_BLOCK;
	block = b;
	type = (GetTargetPointerSize() == 4) ? ILTYPE_INT32 : ILTYPE_INT64;
}


ILParameter::ILParameter(const ILParameter& obj, Type* objType, const string& str)
{
	cls = ILPARAM_MEMBER;
	parent = new ILParameter(obj);
	structure = objType->GetStruct();
	stringValue = str;
}


ILParameter::ILParameter(const ILParameter& param)
{
	cls = param.cls;
	type = param.type;
	stringValue = param.stringValue;
	structure = param.structure;
	variable = param.variable;
	function = param.function;

	if (cls == ILPARAM_MEMBER)
		parent = new ILParameter(*param.parent);
	else if (cls == ILPARAM_BLOCK)
		block = param.block;
	else
		integerValue = param.integerValue;
}


ILParameter::~ILParameter()
{
	if (cls == ILPARAM_MEMBER)
		delete parent;
}


ILParameter& ILParameter::operator=(const ILParameter& param)
{
	if (cls == ILPARAM_MEMBER)
		delete parent;

	cls = param.cls;
	type = param.type;
	stringValue = param.stringValue;
	structure = param.structure;
	variable = param.variable;
	function = param.function;

	if (cls == ILPARAM_MEMBER)
		parent = new ILParameter(*param.parent);
	else if (cls == ILPARAM_BLOCK)
		block = param.block;
	else
		integerValue = param.integerValue;

	return *this;
}


ILParameterType ILParameter::ReduceType(Type* t)
{
	switch (t->GetClass())
	{
	case TYPE_BOOL:
		return ILTYPE_INT8;
	case TYPE_INT:
		switch (t->GetWidth())
		{
		case 1:
			return ILTYPE_INT8;
		case 2:
			return ILTYPE_INT16;
		case 4:
			return ILTYPE_INT32;
		case 8:
			return ILTYPE_INT64;
		default:
			return ILTYPE_VOID;
		}
	case TYPE_FLOAT:
		return (t->GetWidth() == 4) ? ILTYPE_FLOAT : ILTYPE_DOUBLE;
	case TYPE_POINTER:
	case TYPE_ARRAY:
	case TYPE_FUNCTION:
		return (GetTargetPointerSize() == 4) ? ILTYPE_INT32 : ILTYPE_INT64;
	default:
		return ILTYPE_VOID;
	}
}


size_t ILParameter::GetWidth() const
{
	switch (type)
	{
	case ILTYPE_INT8:
		return 1;
	case ILTYPE_INT16:
		return 2;
	case ILTYPE_INT32:
	case ILTYPE_FLOAT:
		return 4;
	case ILTYPE_INT64:
	case ILTYPE_DOUBLE:
		return 8;
	default:
		return 0;
	}
}


void ILParameter::ReplaceFunction(Function* from, Function* to)
{
	if (cls == ILPARAM_MEMBER)
		parent->ReplaceFunction(from, to);
	if (function == from)
		function = to;
}


void ILParameter::ReplaceVariable(Variable* from, Variable* to)
{
	if (cls == ILPARAM_MEMBER)
		parent->ReplaceVariable(from, to);
	if (variable == from)
		variable = to;
}


void ILParameter::CheckForUndefinedReferences(size_t& errors)
{
	if (cls == ILPARAM_MEMBER)
		parent->CheckForUndefinedReferences(errors);

	// All prototypes should have been resolved by the linker
	if (function && (!function->IsFullyDefined()))
	{
		// No location information here, but there error should have been already generated in the
		// parse tree, so we shouldn't get here unless something else is wrong.  Print the error
		// anyway to avoid missing errors
		errors++;
		fprintf(stderr, "error: undefined reference to '%s'\n", function->GetName().c_str());
	}

	// All variables marked 'extern' should have been resolved by the linker
	if (variable && variable->IsExternal())
	{
		// No location information here, but there error should have been already generated in the
		// parse tree, so we shouldn't get here unless something else is wrong.  Print the error
		// anyway to avoid missing errors
		errors++;
		fprintf(stderr, "error: undefined reference to '%s'\n", variable->GetName().c_str());
	}
}


void ILParameter::ConvertStringsToVariables(map< string, Ref<Variable> >& stringMap)
{
	if (cls != ILPARAM_STRING)
		return;

	map< string, Ref<Variable> >::iterator i = stringMap.find(stringValue);
	if (i != stringMap.end())
	{
		cls = ILPARAM_VAR;
		variable = i->second;
		return;
	}

	Type* t = Type::ArrayType(Type::IntType(1, true), stringValue.size() + 1);
	t->SetConst(true);
	Variable* var = new Variable(VAR_GLOBAL, t, string("$str$" + stringValue));
	var->GetData().Write(stringValue.c_str(), stringValue.size() + 1);
	stringMap[stringValue] = var;

	cls = ILPARAM_VAR;
	variable = var;
}


bool ILParameter::IsConstant() const
{
	switch (cls)
	{
	case ILPARAM_INT:
	case ILPARAM_FLOAT:
	case ILPARAM_STRING:
	case ILPARAM_BOOL:
		return true;
	default:
		return false;
	}
}


void ILParameter::TagReferences()
{
	switch (cls)
	{
	case ILPARAM_FUNC:
		function->TagReferences();
		break;
	case ILPARAM_VAR:
		variable->TagReference();
		break;
	default:
		break;
	}
}


void ILParameter::Serialize(OutputBlock* output)
{
	output->WriteInteger(cls);
	output->WriteInteger(type);

	switch (cls)
	{
	case ILPARAM_INT:
		output->WriteInteger(integerValue);
		break;
	case ILPARAM_FLOAT:
		output->Write(&floatValue, sizeof(floatValue));
		break;
	case ILPARAM_STRING:
		output->WriteString(stringValue);
		break;
	case ILPARAM_FIELD:
		structure->Serialize(output);
		output->WriteString(stringValue);
		break;
	case ILPARAM_BOOL:
		output->WriteInteger(boolValue ? 1 : 0);
		break;
	case ILPARAM_VAR:
		variable->Serialize(output);
		break;
	case ILPARAM_FUNC:
		function->Serialize(output);
		break;
	case ILPARAM_BLOCK:
		output->WriteInteger(block->GetIndex());
		break;
	case ILPARAM_MEMBER:
		parent->Serialize(output);
		structure->Serialize(output);
		output->WriteString(stringValue);
		break;
	default:
		break;
	}
}


bool ILParameter::Deserialize(InputBlock* input)
{
	uint32_t clsInt;
	if (!input->ReadUInt32(clsInt))
		return false;
	cls = (ILParameterClass)clsInt;

	uint32_t typeInt;
	if (!input->ReadUInt32(typeInt))
		return false;
	type = (ILParameterType)typeInt;

	int64_t objectIndex;
	switch (cls)
	{
	case ILPARAM_INT:
		if (!input->ReadInt64(integerValue))
			return false;
		break;
	case ILPARAM_FLOAT:
		if (!input->Read(&floatValue, sizeof(floatValue)))
			return false;
		break;
	case ILPARAM_STRING:
		if (!input->ReadString(stringValue))
			return false;
		break;
	case ILPARAM_FIELD:
		structure = Struct::Deserialize(input);
		if (!structure)
			return false;
		if (!input->ReadString(stringValue))
			return false;
		break;
	case ILPARAM_BOOL:
		if (!input->ReadBool(boolValue))
			return false;
		break;
	case ILPARAM_VAR:
		variable = Variable::Deserialize(input);
		if (!variable)
			return false;
		break;
	case ILPARAM_FUNC:
		function = Function::Deserialize(input);
		if (!function)
			return false;
		break;
	case ILPARAM_BLOCK:
		if (!input->ReadInt64(objectIndex))
			return false;
		block = ILBlock::GetSerializationMapping((size_t)objectIndex);
		if (!block)
			return false;
		break;
	case ILPARAM_MEMBER:
		parent = new ILParameter();
		if (!parent->Deserialize(input))
			return false;
		structure = Struct::Deserialize(input);
		if (!structure)
			return false;
		if (!input->ReadString(stringValue))
			return false;
		break;
	default:
		break;
	}

	return true;
}


void ILParameter::Print() const
{
	switch (cls)
	{
	case ILPARAM_VOID:  fprintf(stderr, "void"); break;
#ifdef WIN32
	case ILPARAM_INT:  fprintf(stderr, "%.I64d {%d}", (long long)integerValue, (int)GetWidth()); break;
#else
	case ILPARAM_INT:  fprintf(stderr, "%lld {%d}", (long long)integerValue, (int)GetWidth()); break;
#endif
	case ILPARAM_FLOAT:  fprintf(stderr, "%f", floatValue); break;
	case ILPARAM_STRING:  fprintf(stderr, "\"%s\"", stringValue.c_str()); break;
	case ILPARAM_FIELD:  fprintf(stderr, "%s::%s", structure->GetName().c_str(), stringValue.c_str()); break;
	case ILPARAM_BOOL:  fprintf(stderr, "%s", boolValue ? "true" : "false"); break;
	case ILPARAM_VAR:  fprintf(stderr, "var<%s>", variable->GetName().c_str()); break;
	case ILPARAM_FUNC:  fprintf(stderr, "func<%s>", function->GetName().c_str()); break;
	case ILPARAM_BLOCK:  fprintf(stderr, "block<%d>", (int)block->GetIndex()); break;
	case ILPARAM_MEMBER:  parent->Print(); fprintf(stderr, ".%s::%s", structure->GetName().c_str(), stringValue.c_str()); break;
	case ILPARAM_UNDEFINED:  fprintf(stderr, "__undefined"); break;
	default:  fprintf(stderr, "<invalid>"); break;
	}
}


ILInstruction::ILInstruction()
{
	operation = ILOP_RETURN_VOID;
}


ILInstruction::ILInstruction(ILOperation op)
{
	operation = op;
}


ILInstruction::ILInstruction(ILOperation op, const ILParameter& a)
{
	operation = op;
	params.push_back(a);
}


ILInstruction::ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b)
{
	operation = op;
	params.push_back(a);
	params.push_back(b);
}


ILInstruction::ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c)
{
	operation = op;
	params.push_back(a);
	params.push_back(b);
	params.push_back(c);
}


ILInstruction::ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c, const ILParameter& d)
{
	operation = op;
	params.push_back(a);
	params.push_back(b);
	params.push_back(c);
	params.push_back(d);
}


ILInstruction::ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c,
	const ILParameter& d, const ILParameter& e)
{
	operation = op;
	params.push_back(a);
	params.push_back(b);
	params.push_back(c);
	params.push_back(d);
	params.push_back(e);
}


ILInstruction::ILInstruction(ILOperation op, const vector<ILParameter>& list)
{
	operation = op;
	params = list;
}


ILInstruction::ILInstruction(const ILInstruction& instr)
{
	operation = instr.operation;
	params = instr.params;
}


ILInstruction& ILInstruction::operator=(const ILInstruction& instr)
{
	operation = instr.operation;
	params = instr.params;
	return *this;
}


void ILInstruction::ReplaceFunction(Function* from, Function* to)
{
	for (vector<ILParameter>::iterator i = params.begin(); i != params.end(); i++)
		i->ReplaceFunction(from, to);
}


void ILInstruction::ReplaceVariable(Variable* from, Variable* to)
{
	for (vector<ILParameter>::iterator i = params.begin(); i != params.end(); i++)
		i->ReplaceVariable(from, to);
}


void ILInstruction::CheckForUndefinedReferences(size_t& errors)
{
	for (vector<ILParameter>::iterator i = params.begin(); i != params.end(); i++)
		i->CheckForUndefinedReferences(errors);
}


void ILInstruction::ConvertStringsToVariables(map< string, Ref<Variable> >& stringMap)
{
	for (vector<ILParameter>::iterator i = params.begin(); i != params.end(); i++)
		i->ConvertStringsToVariables(stringMap);
}


void ILInstruction::TagReferences()
{
	for (vector<ILParameter>::iterator i = params.begin(); i != params.end(); i++)
		i->TagReferences();
}


bool ILInstruction::WritesToFirstParameter() const
{
	switch (operation)
	{
	case ILOP_ASSIGN:
	case ILOP_ADDRESS_OF:
	case ILOP_ADDRESS_OF_MEMBER:
	case ILOP_DEREF:
	case ILOP_DEREF_MEMBER:
	case ILOP_ARRAY_INDEX:
	case ILOP_ARRAY_INDEX_ASSIGN:
	case ILOP_PTR_ADD:
	case ILOP_PTR_SUB:
	case ILOP_PTR_DIFF:
	case ILOP_ADD:
	case ILOP_SUB:
	case ILOP_SMULT:
	case ILOP_UMULT:
	case ILOP_SDIV:
	case ILOP_UDIV:
	case ILOP_SMOD:
	case ILOP_UMOD:
	case ILOP_AND:
	case ILOP_OR:
	case ILOP_XOR:
	case ILOP_SHL:
	case ILOP_SHR:
	case ILOP_SAR:
	case ILOP_NEG:
	case ILOP_NOT:
	case ILOP_CALL:
	case ILOP_SCONVERT:
	case ILOP_UCONVERT:
	case ILOP_ALLOCA:
	case ILOP_STRLEN:
	case ILOP_SYSCALL:
	case ILOP_SYSCALL2:
	case ILOP_RDTSC:
	case ILOP_RDTSC_LOW:
	case ILOP_RDTSC_HIGH:
	case ILOP_NEXT_ARG:
	case ILOP_PREV_ARG:
	case ILOP_BYTESWAP:
		// These instructions write to the first parameter
		return true;
	case ILOP_DEREF_ASSIGN:
	case ILOP_DEREF_MEMBER_ASSIGN:
	case ILOP_IF_TRUE:
	case ILOP_IF_LESS_THAN:
	case ILOP_IF_LESS_EQUAL:
	case ILOP_IF_BELOW:
	case ILOP_IF_BELOW_EQUAL:
	case ILOP_IF_EQUAL:
	case ILOP_GOTO:
	case ILOP_NORETURN:
	case ILOP_RETURN:
	case ILOP_RETURN_VOID:
	case ILOP_MEMCPY:
	case ILOP_MEMSET:
	case ILOP_BREAKPOINT:
		// These instructions do not write to any variables, default is avoided here
		// to make the compiler spit out warnings if instructions are added but they
		// aren't handled here (which could lead to well hidden bugs)
		break;
	}

	return false;
}


void ILInstruction::MarkWrittenVariables()
{
	if (operation == ILOP_ADDRESS_OF)
	{
		// This technically isn't a write to the source parameter itself, but this
		// creates an alias, which could lead to an indirect write.  Proper alias analysis
		// could resolve some of these and determine if there is actually a write.  Since
		// the primary use of this function is to do simple optimizations on the inlining
		// process, we can optimize any unnecessary aliased copies later.
		if (params[0].cls == ILPARAM_VAR)
			params[0].variable->SetWritten(true);
		if (params[1].cls == ILPARAM_VAR)
			params[1].variable->SetWritten(true);
		return;
	}

	if (WritesToFirstParameter())
	{
		if (params[0].cls == ILPARAM_VAR)
			params[0].variable->SetWritten(true);
	}
}


void ILInstruction::Serialize(OutputBlock* output)
{
	output->WriteInteger(operation);
	output->WriteInteger(params.size());
	for (vector<ILParameter>::iterator i = params.begin(); i != params.end(); i++)
		i->Serialize(output);
}


bool ILInstruction::Deserialize(InputBlock* input)
{
	uint32_t op;
	if (!input->ReadUInt32(op))
		return false;
	operation = (ILOperation)op;

	size_t paramCount;
	if (!input->ReadNativeInteger(paramCount))
		return false;
	for (size_t i = 0; i < paramCount; i++)
	{
		ILParameter param;
		if (!param.Deserialize(input))
			return false;
		params.push_back(param);
	}

	return true;
}


void ILInstruction::Print() const
{
	switch (operation)
	{
	case ILOP_ASSIGN:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); break;
	case ILOP_ADDRESS_OF:  params[0].Print(); fprintf(stderr, " = &"); params[1].Print(); break;
	case ILOP_ADDRESS_OF_MEMBER:  params[0].Print(); fprintf(stderr, " = &"); params[1].Print(); fprintf(stderr, "->"); params[2].Print(); break;
	case ILOP_DEREF:  params[0].Print(); fprintf(stderr, " = *"); params[1].Print(); break;
	case ILOP_DEREF_MEMBER:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, "->"); params[2].Print(); break;
	case ILOP_DEREF_ASSIGN:  fprintf(stderr, "*"); params[0].Print(); fprintf(stderr, " = "); params[1].Print(); break;
	case ILOP_DEREF_MEMBER_ASSIGN:  params[0].Print(); fprintf(stderr, "->"); params[1].Print(); fprintf(stderr, " = "); params[2].Print(); break;
	case ILOP_ARRAY_INDEX:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, "["); params[2].Print(); fprintf(stderr, "]{%d}", (int)params[3].integerValue); break;
	case ILOP_ARRAY_INDEX_ASSIGN:  params[0].Print(); fprintf(stderr, "["); params[1].Print(); fprintf(stderr, "]{%d} = ", (int)params[2].integerValue); params[3].Print(); break;
	case ILOP_PTR_ADD:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " ptr+ "); params[2].Print(); fprintf(stderr, " {%d}", (int)params[3].integerValue); break;
	case ILOP_PTR_SUB:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " ptr- "); params[2].Print(); fprintf(stderr, " {%d}", (int)params[3].integerValue); break;
	case ILOP_PTR_DIFF:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " ptrdiff "); params[2].Print(); fprintf(stderr, " {%d}", (int)params[3].integerValue); break;
	case ILOP_ADD:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " + "); params[2].Print(); break;
	case ILOP_SUB:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " - "); params[2].Print(); break;
	case ILOP_SMULT:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " smult "); params[2].Print(); break;
	case ILOP_UMULT:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " umult "); params[2].Print(); break;
	case ILOP_SDIV:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " sdiv "); params[2].Print(); break;
	case ILOP_UDIV:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " udiv "); params[2].Print(); break;
	case ILOP_SMOD:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " smod "); params[2].Print(); break;
	case ILOP_UMOD:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " umod "); params[2].Print(); break;
	case ILOP_AND:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " & "); params[2].Print(); break;
	case ILOP_OR:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " | "); params[2].Print(); break;
	case ILOP_XOR:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " ^ "); params[2].Print(); break;
	case ILOP_SHL:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " shl "); params[2].Print(); break;
	case ILOP_SHR:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " shr "); params[2].Print(); break;
	case ILOP_SAR:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " sar "); params[2].Print(); break;
	case ILOP_NEG:  params[0].Print(); fprintf(stderr, " = -"); params[1].Print(); break;
	case ILOP_NOT:  params[0].Print(); fprintf(stderr, " = ~"); params[1].Print(); break;
	case ILOP_IF_TRUE:  fprintf(stderr, "if "); params[0].Print(); fprintf(stderr, " then "); params[1].Print(); fprintf(stderr, " else "); params[2].Print(); break;
	case ILOP_IF_LESS_THAN:  fprintf(stderr, "if signed "); params[0].Print(); fprintf(stderr, " < "); params[1].Print(); fprintf(stderr, " then "); params[2].Print(); fprintf(stderr, " else "); params[3].Print(); break;
	case ILOP_IF_LESS_EQUAL:  fprintf(stderr, "if signed "); params[0].Print(); fprintf(stderr, " <= "); params[1].Print(); fprintf(stderr, " then "); params[2].Print(); fprintf(stderr, " else "); params[3].Print(); break;
	case ILOP_IF_BELOW:  fprintf(stderr, "if unsigned "); params[0].Print(); fprintf(stderr, " < "); params[1].Print(); fprintf(stderr, " then "); params[2].Print(); fprintf(stderr, " else "); params[3].Print(); break;
	case ILOP_IF_BELOW_EQUAL:  fprintf(stderr, "if unsigned "); params[0].Print(); fprintf(stderr, " <= "); params[1].Print(); fprintf(stderr, " then "); params[2].Print(); fprintf(stderr, " else "); params[3].Print(); break;
	case ILOP_IF_EQUAL:  fprintf(stderr, "if "); params[0].Print(); fprintf(stderr, " == "); params[1].Print(); fprintf(stderr, " then "); params[2].Print(); fprintf(stderr, " else "); params[3].Print(); break;
	case ILOP_GOTO:  fprintf(stderr, "goto "); params[0].Print(); break;
	case ILOP_NORETURN:  fprintf(stderr, "noreturn"); break;
	case ILOP_SCONVERT:  params[0].Print(); fprintf(stderr, " = sconvert "); params[1].Print(); break;
	case ILOP_UCONVERT:  params[0].Print(); fprintf(stderr, " = uconvert "); params[1].Print(); break;
	case ILOP_RETURN:  fprintf(stderr, "return "); params[0].Print(); break;
	case ILOP_RETURN_VOID:  fprintf(stderr, "return"); break;
	case ILOP_ALLOCA:  params[0].Print(); fprintf(stderr, " = alloca "); params[1].Print(); break;
	case ILOP_MEMCPY:  fprintf(stderr, "memcpy "); params[0].Print(); fprintf(stderr, ", "); params[1].Print(); fprintf(stderr, ", "); params[2].Print(); break;
	case ILOP_MEMSET:  fprintf(stderr, "memset "); params[0].Print(); fprintf(stderr, ", "); params[1].Print(); fprintf(stderr, ", "); params[2].Print(); break;
	case ILOP_STRLEN:  params[0].Print(); fprintf(stderr, " = strlen "); params[1].Print(); break;
	case ILOP_RDTSC:  params[0].Print(); fprintf(stderr, " = rdtsc"); break;
	case ILOP_RDTSC_LOW:  params[0].Print(); fprintf(stderr, " = rdtsc_low"); break;
	case ILOP_RDTSC_HIGH:  params[0].Print(); fprintf(stderr, " = rdtsc_high"); break;
	case ILOP_NEXT_ARG:  params[0].Print(); fprintf(stderr, " = next_arg "); params[1].Print(); fprintf(stderr, ", "); params[2].Print(); break;
	case ILOP_PREV_ARG:  params[0].Print(); fprintf(stderr, " = prev_arg "); params[1].Print(); fprintf(stderr, ", "); params[2].Print(); break;
	case ILOP_BYTESWAP:  params[0].Print(); fprintf(stderr, " = byteswap "); params[1].Print(); break;
	case ILOP_BREAKPOINT:  fprintf(stderr, "breakpoint"); break;
	case ILOP_CALL:
		params[0].Print();
		fprintf(stderr, " = ");
		params[1].Print();
		fprintf(stderr, "(");
		for (size_t i = 2; i < params.size(); i++)
		{
			if (i > 2)
				fprintf(stderr, ", ");
			params[i].Print();
		}
		fprintf(stderr, ")");
		break;
	case ILOP_SYSCALL:
		params[0].Print();
		fprintf(stderr, " = __syscall");
		fprintf(stderr, "(");
		for (size_t i = 1; i < params.size(); i++)
		{
			if (i > 1)
				fprintf(stderr, ", ");
			params[i].Print();
		}
		fprintf(stderr, ")");
		break;
	case ILOP_SYSCALL2:
		params[0].Print();
		fprintf(stderr, ", ");
		params[1].Print();
		fprintf(stderr, " = __syscall2");
		fprintf(stderr, "(");
		for (size_t i = 2; i < params.size(); i++)
		{
			if (i > 2)
				fprintf(stderr, ", ");
			params[i].Print();
		}
		fprintf(stderr, ")");
		break;
	default:
		fprintf(stderr, "<invalid>");
		break;
	}
}


ILBlock::ILBlock()
{
	m_index = 0;
	m_output = NULL;
	m_blockEnded = false;
}


ILBlock::ILBlock(size_t i)
{
	m_index = i;
	m_output = NULL;
	m_blockEnded = false;
}


ILBlock::~ILBlock()
{
	if (m_output)
	{
		free(m_output->code);
		delete m_output;
	}
}


void ILBlock::AddInstruction(const ILInstruction& instr)
{
	if (m_blockEnded)
	{
		// A block ending instruction has already been emitted, just ignore anything after it
		return;
	}

	// For instructions that use the special __undefined value, the output is considered
	// a "don't care".  For conditions, always take the false case.  For returns, convert
	// to a return of void (which ignores the return value).  For calls, leave the
	// parameter so that it can be skipped during parameter passing.  For others, omit the
	// instruction entirely.
	bool hasUndefinedResults = false;
	for (vector<ILParameter>::const_iterator i = instr.params.begin(); i != instr.params.end(); i++)
	{
		if (i->cls == ILPARAM_UNDEFINED)
		{
			hasUndefinedResults = true;
			break;
		}
	}

	if (hasUndefinedResults)
	{
		// Undefined output
		if (instr.operation == ILOP_RETURN)
			AddInstruction(ILOP_RETURN_VOID);
		else if (instr.operation == ILOP_CALL)
			m_instrs.push_back(instr);
		else if ((instr.operation == ILOP_SYSCALL) || (instr.operation == ILOP_SYSCALL2))
			m_instrs.push_back(instr);
		else if ((instr.operation == ILOP_IF_TRUE) || (instr.operation == ILOP_IF_LESS_THAN) ||
			(instr.operation == ILOP_IF_LESS_EQUAL) || (instr.operation == ILOP_IF_BELOW) ||
			(instr.operation == ILOP_IF_BELOW_EQUAL) || (instr.operation == ILOP_IF_EQUAL))
			AddInstruction(ILOP_GOTO, instr.params[2]);
	}
	else
	{
		// Normal instruction, add to list
		m_instrs.push_back(instr);
	}

	// Check for block ending instructions
	switch (instr.operation)
	{
	case ILOP_IF_TRUE:
	case ILOP_IF_LESS_THAN:
	case ILOP_IF_LESS_EQUAL:
	case ILOP_IF_BELOW:
	case ILOP_IF_BELOW_EQUAL:
	case ILOP_IF_EQUAL:
	case ILOP_GOTO:
	case ILOP_NORETURN:
	case ILOP_RETURN:
	case ILOP_RETURN_VOID:
		m_blockEnded = true;
		break;
	default:
		break;
	}
}


void ILBlock::RemoveLastInstruction()
{
	m_instrs.erase(m_instrs.end() - 1);
	m_blockEnded = false;
}


void ILBlock::SplitBlock(size_t firstToMove, ILBlock* target)
{
	if (target)
		target->m_instrs.insert(target->m_instrs.begin(), m_instrs.begin() + firstToMove, m_instrs.end());
	m_instrs.erase(m_instrs.begin() + firstToMove, m_instrs.end());
	if (target)
		target->m_blockEnded = m_blockEnded;
	m_blockEnded = false;
}


void ILBlock::ReplaceFunction(Function* from, Function* to)
{
	for (vector<ILInstruction>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		i->ReplaceFunction(from, to);
}


void ILBlock::ReplaceVariable(Variable* from, Variable* to)
{
	for (vector<ILInstruction>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		i->ReplaceVariable(from, to);
}


void ILBlock::CheckForUndefinedReferences(size_t& errors)
{
	for (vector<ILInstruction>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		i->CheckForUndefinedReferences(errors);
}


void ILBlock::ConvertStringsToVariables(map< string, Ref<Variable> >& stringMap)
{
	for (vector<ILInstruction>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		i->ConvertStringsToVariables(stringMap);
}


void ILBlock::SetOutputBlock(OutputBlock* output)
{
	if (m_output)
	{
		free(m_output->code);
		delete m_output;
	}

	m_output = output;
}


static int32_t SignedFieldExtract(uint32_t value, size_t offset, size_t size)
{
	int32_t result = (value >> offset) & ((1 << size) - 1);
	if (result & (1 << (size - 1)))
		result |= 0xffffffff << size;
	return result;
}


static void SignedFieldInsert(uint32_t* data, size_t offset, size_t size, int32_t value)
{
	uint32_t mask = ((1 << size) - 1) << offset;
	*data = ((*data) & ~mask) | ((value << offset) & mask);
}


bool ILBlock::CheckRelocations(uint64_t codeSectionBase, uint64_t dataSectionBase, std::vector<RelocationReference>& overflows)
{
	if (!m_output)
	{
		fprintf(stderr, "error: resolving relocations for block that has no output\n");
		return false;
	}

	for (vector<Relocation>::iterator i = m_output->relocs.begin(); i != m_output->relocs.end(); i++)
	{
		int64_t diff;
		switch (i->type)
		{
		case CODE_RELOC_RELATIVE_8:
			if (!i->target)
				break;
			diff = i->target->m_addr - (m_addr + i->offset + 1);
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				RelocationReference ref;
				ref.block = m_output;
				ref.reloc = &(*i);
				overflows.push_back(ref);
			}
			break;
		case CODE_RELOC_RELATIVE_32_FIELD:
			if (!i->target)
				break;
			diff = i->target->m_addr - (m_addr + i->offset + 4);
			diff += SignedFieldExtract(*(uint32_t*)((size_t)m_output->code + i->offset), i->bitOffset, i->bitSize) << i->bitShift;
			diff >>= i->bitShift;
			if ((diff < -(1 << (i->bitSize - 1))) || (diff >= (1 << (i->bitSize - 1))))
			{
				RelocationReference ref;
				ref.block = m_output;
				ref.reloc = &(*i);
				overflows.push_back(ref);
			}
			break;
		case CODE_RELOC_BASE_RELATIVE_8:
			if (i->target)
				diff = i->target->m_addr - codeSectionBase;
			else
				diff = (m_addr + i->offset + 1) - codeSectionBase;
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				RelocationReference ref;
				ref.block = m_output;
				ref.reloc = &(*i);
				overflows.push_back(ref);
			}
			break;
		case DATA_RELOC_RELATIVE_8:
			diff = (dataSectionBase + i->dataOffset) - (m_addr + i->offset + 1);
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				RelocationReference ref;
				ref.block = m_output;
				ref.reloc = &(*i);
				overflows.push_back(ref);
			}
			break;
		case DATA_RELOC_RELATIVE_32_FIELD:
			diff = (dataSectionBase + i->dataOffset) - (m_addr + i->offset + 4);
			diff += SignedFieldExtract(*(uint32_t*)((size_t)m_output->code + i->offset), i->bitOffset, i->bitSize) << i->bitShift;
			diff >>= i->bitShift;
			if ((diff < -(1 << (i->bitSize - 1))) || (diff >= (1 << (i->bitSize - 1))))
			{
				RelocationReference ref;
				ref.block = m_output;
				ref.reloc = &(*i);
				overflows.push_back(ref);
			}
			break;
		case DATA_RELOC_BASE_RELATIVE_8:
			diff = (dataSectionBase + i->dataOffset) - codeSectionBase;
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				RelocationReference ref;
				ref.block = m_output;
				ref.reloc = &(*i);
				overflows.push_back(ref);
			}
			break;
		case CODE_RELOC_RELATIVE_32:
		case CODE_RELOC_RELATIVE_64_SPLIT_FIELD:
		case CODE_RELOC_BASE_RELATIVE_32:
		case CODE_RELOC_ABSOLUTE_32:
		case CODE_RELOC_ABSOLUTE_64:
		case DATA_RELOC_RELATIVE_32:
		case DATA_RELOC_RELATIVE_64_SPLIT_FIELD:
		case DATA_RELOC_BASE_RELATIVE_32:
		case DATA_RELOC_ABSOLUTE_32:
		case DATA_RELOC_ABSOLUTE_64:
			break;
		default:
			fprintf(stderr, "error: invalid relocation\n");
			return false;
		}
	}

	return true;
}


bool ILBlock::ResolveRelocations(uint64_t codeSectionBase, uint64_t dataSectionBase)
{
	if (!m_output)
	{
		fprintf(stderr, "error: resolving relocations for block that has no output\n");
		return false;
	}

	for (vector<Relocation>::iterator i = m_output->relocs.begin(); i != m_output->relocs.end(); i++)
	{
		int64_t diff;
		switch (i->type)
		{
		case CODE_RELOC_RELATIVE_8:
			if (!i->target)
				break;
			diff = i->target->m_addr - (m_addr + i->offset + 1);
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				fprintf(stderr, "error: 8-bit relative reference out of range\n");
				return false;
			}
			*(int8_t*)((size_t)m_output->code + i->offset) = (int8_t)diff;
			break;
		case CODE_RELOC_BASE_RELATIVE_8:
			if (i->target)
				diff = i->target->m_addr - codeSectionBase;
			else
				diff = (m_addr + i->offset + 1) - codeSectionBase;
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				fprintf(stderr, "error: 8-bit relative reference out of range\n");
				return false;
			}
			*(int8_t*)((size_t)m_output->code + i->offset) = (int8_t)diff;
			break;
		case CODE_RELOC_RELATIVE_32:
			if (!i->target)
				break;
			diff = i->target->m_addr - (m_addr + i->offset + 4);
			diff += *(int32_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80000000LL) || (diff >= 0x80000000LL))
			{
				fprintf(stderr, "error: 32-bit relative reference out of range\n");
				return false;
			}
			*(int32_t*)((size_t)m_output->code + i->offset) = (int32_t)diff;
			break;
		case CODE_RELOC_RELATIVE_32_FIELD:
			if (!i->target)
				break;
			diff = i->target->m_addr - (m_addr + i->offset + 4);
			diff += SignedFieldExtract(*(uint32_t*)((size_t)m_output->code + i->offset), i->bitOffset, i->bitSize) << i->bitShift;
			diff >>= i->bitShift;
			if ((diff < -(1 << (i->bitSize - 1))) || (diff >= (1 << (i->bitSize - 1))))
			{
				fprintf(stderr, "error: %d-bit relative reference out of range\n", (int)i->bitSize);
				return false;
			}
			SignedFieldInsert((uint32_t*)((size_t)m_output->code + i->offset), i->bitOffset, i->bitSize, (int32_t)diff);
			break;
		case CODE_RELOC_BASE_RELATIVE_32:
			if (i->target)
				diff = i->target->m_addr - codeSectionBase;
			else
				diff = (m_addr + i->offset + 4) - codeSectionBase;
			diff += *(int32_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80000000LL) || (diff >= 0x80000000LL))
			{
				fprintf(stderr, "error: 32-bit relative reference out of range\n");
				return false;
			}
			*(int32_t*)((size_t)m_output->code + i->offset) = (int32_t)diff;
			break;
		case CODE_RELOC_ABSOLUTE_32:
			if (i->target)
				*(uint32_t*)((size_t)m_output->code + i->offset) += (uint32_t)(i->target->m_addr);
			else
				*(uint32_t*)((size_t)m_output->code + i->offset) += (uint32_t)(m_addr + i->offset + 4);
			break;
		case CODE_RELOC_ABSOLUTE_64:
			if (i->target)
				*(uint64_t*)((size_t)m_output->code + i->offset) += i->target->m_addr;
			else
				*(uint64_t*)((size_t)m_output->code + i->offset) += m_addr + i->offset + 8;
			break;
		case DATA_RELOC_RELATIVE_8:
			diff = (dataSectionBase + i->dataOffset) - (m_addr + i->offset + 1);
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				fprintf(stderr, "error: 8-bit relative reference out of range\n");
				return false;
			}
			*(int8_t*)((size_t)m_output->code + i->offset) = (int8_t)diff;
			break;
		case DATA_RELOC_BASE_RELATIVE_8:
			diff = (dataSectionBase + i->dataOffset) - codeSectionBase;
			diff += *(int8_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				fprintf(stderr, "error: 8-bit relative reference out of range\n");
				return false;
			}
			*(int8_t*)((size_t)m_output->code + i->offset) = (int8_t)diff;
			break;
		case DATA_RELOC_RELATIVE_32:
			diff = (dataSectionBase + i->dataOffset) - (m_addr + i->offset + 4);
			diff += *(int32_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80000000LL) || (diff >= 0x80000000LL))
			{
				fprintf(stderr, "error: 32-bit relative reference out of range\n");
				return false;
			}
			*(int32_t*)((size_t)m_output->code + i->offset) = (int32_t)diff;
			break;
		case DATA_RELOC_RELATIVE_32_FIELD:
			diff = (dataSectionBase + i->dataOffset) - (m_addr + i->offset + 4);
			diff += SignedFieldExtract(*(uint32_t*)((size_t)m_output->code + i->offset), i->bitOffset, i->bitSize) << i->bitShift;
			diff >>= i->bitShift;
			if ((diff < -(1 << (i->bitSize - 1))) || (diff >= (1 << (i->bitSize - 1))))
			{
				fprintf(stderr, "error: %d-bit relative reference out of range\n", (int)i->bitSize);
				return false;
			}
			SignedFieldInsert((uint32_t*)((size_t)m_output->code + i->offset), i->bitOffset, i->bitSize, (int32_t)diff);
			break;
		case DATA_RELOC_BASE_RELATIVE_32:
			diff = (dataSectionBase + i->dataOffset) - codeSectionBase;
			diff += *(int32_t*)((size_t)m_output->code + i->offset);
			if ((diff < -0x80000000LL) || (diff >= 0x80000000LL))
			{
				fprintf(stderr, "error: 32-bit relative reference out of range\n");
				return false;
			}
			*(int32_t*)((size_t)m_output->code + i->offset) = (int32_t)diff;
			break;
		case DATA_RELOC_ABSOLUTE_32:
			*(uint32_t*)((size_t)m_output->code + i->offset) = (uint32_t)(dataSectionBase + i->dataOffset);
			break;
		case DATA_RELOC_ABSOLUTE_64:
			*(uint64_t*)((size_t)m_output->code + i->offset) = dataSectionBase + i->dataOffset;
			break;
		default:
			fprintf(stderr, "error: invalid relocation\n");
			return false;
		}
	}

	return true;
}


bool ILBlock::EndsWithReturn() const
{
	if (m_instrs.size() == 0)
		return false;

	if (m_instrs[m_instrs.size() - 1].operation == ILOP_RETURN)
		return true;
	if (m_instrs[m_instrs.size() - 1].operation == ILOP_RETURN_VOID)
		return true;
	return false;
}


void ILBlock::TagReferences()
{
	for (vector<ILInstruction>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		i->TagReferences();
}


void ILBlock::ClearEntryAndExitBlocks()
{
	m_entryBlocks.clear();
	m_exitBlocks.clear();
}


void ILBlock::ClearExitBlocks()
{
	m_exitBlocks.clear();
}


void ILBlock::ResetDataFlowInfo(size_t bits)
{
	m_defPreserve.Reset(bits, true);
	m_defGenerate.Reset(bits, false);
	m_defReachIn.Reset(bits, false);
	m_defReachOut.Reset(bits, false);
	m_useDefChains.clear();
	m_useDefChains.resize(m_instrs.size());
	m_defUseChains.clear();
	m_defUseChains.resize(m_instrs.size());
}


void ILBlock::Serialize(OutputBlock* output)
{
	output->WriteInteger(m_instrs.size());
	for (vector<ILInstruction>::iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
		i->Serialize(output);
}


bool ILBlock::Deserialize(InputBlock* input)
{
	size_t instrCount;
	if (!input->ReadNativeInteger(instrCount))
		return false;
	for (size_t i = 0; i < instrCount; i++)
	{
		ILInstruction instr;
		if (!instr.Deserialize(input))
			return false;
		m_instrs.push_back(instr);
	}
	return true;
}


ILBlock* ILBlock::GetSerializationMapping(size_t i)
{
	return m_serializationMapping[i];
}


void ILBlock::SetSerializationMapping(size_t i, ILBlock* block)
{
	m_serializationMapping[i] = block;
}


void ILBlock::SaveSerializationMapping()
{
	m_savedSerializationMappings.push(m_serializationMapping);
	m_serializationMapping.clear();
}


void ILBlock::RestoreSerializationMapping()
{
	m_serializationMapping = m_savedSerializationMappings.top();
	m_savedSerializationMappings.pop();
}


void ILBlock::Print() const
{
	for (vector<ILInstruction>::const_iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
	{
		fprintf(stderr, "\t");
		i->Print();
		fprintf(stderr, "\n");
	}
}

