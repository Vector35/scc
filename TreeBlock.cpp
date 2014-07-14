// Copyright (c) 2014 Rusty Wagner
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
#include "TreeBlock.h"
#include "Function.h"
#include "SymInstr.h"

using namespace std;


TreeBlock::TreeBlock(ILBlock* source, size_t idx): m_source(source), m_index(idx)
{
}


void TreeBlock::AddNode(TreeNode* node)
{
	m_nodes.push_back(node);
}


bool TreeBlock::GetMemberVariableAndOffset(ILParameter& operand, Variable*& var, size_t& offset, Type*& type)
{
	const StructMember* member;

	switch (operand.cls)
	{
	case ILPARAM_VAR:
		var = operand.variable;
		offset = 0;
		type = operand.variable->GetType();
		return true;
	case ILPARAM_MEMBER:
		if (!GetMemberVariableAndOffset(*operand.parent, var, offset, type))
			return false;
		if (!operand.structure)
		{
			fprintf(stderr, "internal error: reference to member in variable that is not a structure or union\n");
			m_errors++;
			return false;
		}

		member = operand.structure->GetMember(operand.stringValue);
		if (!member)
		{
			fprintf(stderr, "error: structure or union '%s' does not have member '%s'\n", operand.structure->GetName().c_str(),
				operand.stringValue.c_str());
			m_errors++;
			return false;
		}

		offset += member->offset;
		type = member->type;
		return true;
	default:
		fprintf(stderr, "internal error: invalid IL operand\n");
		m_errors++;
		return false;
	}
}


TreeNodeType TreeBlock::GetPtrType()
{
	return m_ptrType;
}


TreeNodeType TreeBlock::VariableTypeToNodeType(Type* type)
{
	if (type->IsFloat())
	{
		if (type->GetWidth() == 4)
			return NODETYPE_F32;
		else
			return NODETYPE_F64;
	}

	if (type->IsSigned())
	{
		switch (type->GetWidth())
		{
		case 1:
			return NODETYPE_S8;
		case 2:
			return NODETYPE_S16;
		case 4:
			return NODETYPE_S32;
		case 8:
			return NODETYPE_S64;
		default:
			break;
		}
	}
	else
	{
		switch (type->GetWidth())
		{
		case 1:
			return NODETYPE_U8;
		case 2:
			return NODETYPE_U16;
		case 4:
			return NODETYPE_U32;
		case 8:
			return NODETYPE_U64;
		default:
			break;
		}
	}

	return NODETYPE_UNDEFINED;
}


TreeNodeType TreeBlock::OperandToNodeType(ILParameter& operand)
{
	Variable* var;
	size_t offset;
	Type* type;

	switch (operand.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!GetMemberVariableAndOffset(operand, var, offset, type))
			return NODETYPE_UNDEFINED;
		return VariableTypeToNodeType(type);

	default:
		switch (operand.type)
		{
		case ILTYPE_INT8:
			return NODETYPE_U8;
		case ILTYPE_INT16:
			return NODETYPE_U16;
		case ILTYPE_INT32:
			return NODETYPE_U32;
		case ILTYPE_INT64:
			return NODETYPE_U64;
		case ILTYPE_FLOAT:
			return NODETYPE_F32;
		case ILTYPE_DOUBLE:
			return NODETYPE_F64;
		default:
			fprintf(stderr, "internal error: operand has invalid type\n");
			m_errors++;
			return NODETYPE_UNDEFINED;
		}
	}
}


