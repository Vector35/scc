#include <stdio.h>
#include "Function.h"
#include "Struct.h"
#include "ParserState.h"
#include "Output.h"

using namespace std;


size_t Function::m_nextSerializationIndex;
map< size_t, Ref<Function> > Function::m_serializationMap;


Function::Function()
{
	m_callingConvention = CALLING_CONVENTION_DEFAULT;
	m_location.lineNumber = 0;
	m_nextTempId = 0;
	m_defaultBlock = NULL;
	m_localScope = false;
	m_variableSizedStackFrame = false;
	m_serializationIndexValid = false;
}


Function::Function(const FunctionInfo& info, bool isLocalScope)
{
	m_returnValue = info.returnValue;
	m_callingConvention = info.callingConvention;
	m_name = info.name;
	m_location = info.location;
	m_nextTempId = 0;
	m_defaultBlock = NULL;
	m_localScope = isLocalScope;
	m_variableSizedStackFrame = false;
	m_serializationIndexValid = false;

	for (vector< pair< Ref<Type>, string > >::const_iterator i = info.params.begin(); i != info.params.end(); i++)
	{
		FunctionParameter param;
		param.type = i->first;
		param.name = i->second;
		m_params.push_back(param);
	}
}


Function::Function(const FunctionInfo& info, const vector< Ref<Variable> >& vars, Expr* body, bool isLocalScope)
{
	m_returnValue = info.returnValue;
	m_callingConvention = info.callingConvention;
	m_name = info.name;
	m_location = info.location;
	m_vars = vars;
	m_body = body;
	m_nextTempId = 0;
	m_defaultBlock = NULL;
	m_localScope = isLocalScope;
	m_variableSizedStackFrame = false;
	m_serializationIndexValid = false;

	for (vector< pair< Ref<Type>, string > >::const_iterator i = info.params.begin(); i != info.params.end(); i++)
	{
		FunctionParameter param;
		param.type = i->first;
		param.name = i->second;
		m_params.push_back(param);
	}
}


Function::~Function()
{
	for (vector<ILBlock*>::iterator i = m_ilBlocks.begin(); i != m_ilBlocks.end(); i++)
		delete *i;
}


Function* Function::Duplicate(DuplicateContext& dup)
{
	if (dup.funcs.find(this) != dup.funcs.end())
		return dup.funcs[this];

	Function* func = new Function();
	dup.funcs[this] = func;

	func->m_returnValue = m_returnValue->Duplicate(dup);
	func->m_callingConvention = m_callingConvention;
	func->m_name = m_name;
	func->m_location = m_location;
	func->m_body = (m_body != NULL) ? m_body->Duplicate(dup) : NULL;
	func->m_localScope = m_localScope;
	func->m_variableSizedStackFrame = m_variableSizedStackFrame;

	for (vector<FunctionParameter>::iterator i = m_params.begin(); i != m_params.end(); i++)
	{
		FunctionParameter param;
		param.type = i->type->Duplicate(dup);
		param.name = i->name;
		func->m_params.push_back(param);
	}

	for (vector< Ref<Variable> >::iterator i = m_vars.begin(); i != m_vars.end(); i++)
		func->m_vars.push_back((*i)->Duplicate(dup));

	return func;
}


bool Function::IsCompatible(const FunctionInfo& info)
{
	return IsCompatible(info.returnValue, info.callingConvention, info.params);
}


bool Function::IsCompatible(Type* returnValue, CallingConvention callingConvention,
	const vector< pair< Ref<Type>, string > >& params)
{
	if ((*m_returnValue) != (*returnValue))
		return false;
	if (callingConvention != m_callingConvention)
		return false;
	if (params.size() != m_params.size())
		return false;

	for (size_t i = 0; i < params.size(); i++)
	{
		if ((*params[i].first) != (*m_params[i].type))
			return false;
	}

	return true;
}


Type* Function::GetType() const
{
	vector< pair< Ref<Type>, string > > params;
	for (vector<FunctionParameter>::const_iterator i = m_params.begin(); i != m_params.end(); i++)
		params.push_back(pair< Ref<Type>, string >(i->type, i->name));
	return Type::FunctionType(m_returnValue, m_callingConvention, params);
}


void Function::GenerateIL(ParserState* state)
{
	m_ilBlocks.clear();
	ILBlock* entry = new ILBlock(0);
	m_ilBlocks.push_back(entry);

	m_body->GenerateIL(state, this, entry);

	// Ensure function always exits
	if (!entry->EndsWithReturn())
		entry->AddInstruction(ILOP_RETURN_VOID);
}


