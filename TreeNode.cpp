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
#include "TreeNode.h"
#include "TreeBlock.h"
#include "SymInstr.h"
#include "Function.h"

using namespace std;


TreeNode::TreeNode(TreeNodeClass cls): m_class(cls), m_type(NODETYPE_UNDEFINED)
{
}


TreeNode::TreeNode(const TreeNode& copy)
{
	m_type = copy.m_type;
	m_class = copy.m_class;
	m_block = copy.m_block;
	m_func = copy.m_func;
	m_reg = copy.m_reg;
	m_highReg = copy.m_highReg;
	m_regClass = copy.m_regClass;
	m_highRegClass = copy.m_highRegClass;
	m_var = copy.m_var;
	m_immediate = copy.m_immediate;
	for (vector< Ref<TreeNode> >::const_iterator i = copy.m_children.begin(); i != copy.m_children.end(); ++i)
		m_children.push_back(new TreeNode(**i));
}


void TreeNode::SetBlock(TreeBlock* block)
{
	m_block = block;
}


void TreeNode::SetFunction(Function* func)
{
	m_func = func;
}


TreeBlock* TreeNode::GetBlock() const
{
	return m_block;
}


Function* TreeNode::GetFunction() const
{
	return m_func;
}


TreeNode* TreeNode::CreateStackVarNode(uint32_t base, int32_t var, int64_t offset, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_STACK_VAR);
	result->SetType(type);
	result->SetRegister(base);
	result->SetVariable(var);
	result->SetImmediate(offset);
	return result;
}


TreeNode* TreeNode::CreateGlobalVarNode(int64_t offset, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_GLOBAL_VAR);
	result->SetType(type);
	result->SetImmediate(offset);
	return result;
}


TreeNode* TreeNode::CreateBlockNode(TreeBlock* block)
{
	TreeNode* result = new TreeNode(NODE_BLOCK);
	result->SetBlock(block);
	return result;
}


TreeNode* TreeNode::CreateFunctionNode(Function* func)
{
	TreeNode* result = new TreeNode(NODE_FUNC);
	result->SetFunction(func);
	return result;
}


TreeNode* TreeNode::CreateRegNode(uint32_t reg, uint32_t regClass, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_REG);
	result->SetType(type);
	result->SetRegister(reg);
	result->SetRegisterClass(reg);
	return result;
}


TreeNode* TreeNode::CreateLargeRegNode(uint32_t low, uint32_t high, uint32_t lowClass, uint32_t highClass, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_LARGE_REG);
	result->SetType(type);
	result->SetRegister(low);
	result->SetHighRegister(high);
	result->SetRegisterClass(lowClass);
	result->SetHighRegisterClass(highClass);
	return result;
}


TreeNode* TreeNode::CreateImmediateNode(int64_t immed)
{
	TreeNode* result = new TreeNode(NODE_IMMED);
	result->SetImmediate(immed);
	return result;
}


TreeNode* TreeNode::CreateCallNode(TreeNode* func, size_t stdParamCount, const std::vector< Ref<TreeNode> >& params,
	TreeNodeType returnType)
{
	TreeNode* result = new TreeNode(NODE_CALL);
	result->SetType(returnType);
	result->SetImmediate(stdParamCount);
	result->AddChildNode(func);
	result->AddChildNodes(params);
	return result;
}


TreeNode* TreeNode::CreateSyscallNode(TreeNode* num, const std::vector< Ref<TreeNode> >& params, TreeNodeType returnType)
{
	TreeNode* result = new TreeNode(NODE_SYSCALL);
	result->SetType(returnType);
	result->AddChildNode(num);
	result->AddChildNodes(params);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNodeType type)
{
	TreeNode* result = new TreeNode(cls);
	result->SetType(type);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a)
{
	TreeNode* result = new TreeNode(cls);
	result->SetType(type);
	result->AddChildNode(a);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a, TreeNode* b)
{
	TreeNode* result = new TreeNode(cls);
	result->SetType(type);
	result->AddChildNode(a);
	result->AddChildNode(b);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a, TreeNode* b, TreeNode* c)
{
	TreeNode* result = new TreeNode(cls);
	result->SetType(type);
	result->AddChildNode(a);
	result->AddChildNode(b);
	result->AddChildNode(c);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a, TreeNode* b, TreeNode* c, TreeNode* d)
{
	TreeNode* result = new TreeNode(cls);
	result->SetType(type);
	result->AddChildNode(a);
	result->AddChildNode(b);
	result->AddChildNode(c);
	result->AddChildNode(d);
	return result;
}


void TreeNode::PrintType() const
{
	switch (m_type)
	{
	case NODETYPE_U8:
		fprintf(stderr, ":U8");
		break;
	case NODETYPE_S8:
		fprintf(stderr, ":S8");
		break;
	case NODETYPE_U16:
		fprintf(stderr, ":U16");
		break;
	case NODETYPE_S16:
		fprintf(stderr, ":S16");
		break;
	case NODETYPE_U32:
		fprintf(stderr, ":U32");
		break;
	case NODETYPE_S32:
		fprintf(stderr, ":S32");
		break;
	case NODETYPE_U64:
		fprintf(stderr, ":U64");
		break;
	case NODETYPE_S64:
		fprintf(stderr, ":S64");
		break;
	case NODETYPE_U128:
		fprintf(stderr, ":U128");
		break;
	case NODETYPE_S128:
		fprintf(stderr, ":S128");
		break;
	case NODETYPE_F32:
		fprintf(stderr, ":F32");
		break;
	case NODETYPE_F64:
		fprintf(stderr, ":F64");
		break;
	default:
		fprintf(stderr, ":VOID");
		break;
	}
}