TreeNode* TreeBlock::OperandToNode(const VariableAssignments& vars, ILParameter& operand)
{
	Variable* var;
	size_t offset;
	Type* type;
	map<Variable*, uint32_t>::const_iterator regVarIter;
	map<Variable*, uint32_t>::const_iterator highRegVarIter;
	map<Variable*, int32_t>::const_iterator stackVarIter;

	switch (operand.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!GetMemberVariableAndOffset(operand, var, offset, type))
			return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);

		if (var->IsGlobal())
		{
			return TreeNode::CreateNode(NODE_LOAD, VariableTypeToNodeType(type), TreeNode::CreateNode(NODE_REF, GetPtrType(),
				TreeNode::CreateGlobalVarNode(var->GetDataSectionOffset() + offset, VariableTypeToNodeType(type))));
		}

		regVarIter = vars.registerVariables.find(var);
		if ((offset == 0) && (regVarIter != vars.registerVariables.end()))
		{
			highRegVarIter = vars.highRegisterVariables.find(var);
			if (highRegVarIter != vars.highRegisterVariables.end())
			{
				return TreeNode::CreateLargeRegNode(regVarIter->second, highRegVarIter->second,
					vars.function->GetRegisterClass(regVarIter->second), vars.function->GetRegisterClass(highRegVarIter->second),
					VariableTypeToNodeType(type));
			}
			return TreeNode::CreateRegNode(regVarIter->second, vars.function->GetRegisterClass(regVarIter->second),
				VariableTypeToNodeType(type));
		}

		stackVarIter = vars.stackVariables.find(var);
		if (stackVarIter == vars.stackVariables.end())
		{
			fprintf(stderr, "internal error: invalid stack variable\n");
			m_errors++;
			return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);
		}

		return TreeNode::CreateNode(NODE_LOAD, VariableTypeToNodeType(type), TreeNode::CreateNode(NODE_REF, GetPtrType(),
			TreeNode::CreateStackVarNode(vars.stackVariableBase, stackVarIter->second, offset, VariableTypeToNodeType(type))));

	case ILPARAM_INT:
		return TreeNode::CreateImmediateNode(operand.integerValue);
	case ILPARAM_BOOL:
		return TreeNode::CreateImmediateNode(operand.boolValue ? 1 : 0);
	case ILPARAM_UNDEFINED:
		return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);
	case ILPARAM_FUNC:
		return TreeNode::CreateFunctionNode(operand.function);

	default:
		fprintf(stderr, "internal error: invalid IL operand\n");
		m_errors++;
		return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);
	}
}


TreeNode* TreeBlock::OperandRefToNode(const VariableAssignments& vars, ILParameter& operand)
{
	Variable* var;
	size_t offset;
	Type* type;
	map<Variable*, int32_t>::const_iterator stackVarIter;

	switch (operand.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!GetMemberVariableAndOffset(operand, var, offset, type))
			return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);

		if (var->IsGlobal())
		{
			return TreeNode::CreateNode(NODE_REF, GetPtrType(), TreeNode::CreateGlobalVarNode(var->GetDataSectionOffset() + offset,
				VariableTypeToNodeType(type)));
		}

		stackVarIter = vars.stackVariables.find(var);
		if (stackVarIter == vars.stackVariables.end())
		{
			fprintf(stderr, "internal error: referencing invalid stack variable\n");
			m_errors++;
			return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);
		}

		return TreeNode::CreateNode(NODE_REF, GetPtrType(),
			TreeNode::CreateStackVarNode(vars.stackVariableBase, stackVarIter->second, offset,
			VariableTypeToNodeType(type)));

	default:
		fprintf(stderr, "internal error: taking address of IL operand that is not valid\n");
		m_errors++;
		return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);
	}
}