ILBlock* Function::CreateILBlock()
{
	ILBlock* block = new ILBlock(m_ilBlocks.size());
	m_ilBlocks.push_back(block);
	return block;
}


ILParameter Function::CreateTempVariable(Type* type)
{
	char tempStr[32];
	sprintf(tempStr, "@t%u", m_nextTempId++);
	Variable* var = new Variable(VAR_TEMP, type, tempStr);
	m_vars.push_back(var);
	return ILParameter(var);
}


void Function::SetLabel(const std::string& name, ILBlock* block)
{
	m_labels[name] = block;

	// Resolve forward references
	map< string, vector<LabelFixup> >::iterator i = m_labelFixups.find(name);
	if (i == m_labelFixups.end())
		return;

	for (vector<LabelFixup>::iterator j = i->second.begin(); j != i->second.end(); j++)
		j->block->SetInstructionParameter(j->index, 0, ILParameter(block));
	m_labelFixups.erase(i);
}


ILBlock* Function::GetLabel(const string& name) const
{
	map<string, ILBlock*>::const_iterator i = m_labels.find(name);
	if (i == m_labels.end())
		return NULL;
	return i->second;
}


void Function::AddLabelFixup(ILBlock* block, size_t i, const Location& loc, const string& name)
{
	LabelFixup fixup;
	fixup.block = block;
	fixup.index = i;
	fixup.location = loc;
	m_labelFixups[name].push_back(fixup);
}


void Function::ReportUndefinedLabels(ParserState* state)
{
	for (map< string, vector<LabelFixup> >::iterator i = m_labelFixups.begin(); i != m_labelFixups.end(); i++)
	{
		if (i->second.size() == 0)
			continue;

		state->Error();
		fprintf(stderr, "%s:%d: error: label '%s' is not defined\n", i->second[0].location.fileName.c_str(),
			i->second[0].location.lineNumber, i->first.c_str());
	}
}


ILBlock* Function::GetBreakBlock() const
{
	if (m_breakStack.empty())
		return NULL;
	return m_breakStack.top();
}


ILBlock* Function::GetContinueBlock() const
{
	if (m_continueStack.empty())
		return NULL;
	return m_continueStack.top();
}


void Function::PushSwitchLabels()
{
	m_switchLabelsStack.push(m_switchLabels);
	m_defaultStack.push(m_defaultBlock);
	m_switchLabels.clear();
	m_defaultBlock = NULL;
}


void Function::PopSwitchLabels()
{
	m_switchLabels = m_switchLabelsStack.top();
	m_switchLabelsStack.pop();
	m_defaultBlock = m_defaultStack.top();
	m_defaultStack.pop();
}


void Function::ReplaceFunction(Function* from, Function* to)
{
	if (m_body)
		m_body->ReplaceFunction(from, to);

	for (vector<ILBlock*>::iterator i = m_ilBlocks.begin(); i != m_ilBlocks.end(); i++)
		(*i)->ReplaceFunction(from, to);
}


void Function::ReplaceVariable(Variable* from, Variable* to)
{
	if (m_body)
		m_body->ReplaceVariable(from, to);

	for (vector<ILBlock*>::iterator i = m_ilBlocks.begin(); i != m_ilBlocks.end(); i++)
		(*i)->ReplaceVariable(from, to);
}


void Function::CheckForUndefinedReferences(size_t& errors)
{
	m_body->CheckForUndefinedReferences(errors);

	if (!errors)
	{
		// In theory it shouldn't be necessary to check the IL in addition to the parse tree, but check
		// anyway to catch any bugs in the rest of the code
		for (vector<ILBlock*>::iterator i = m_ilBlocks.begin(); i != m_ilBlocks.end(); i++)
			(*i)->CheckForUndefinedReferences(errors);
	}
}


void Function::Serialize(OutputBlock* output)
{
	if (m_serializationIndexValid)
	{
		output->WriteInteger(1);
		output->WriteInteger(m_serializationIndex);
		return;
	}

	m_serializationIndexValid = true;
	m_serializationIndex = m_nextSerializationIndex++;
	output->WriteInteger(0);
	output->WriteInteger(m_serializationIndex);

	m_returnValue->Serialize(output);
	output->WriteInteger(m_callingConvention);
	output->WriteString(m_name);

	output->WriteInteger(m_params.size());
	for (vector<FunctionParameter>::iterator i = m_params.begin(); i != m_params.end(); i++)
	{
		i->type->Serialize(output);
		output->WriteString(i->name);
	}

	output->WriteString(m_location.fileName);
	output->WriteInteger(m_location.lineNumber);

	output->WriteInteger(m_vars.size());
	for (size_t i = 0; i < m_vars.size(); i++)
		m_vars[i]->Serialize(output);

	output->WriteInteger((m_body != NULL) ? 1 : 0);
	if (m_body)
		m_body->Serialize(output);

	output->WriteInteger(m_ilBlocks.size());
	for (vector<ILBlock*>::iterator i = m_ilBlocks.begin(); i != m_ilBlocks.end(); i++)
		(*i)->Serialize(output);

	output->WriteInteger(m_nextTempId);
	output->WriteInteger(m_localScope ? 1 : 0);
	output->WriteInteger(m_variableSizedStackFrame ? 1 : 0);
}


