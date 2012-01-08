#include <stdio.h>
#include <stdlib.h>
#include "ILBlock.h"
#include "Variable.h"
#include "Function.h"
#include "Struct.h"
#include "Output.h"

using namespace std;

	
ILParameter::ILParameter()
{
	cls = ILPARAM_VOID;
	integerValue = 0;
}


ILParameter::ILParameter(bool b)
{
	cls = ILPARAM_BOOL;
	boolValue = b;
	type = Type::BoolType();
}


ILParameter::ILParameter(Type* t, int64_t i)
{
	cls = ILPARAM_INT;
	integerValue = i;
	type = t;
}


ILParameter::ILParameter(Type* t, double f)
{
	cls = ILPARAM_FLOAT;
	floatValue = f;
	type = t;
}


ILParameter::ILParameter(const string& str, ILParameterClass c)
{
	cls = c;
	stringValue = str;
	type = Type::PointerType(Type::IntType(1, true), 1); // const char*
	type->SetConst(true);
}


ILParameter::ILParameter(Variable* var)
{
	cls = ILPARAM_VAR;
	variable = var;
	type = var->GetType();
}


ILParameter::ILParameter(Function* func)
{
	cls = ILPARAM_FUNC;
	function = func;
	type = func->GetType();
}


ILParameter::ILParameter(ILBlock* b)
{
	cls = ILPARAM_BLOCK;
	block = b;
	type = Type::VoidType();
}


ILParameter::ILParameter(const ILParameter& obj, const string& str)
{
	cls = ILPARAM_MEMBER;
	parent = new ILParameter(obj);
	stringValue = str;
}


ILParameter::ILParameter(const ILParameter& param)
{
	cls = param.cls;
	type = param.type;
	stringValue = param.stringValue;
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


void ILParameter::Print() const
{
	switch (cls)
	{
	case ILPARAM_VOID:  fprintf(stderr, "void"); break;
	case ILPARAM_INT:  fprintf(stderr, "%lld", (long long)integerValue); break;
	case ILPARAM_FLOAT:  fprintf(stderr, "%f", floatValue); break;
	case ILPARAM_STRING:  fprintf(stderr, "\"%s\"", stringValue.c_str()); break;
	case ILPARAM_NAME:  fprintf(stderr, "%s", stringValue.c_str()); break;
	case ILPARAM_BOOL:  fprintf(stderr, "%s", boolValue ? "true" : "false"); break;
	case ILPARAM_VAR:  fprintf(stderr, "var<%s>", variable->GetName().c_str()); break;
	case ILPARAM_FUNC:  fprintf(stderr, "func<%s>", function->GetName().c_str()); break;
	case ILPARAM_BLOCK:  fprintf(stderr, "block<%d>", (int)block->GetIndex()); break;
	case ILPARAM_MEMBER:  parent->Print(); fprintf(stderr, ".%s", stringValue.c_str()); break;
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


void ILInstruction::Print() const
{
	switch (operation)
	{
	case ILOP_ASSIGN:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); break;
	case ILOP_ADDRESS_OF:  params[0].Print(); fprintf(stderr, " = &"); params[1].Print(); break;
	case ILOP_DEREF:  params[0].Print(); fprintf(stderr, " = *"); params[1].Print(); break;
	case ILOP_DEREF_MEMBER:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, "->"); params[2].Print(); break;
	case ILOP_DEREF_ASSIGN:  fprintf(stderr, "*"); params[0].Print(); fprintf(stderr, " = "); params[1].Print(); break;
	case ILOP_DEREF_MEMBER_ASSIGN:  params[0].Print(); fprintf(stderr, "->"); params[1].Print(); fprintf(stderr, " = "); params[2].Print(); break;
	case ILOP_ARRAY_INDEX:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, "["); params[2].Print(); fprintf(stderr, "]"); break;
	case ILOP_ARRAY_INDEX_ASSIGN:  params[0].Print(); fprintf(stderr, "["); params[1].Print(); fprintf(stderr, "] = "); params[2].Print(); break;
	case ILOP_ADD:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " + "); params[2].Print(); break;
	case ILOP_SUB:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " - "); params[2].Print(); break;
	case ILOP_MULT:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " * "); params[2].Print(); break;
	case ILOP_DIV:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " / "); params[2].Print(); break;
	case ILOP_MOD:  params[0].Print(); fprintf(stderr, " = "); params[1].Print(); fprintf(stderr, " %% "); params[2].Print(); break;
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
	case ILOP_CONVERT:  params[0].Print(); fprintf(stderr, " = convert "); params[1].Print(); break;
	case ILOP_RETURN:  fprintf(stderr, "return "); params[0].Print(); break;
	case ILOP_RETURN_VOID:  fprintf(stderr, "return"); break;
	case ILOP_ALLOCA:  params[0].Print(); fprintf(stderr, " = alloca "); params[1].Print(); break;
	case ILOP_MEMCPY:  fprintf(stderr, "memcpy "); params[0].Print(); fprintf(stderr, ", "); params[1].Print(); fprintf(stderr, ", "); params[2].Print(); break;
	case ILOP_MEMSET:  fprintf(stderr, "memset "); params[0].Print(); fprintf(stderr, ", "); params[1].Print(); fprintf(stderr, ", "); params[2].Print(); break;
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
	default:
		fprintf(stderr, "<invalid>");
		break;
	}
}


ILBlock::ILBlock()
{
	m_index = 0;
	m_output = NULL;
}


ILBlock::ILBlock(size_t i)
{
	m_index = i;
	m_output = NULL;
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


void ILBlock::SetOutputBlock(OutputBlock* output)
{
	if (m_output)
	{
		free(m_output->code);
		delete m_output;
	}

	m_output = output;
}


bool ILBlock::ResolveRelocations()
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
		case RELOC_RELATIVE_8:
			diff = i->target->m_addr - (m_addr + i->offset + 1);
			if ((diff < -0x80) || (diff >= 0x80))
			{
				fprintf(stderr, "error: 8-bit relative reference out of range\n");
				return false;
			}
			*(int8_t*)((size_t)m_output->code + i->offset) += (int8_t)diff;
			break;
		case RELOC_RELATIVE_32:
			diff = i->target->m_addr - (m_addr + i->offset + 4);
			if ((diff < -0x80000000LL) || (diff >= 0x80000000LL))
			{
				fprintf(stderr, "error: 32-bit relative reference out of range\n");
				return false;
			}
			*(int32_t*)((size_t)m_output->code + i->offset) += (int32_t)diff;
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


void ILBlock::Print() const
{
	for (vector<ILInstruction>::const_iterator i = m_instrs.begin(); i != m_instrs.end(); i++)
	{
		fprintf(stderr, "\t");
		i->Print();
		fprintf(stderr, "\n");
	}
}