void TreeBlock::StoreToOperand(const VariableAssignments& vars, ILParameter& operand, TreeNode* value)
{
	Variable* var;
	size_t offset;
	Type* type;
	map<Variable*, uint32_t>::const_iterator regVarIter;
	map<Variable*, uint32_t>::const_iterator highRegVarIter;
	map<Variable*, int32_t>::const_iterator stackVarIter;

	switch (operand.cls)
	{
	case ILPARAM_VAR:
	case ILPARAM_MEMBER:
		if (!GetMemberVariableAndOffset(operand, var, offset, type))
			break;

		if (var->IsGlobal())
		{
			AddNode(TreeNode::CreateNode(NODE_STORE, VariableTypeToNodeType(type), TreeNode::CreateNode(NODE_REF, GetPtrType(),
				TreeNode::CreateGlobalVarNode(var->GetDataSectionOffset(), VariableTypeToNodeType(type))), value));
		}

		regVarIter = vars.registerVariables.find(var);
		if ((offset == 0) && (regVarIter != vars.registerVariables.end()))
		{
			highRegVarIter = vars.highRegisterVariables.find(var);
			if (highRegVarIter != vars.highRegisterVariables.end())
			{
				AddNode(TreeNode::CreateNode(NODE_ASSIGN, VariableTypeToNodeType(type),
					TreeNode::CreateLargeRegNode(regVarIter->second, highRegVarIter->second,
					vars.function->GetRegisterClass(regVarIter->second), vars.function->GetRegisterClass(highRegVarIter->second),
					VariableTypeToNodeType(type)), value));
				break;
			}
			AddNode(TreeNode::CreateNode(NODE_ASSIGN, VariableTypeToNodeType(type), TreeNode::CreateRegNode(regVarIter->second,
				vars.function->GetRegisterClass(regVarIter->second), VariableTypeToNodeType(type)), value));
			break;
		}

		stackVarIter = vars.stackVariables.find(var);
		if (stackVarIter == vars.stackVariables.end())
		{
			fprintf(stderr, "internal error: invalid stack variable\n");
			m_errors++;
			break;
		}

		AddNode(TreeNode::CreateNode(NODE_STORE, VariableTypeToNodeType(type), TreeNode::CreateNode(NODE_REF, GetPtrType(),
			TreeNode::CreateStackVarNode(vars.stackVariableBase, stackVarIter->second, offset,
			VariableTypeToNodeType(type))), value));
		break;

	default:
		fprintf(stderr, "internal error: invalid IL destination\n");
		m_errors++;
		break;
	}
}


TreeNode* TreeBlock::ConstantMultiplyNode(TreeNode* val, uint64_t mult)
{
	if (mult == 0)
		return TreeNode::CreateImmediateNode(0);
	if (mult == 1)
		return val;

	for (uint32_t shiftCount = 0; shiftCount < 64; shiftCount++)
	{
		if (mult == ((uint64_t)1 << shiftCount))
			return TreeNode::CreateNode(NODE_SHL, val->GetType(), val, TreeNode::CreateImmediateNode(shiftCount));
	}

	return TreeNode::CreateNode(NODE_SMUL, val->GetType(), val, TreeNode::CreateImmediateNode(mult));
}


TreeNode* TreeBlock::ConstantDivideNode(TreeNode* val, uint64_t div)
{
	if (div == 0)
	{
		fprintf(stderr, "internal error: divide by zero\n");
		m_errors++;
		return TreeNode::CreateNode(NODE_UNDEFINED, NODETYPE_UNDEFINED);
	}

	if (div == 1)
		return val;

	for (uint32_t shiftCount = 0; shiftCount < 64; shiftCount++)
	{
		if (div == ((uint64_t)1 << shiftCount))
			return TreeNode::CreateNode(NODE_SAR, val->GetType(), val, TreeNode::CreateImmediateNode(shiftCount));
	}

	return TreeNode::CreateNode(NODE_SDIV, val->GetType(), val, TreeNode::CreateImmediateNode(div));
}


TreeNodeType TreeBlock::DoubleTypeSize(TreeNodeType type)
{
	switch (type)
	{
	case NODETYPE_S8:
		return NODETYPE_S16;
	case NODETYPE_U8:
		return NODETYPE_U16;
	case NODETYPE_S16:
		return NODETYPE_S32;
	case NODETYPE_U16:
		return NODETYPE_U32;
	case NODETYPE_S32:
		return NODETYPE_S64;
	case NODETYPE_U32:
		return NODETYPE_U64;
	case NODETYPE_S64:
		return NODETYPE_S128;
	case NODETYPE_U64:
		return NODETYPE_U128;
	default:
		fprintf(stderr, "internal error: invalid type\n");
		m_errors++;
		return NODETYPE_UNDEFINED;
	}
}