bool Function::DeserializeInternal(InputBlock* input)
{
	m_returnValue = Type::Deserialize(input);
	if (!m_returnValue)
		return false;

	uint32_t convention;
	if (!input->ReadUInt32(convention))
		return false;
	m_callingConvention = (CallingConvention)convention;

	if (!input->ReadString(m_name))
		return false;

	size_t paramCount;
	if (!input->ReadNativeInteger(paramCount))
		return false;
	for (size_t i = 0; i < paramCount; i++)
	{
		FunctionParameter param;
		param.type = Type::Deserialize(input);
		if (!param.type)
			return false;
		if (!input->ReadString(param.name))
			return false;
		m_params.push_back(param);
	}

	if (!input->ReadString(m_location.fileName))
		return false;
	if (!input->ReadInt32(m_location.lineNumber))
		return false;

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

	bool bodyPresent;
	if (!input->ReadBool(bodyPresent))
		return false;
	if (bodyPresent)
	{
		m_body = Expr::Deserialize(input);
		if (!m_body)
			return false;
	}
	else
	{
		m_body = NULL;
	}

	size_t blockCount;
	if (!input->ReadNativeInteger(blockCount))
		return false;
	for (size_t i = 0; i < blockCount; i++)
	{
		ILBlock* block = new ILBlock(i);
		m_ilBlocks.push_back(block);
		ILBlock::SetSerializationMapping(i, block);
	}
	for (size_t i = 0; i < blockCount; i++)
	{
		if (!m_ilBlocks[i]->Deserialize(input))
			return false;
	}

	if (!input->ReadUInt32(m_nextTempId))
		return false;
	if (!input->ReadBool(m_localScope))
		return false;
	if (!input->ReadBool(m_variableSizedStackFrame))
		return false;

	return true;
}


Function* Function::Deserialize(InputBlock* input)
{
	bool existingFunc;
	size_t i;
	if (!input->ReadBool(existingFunc))
		return NULL;
	if (!input->ReadNativeInteger(i))
		return NULL;

	if (existingFunc)
		return m_serializationMap[i];

	Function* func = new Function();
	m_serializationMap[i] = func;
	if (func->DeserializeInternal(input))
		return func;
	return NULL;
}


void Function::Print()
{
	m_returnValue->Print();
	fprintf(stderr, " ");

	switch (m_callingConvention)
	{
	case CALLING_CONVENTION_CDECL:
		fprintf(stderr, "__cdecl ");
		break;
	case CALLING_CONVENTION_STDCALL:
		fprintf(stderr, "__stdcall ");
		break;
	case CALLING_CONVENTION_FASTCALL:
		fprintf(stderr, "__fastcall ");
		break;
	default:
		break;
	}

	fprintf(stderr, "%s(", m_name.c_str());

	for (size_t i = 0; i < m_params.size(); i++)
	{
		if (i > 0)
			fprintf(stderr, ", ");
		m_params[i].type->Print();
		if (m_params[i].name.size() != 0)
			fprintf(stderr, " %s", m_params[i].name.c_str());
	}

	fprintf(stderr, ")\n");

	if (!m_body)
	{
		fprintf(stderr, "\n");
		return;
	}

	for (vector< Ref<Variable> >::iterator i = m_vars.begin(); i != m_vars.end(); i++)
	{
		fprintf(stderr, "\t");
		if ((*i)->IsParameter())
			fprintf(stderr, "[param %ld] ", (*i)->GetParameterIndex());
		(*i)->GetType()->Print();
		if ((*i)->GetName().size() != 0)
			fprintf(stderr, " %s", (*i)->GetName().c_str());
		fprintf(stderr, "\n");
	}

	m_body->Print(0);
	fprintf(stderr, "\n\n");

	for (vector<ILBlock*>::iterator i = m_ilBlocks.begin(); i != m_ilBlocks.end(); i++)
	{
		fprintf(stderr, "%d:\n", (int)(*i)->GetIndex());
		(*i)->Print();
	}

	fprintf(stderr, "\n\n");
}