void TreeNode::Print() const
{
	switch (m_class)
	{
	case NODE_NOP:
		fprintf(stderr, "nop");
		break;
	case NODE_STACK_VAR:
		fprintf(stderr, "stack:");
		switch (m_reg)
		{
		case SYMREG_SP:
			fprintf(stderr, "sp");
			break;
		case SYMREG_BP:
			fprintf(stderr, "sp");
			break;
		default:
			fprintf(stderr, "reg%u", m_reg);
			break;
		}
		fprintf(stderr, "+var%d", m_var);
		if (m_immediate != 0)
			fprintf(stderr, "%+" PRIi64, m_immediate);
		PrintType();
		break;
	case NODE_GLOBAL_VAR:
		fprintf(stderr, "global%+" PRIi64, m_immediate);
		PrintType();
		break;
	case NODE_REG:
		fprintf(stderr, "reg%u", m_reg);
		PrintType();
		break;
	case NODE_LARGE_REG:
		fprintf(stderr, "reg%u:reg%u", m_reg, m_highReg);
		PrintType();
		break;
	case NODE_BLOCK:
		fprintf(stderr, "block %u", (int)m_block->GetIndex());
		break;
	case NODE_FUNC:
		fprintf(stderr, "func %s", m_func->GetName().c_str());
		break;
	case NODE_UNDEFINED:
		fprintf(stderr, "undefined");
		break;
	case NODE_IMMED:
		fprintf(stderr, "%" PRIi64, m_immediate);
		break;
	case NODE_ASSIGN:
		fprintf(stderr, "assign");
		break;
	case NODE_LOAD:
		fprintf(stderr, "load");
		PrintType();
		break;
	case NODE_STORE:
		fprintf(stderr, "store");
		PrintType();
		break;
	case NODE_REF:
		fprintf(stderr, "ref");
		break;
	case NODE_ADD:
		fprintf(stderr, "add");
		break;
	case NODE_SUB:
		fprintf(stderr, "sub");
		break;
	case NODE_SMUL:
		fprintf(stderr, "smul");
		break;
	case NODE_UMUL:
		fprintf(stderr, "umul");
		break;
	case NODE_SDIV:
		fprintf(stderr, "sdiv");
		break;
	case NODE_UDIV:
		fprintf(stderr, "udiv");
		break;
	case NODE_SMOD:
		fprintf(stderr, "smod");
		break;
	case NODE_UMOD:
		fprintf(stderr, "umod");
		break;
	case NODE_AND:
		fprintf(stderr, "and");
		break;
	case NODE_OR:
		fprintf(stderr, "or");
		break;
	case NODE_XOR:
		fprintf(stderr, "xor");
		break;
	case NODE_SHL:
		fprintf(stderr, "shl");
		break;
	case NODE_SHR:
		fprintf(stderr, "shr");
		break;
	case NODE_SAR:
		fprintf(stderr, "sar");
		break;
	case NODE_NEG:
		fprintf(stderr, "neg");
		break;
	case NODE_NOT:
		fprintf(stderr, "not");
		break;
	case NODE_IFTRUE:
		fprintf(stderr, "iftrue");
		break;
	case NODE_IFSLT:
		fprintf(stderr, "ifslt");
		break;
	case NODE_IFULT:
		fprintf(stderr, "ifult");
		break;
	case NODE_IFSLE:
		fprintf(stderr, "ifsle");
		break;
	case NODE_IFULE:
		fprintf(stderr, "ifule");
		break;
	case NODE_IFE:
		fprintf(stderr, "ife");
		break;
	case NODE_GOTO:
		fprintf(stderr, "goto");
		break;
	case NODE_CALL:
		fprintf(stderr, "call[%" PRIi64 "]", m_immediate);
		break;
	case NODE_SYSCALL:
		fprintf(stderr, "syscall");
		break;
	case NODE_SCONVERT:
		fprintf(stderr, "sconvert");
		PrintType();
		break;
	case NODE_UCONVERT:
		fprintf(stderr, "uconvert");
		PrintType();
		break;
	case NODE_RETURN:
		fprintf(stderr, "return");
		break;
	case NODE_RETURNVOID:
		fprintf(stderr, "returnvoid");
		break;
	case NODE_VARARG:
		fprintf(stderr, "vararg");
		break;
	case NODE_BYTESWAP:
		fprintf(stderr, "byteswap");
		break;
	case NODE_BREAKPOINT:
		fprintf(stderr, "breakpoint");
		break;
	case NODE_POW:
		fprintf(stderr, "pow");
		break;
	case NODE_FLOOR:
		fprintf(stderr, "floor");
		break;
	case NODE_CEIL:
		fprintf(stderr, "ceil");
		break;
	case NODE_SQRT:
		fprintf(stderr, "sqrt");
		break;
	case NODE_SIN:
		fprintf(stderr, "sin");
		break;
	case NODE_COS:
		fprintf(stderr, "cos");
		break;
	case NODE_TAN:
		fprintf(stderr, "tan");
		break;
	case NODE_ASIN:
		fprintf(stderr, "asin");
		break;
	case NODE_ACOS:
		fprintf(stderr, "acos");
		break;
	case NODE_ATAN:
		fprintf(stderr, "atan");
		break;
	case NODE_NORETURN:
		fprintf(stderr, "noreturn");
		break;
	default:
		fprintf(stderr, "<invalid>");
		break;
	}

	for (vector< Ref<TreeNode> >::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
	{
		fprintf(stderr, " ");
		(*i)->Print();
	}
}