bool TreeBlock::GenerateFromILBlock(ILBlock* il, vector< Ref<TreeBlock> >& blocks, const VariableAssignments& vars,
	const Settings& settings)
{
	m_errors = 0;

	if (settings.preferredBits == 64)
		m_ptrType = NODETYPE_U64;
	else
		m_ptrType = NODETYPE_U32;

	for (vector<ILInstruction>::iterator i = il->GetInstructions().begin(); i != il->GetInstructions().end(); ++i)
	{
		ILInstruction& instr = *i;
		TreeNode* src;
		TreeNode* a;
		TreeNode* b;
		map<Variable*, int32_t>::const_iterator stackVarIter;
		vector< Ref<TreeNode> > params;
		const StructMember* member;

		switch (instr.operation)
		{
		case ILOP_ASSIGN:
			StoreToOperand(vars, instr.params[0], OperandToNode(vars, instr.params[1]));
			break;

		case ILOP_ADDRESS_OF:
			StoreToOperand(vars, instr.params[0], OperandRefToNode(vars, instr.params[1]));
			break;

		case ILOP_ADDRESS_OF_MEMBER:
			src = OperandToNode(vars, instr.params[1]);

			if (!instr.params[2].structure)
			{
				fprintf(stderr, "internal error: invalid structure reference\n");
				m_errors++;
				break;
			}

			member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
			if (!member)
			{
				fprintf(stderr, "error: member '%s' of structure '%s' not found\n",
					instr.params[2].stringValue.c_str(), instr.params[2].structure->GetName().c_str());
				m_errors++;
				break;
			}

			if (member->offset == 0)
				StoreToOperand(vars, instr.params[0], src);
			else
			{
				StoreToOperand(vars, instr.params[0],
					TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
					src, TreeNode::CreateImmediateNode(member->offset)));
			}
			break;

		case ILOP_DEREF:
			src = OperandToNode(vars, instr.params[1]);
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]), src));
			break;

		case ILOP_DEREF_MEMBER:
			src = OperandToNode(vars, instr.params[1]);

			if (!instr.params[2].structure)
			{
				fprintf(stderr, "internal error: invalid structure reference\n");
				m_errors++;
				break;
			}

			member = instr.params[2].structure->GetMember(instr.params[2].stringValue);
			if (!member)
			{
				fprintf(stderr, "error: member '%s' of structure '%s' not found\n",
					instr.params[2].stringValue.c_str(), instr.params[2].structure->GetName().c_str());
				m_errors++;
				break;
			}

			if (member->offset == 0)
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]), src));
			else
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]),
					TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
					src, TreeNode::CreateImmediateNode(member->offset))));
			}
			break;

		case ILOP_DEREF_ASSIGN:
			src = OperandToNode(vars, instr.params[0]);
			AddNode(TreeNode::CreateNode(NODE_STORE, OperandToNodeType(instr.params[1]),
				OperandToNode(vars, instr.params[0]), OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_DEREF_MEMBER_ASSIGN:
			src = OperandToNode(vars, instr.params[0]);

			if (!instr.params[1].structure)
			{
				fprintf(stderr, "internal error: invalid structure reference\n");
				m_errors++;
				break;
			}

			member = instr.params[1].structure->GetMember(instr.params[1].stringValue);
			if (!member)
			{
				fprintf(stderr, "error: member '%s' of structure '%s' not found\n",
					instr.params[1].stringValue.c_str(), instr.params[1].structure->GetName().c_str());
				m_errors++;
				break;
			}

			if (member->offset == 0)
			{
				AddNode(TreeNode::CreateNode(NODE_STORE, VariableTypeToNodeType(member->type), src,
					OperandToNode(vars, instr.params[2])));
			}
			else
			{
				AddNode(TreeNode::CreateNode(NODE_STORE, VariableTypeToNodeType(member->type),
					TreeNode::CreateNode(NODE_ADD, src->GetType(), src,
					TreeNode::CreateImmediateNode(member->offset)), OperandToNode(vars, instr.params[2])));
			}
			break;

		case ILOP_ARRAY_INDEX:
			src = OperandRefToNode(vars, instr.params[1]);

			if ((instr.params[2].cls == ILPARAM_INT) && (instr.params[2].integerValue == 0))
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]), src));
			else if ((instr.params[2].cls == ILPARAM_INT) && (src->GetClass() == NODE_REF) &&
				((src->GetChildNodes()[0]->GetClass() == NODE_STACK_VAR) ||
				(src->GetChildNodes()[0]->GetClass() == NODE_GLOBAL_VAR)))
			{
				src->GetChildNodes()[0]->SetImmediate(src->GetChildNodes()[0]->GetImmediate() +
					(instr.params[2].integerValue * instr.params[3].integerValue));
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]), src));
			}
			else if (instr.params[2].cls == ILPARAM_INT)
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]),
					TreeNode::CreateNode(NODE_ADD, src->GetType(), src,
					TreeNode::CreateImmediateNode(instr.params[2].integerValue * instr.params[3].integerValue))));
			}
			else
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_LOAD, OperandToNodeType(instr.params[0]),
					TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
					src, ConstantMultiplyNode(OperandToNode(vars, instr.params[2]),
					instr.params[3].integerValue))));
			}
			break;

		case ILOP_ARRAY_INDEX_ASSIGN:
			src = OperandRefToNode(vars, instr.params[0]);

			if ((instr.params[1].cls == ILPARAM_INT) && (instr.params[1].integerValue == 0))
			{
				AddNode(TreeNode::CreateNode(NODE_STORE, OperandToNodeType(instr.params[3]), src,
					OperandToNode(vars, instr.params[3])));
			}
			else if ((instr.params[1].cls == ILPARAM_INT) && (src->GetClass() == NODE_REF) &&
				((src->GetChildNodes()[0]->GetClass() == NODE_STACK_VAR) ||
				(src->GetChildNodes()[0]->GetClass() == NODE_GLOBAL_VAR)))
			{
				src->GetChildNodes()[0]->SetImmediate(src->GetChildNodes()[0]->GetImmediate() +
					(instr.params[1].integerValue * instr.params[2].integerValue));
				AddNode(TreeNode::CreateNode(NODE_STORE, OperandToNodeType(instr.params[3]), src,
					OperandToNode(vars, instr.params[3])));
			}
			else if (instr.params[1].cls == ILPARAM_INT)
			{
				AddNode(TreeNode::CreateNode(NODE_STORE, OperandToNodeType(instr.params[3]),
					TreeNode::CreateNode(NODE_ADD, src->GetType(), src,
					TreeNode::CreateImmediateNode(instr.params[1].integerValue * instr.params[2].integerValue)),
					OperandToNode(vars, instr.params[3])));
			}
			else
			{
				AddNode(TreeNode::CreateNode(NODE_STORE, OperandToNodeType(instr.params[3]),
					TreeNode::CreateNode(NODE_ADD, src->GetType(), src,
					ConstantMultiplyNode(OperandToNode(vars, instr.params[1]), instr.params[2].integerValue)),
					OperandToNode(vars, instr.params[3])));
			}
			break;

		case ILOP_PTR_ADD:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), ConstantMultiplyNode(
				OperandToNode(vars, instr.params[2]), instr.params[3].integerValue)));
			break;

		case ILOP_PTR_SUB:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SUB, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), ConstantMultiplyNode(
				OperandToNode(vars, instr.params[2]), instr.params[3].integerValue)));
			break;

		case ILOP_PTR_DIFF:
			StoreToOperand(vars, instr.params[0], ConstantDivideNode(TreeNode::CreateNode(NODE_SUB,
				OperandToNodeType(instr.params[0]), OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])),
				instr.params[3].integerValue));
			break;

		case ILOP_ADD:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SUB:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SUB, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SMULT:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SMUL, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_UMULT:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_UMUL, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SDIV:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SDIV, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_UDIV:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_UDIV, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SMOD:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SMOD, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_UMOD:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_UMOD, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_AND:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_AND, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_OR:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_OR, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_XOR:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_XOR, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SHL:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SHL, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SHR:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SHR, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_SAR:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SAR, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_NEG:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_NEG, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_NOT:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_NOT, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_IF_TRUE:
			AddNode(TreeNode::CreateNode(NODE_IFTRUE, NODETYPE_UNDEFINED, OperandToNode(vars, instr.params[0]),
				TreeNode::CreateBlockNode(blocks[instr.params[1].block->GetIndex()]),
				TreeNode::CreateBlockNode(blocks[instr.params[2].block->GetIndex()])));
			break;

		case ILOP_IF_LESS_THAN:
			AddNode(TreeNode::CreateNode(NODE_IFSLT, NODETYPE_UNDEFINED,
				OperandToNode(vars, instr.params[0]), OperandToNode(vars, instr.params[1]),
				TreeNode::CreateBlockNode(blocks[instr.params[2].block->GetIndex()]),
				TreeNode::CreateBlockNode(blocks[instr.params[3].block->GetIndex()])));
			break;

		case ILOP_IF_LESS_EQUAL:
			AddNode(TreeNode::CreateNode(NODE_IFSLE, NODETYPE_UNDEFINED,
				OperandToNode(vars, instr.params[0]), OperandToNode(vars, instr.params[1]),
				TreeNode::CreateBlockNode(blocks[instr.params[2].block->GetIndex()]),
				TreeNode::CreateBlockNode(blocks[instr.params[3].block->GetIndex()])));
			break;

		case ILOP_IF_BELOW:
			AddNode(TreeNode::CreateNode(NODE_IFULT, NODETYPE_UNDEFINED,
				OperandToNode(vars, instr.params[0]), OperandToNode(vars, instr.params[1]),
				TreeNode::CreateBlockNode(blocks[instr.params[2].block->GetIndex()]),
				TreeNode::CreateBlockNode(blocks[instr.params[3].block->GetIndex()])));
			break;

		case ILOP_IF_BELOW_EQUAL:
			AddNode(TreeNode::CreateNode(NODE_IFULE, NODETYPE_UNDEFINED,
				OperandToNode(vars, instr.params[0]), OperandToNode(vars, instr.params[1]),
				TreeNode::CreateBlockNode(blocks[instr.params[2].block->GetIndex()]),
				TreeNode::CreateBlockNode(blocks[instr.params[3].block->GetIndex()])));
			break;

		case ILOP_IF_EQUAL:
			AddNode(TreeNode::CreateNode(NODE_IFE, NODETYPE_UNDEFINED,
				OperandToNode(vars, instr.params[0]), OperandToNode(vars, instr.params[1]),
				TreeNode::CreateBlockNode(blocks[instr.params[2].block->GetIndex()]),
				TreeNode::CreateBlockNode(blocks[instr.params[3].block->GetIndex()])));
			break;

		case ILOP_GOTO:
			AddNode(TreeNode::CreateNode(NODE_GOTO, NODETYPE_UNDEFINED,
				TreeNode::CreateBlockNode(blocks[instr.params[0].block->GetIndex()])));
			break;

		case ILOP_CALL:
			params.clear();
			for (size_t param = 3; param < instr.params.size(); param++)
				params.push_back(OperandToNode(vars, instr.params[param]));

			if (instr.params[0].cls == ILPARAM_VOID)
			{
				AddNode(TreeNode::CreateCallNode(OperandToNode(vars, instr.params[1]), (size_t)instr.params[2].integerValue,
					params, NODETYPE_UNDEFINED));
			}
			else
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateCallNode(OperandToNode(vars, instr.params[1]),
					(size_t)instr.params[2].integerValue, params, OperandToNodeType(instr.params[0])));
			}
			break;

		case ILOP_NORETURN:
			AddNode(TreeNode::CreateNode(NODE_NORETURN, NODETYPE_UNDEFINED));
			break;

		case ILOP_SCONVERT:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SCONVERT, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_UCONVERT:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_UCONVERT, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_RETURN:
			AddNode(TreeNode::CreateNode(NODE_RETURN, NODETYPE_UNDEFINED, OperandToNode(vars, instr.params[0])));
			break;

		case ILOP_RETURN_VOID:
			AddNode(TreeNode::CreateNode(NODE_RETURNVOID, NODETYPE_UNDEFINED));
			break;

		case ILOP_ALLOCA:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ALLOCA, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_MEMCPY:
			AddNode(TreeNode::CreateNode(NODE_MEMCPY, NODETYPE_UNDEFINED, OperandToNode(vars, instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_MEMSET:
			AddNode(TreeNode::CreateNode(NODE_MEMSET, NODETYPE_UNDEFINED, OperandToNode(vars, instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_STRLEN:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_STRLEN, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_SYSCALL:
			params.clear();
			for (size_t param = 2; param < instr.params.size(); param++)
				params.push_back(OperandToNode(vars, instr.params[param]));

			if (instr.params[0].cls == ILPARAM_VOID)
			{
				AddNode(TreeNode::CreateSyscallNode(OperandToNode(vars, instr.params[1]),
					params, NODETYPE_UNDEFINED));
			}
			else
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateSyscallNode(OperandToNode(vars, instr.params[1]),
					params, OperandToNodeType(instr.params[0])));
			}
			break;

		case ILOP_SYSCALL2:
			params.clear();
			for (size_t param = 3; param < instr.params.size(); param++)
				params.push_back(OperandToNode(vars, instr.params[param]));

			a = OperandToNode(vars, instr.params[0]);
			b = OperandToNode(vars, instr.params[1]);
			if ((a->GetClass() != NODE_REG) || (b->GetClass() != NODE_REG))
			{
				fprintf(stderr, "internal error: two parameter syscall requires temporary register destination\n");
				m_errors++;
				break;
			}

			AddNode(TreeNode::CreateNode(NODE_ASSIGN, DoubleTypeSize(a->GetType()),
				TreeNode::CreateLargeRegNode(a->GetRegister(), b->GetRegister(),
				vars.function->GetRegisterClass(a->GetRegister()), vars.function->GetRegisterClass(b->GetRegister()),
				DoubleTypeSize(a->GetType())), TreeNode::CreateSyscallNode(OperandToNode(vars, instr.params[2]),
				params, DoubleTypeSize(a->GetType()))));
			break;

		case ILOP_RDTSC:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_RDTSC, NODETYPE_UNDEFINED));
			break;

		case ILOP_RDTSC_LOW:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_RDTSC_LOW, NODETYPE_UNDEFINED));
			break;

		case ILOP_RDTSC_HIGH:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_RDTSC_HIGH, NODETYPE_UNDEFINED));
			break;

		case ILOP_INITIAL_VARARG:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_VARARG, NODETYPE_UNDEFINED));
			break;

		case ILOP_NEXT_ARG:
			if (settings.stackGrowsUp)
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SUB, OperandToNodeType(instr.params[0]),
					OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			}
			else
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
					OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			}
			break;

		case ILOP_PREV_ARG:
			if (settings.stackGrowsUp)
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ADD, OperandToNodeType(instr.params[0]),
					OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			}
			else
			{
				StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SUB, OperandToNodeType(instr.params[0]),
					OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			}
			break;

		case ILOP_BYTESWAP:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_BYTESWAP, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_BREAKPOINT:
			AddNode(TreeNode::CreateNode(NODE_BREAKPOINT, NODETYPE_UNDEFINED));
			break;

		case ILOP_POW:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_POW, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1]), OperandToNode(vars, instr.params[2])));
			break;

		case ILOP_FLOOR:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_FLOOR, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_CEIL:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_CEIL, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_SQRT:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SQRT, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_SIN:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_SIN, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_COS:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_COS, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_TAN:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_TAN, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_ASIN:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ASIN, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_ACOS:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ACOS, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		case ILOP_ATAN:
			StoreToOperand(vars, instr.params[0], TreeNode::CreateNode(NODE_ATAN, OperandToNodeType(instr.params[0]),
				OperandToNode(vars, instr.params[1])));
			break;

		default:
			fprintf(stderr, "internal error: invalid IL instruction\n");
			m_errors++;
			break;
		}

		if (m_errors)
			break;
	}

	return m_errors == 0;
}


void TreeBlock::Print() const
{
	for (vector< Ref<TreeNode> >::const_iterator i = m_nodes.begin(); i != m_nodes.end(); i++)
	{
		fprintf(stderr, "\t");
		(*i)->Print();
		fprintf(stderr, "\n");
	}
}

