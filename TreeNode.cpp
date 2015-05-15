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


TreeNode::TreeNode(const TreeNode* copy, TreeNode* replaceFrom, TreeNode* replaceTo)
{
	if (copy == replaceFrom)
		copy = replaceTo;

	m_type = copy->m_type;
	m_class = copy->m_class;
	m_block = copy->m_block;
	m_func = copy->m_func;
	m_reg = copy->m_reg;
	m_highReg = copy->m_highReg;
	m_regClass = copy->m_regClass;
	m_highRegClass = copy->m_highRegClass;
	m_var = copy->m_var;
	m_immediate = copy->m_immediate;
	for (vector< Ref<TreeNode> >::const_iterator i = copy->m_children.begin(); i != copy->m_children.end(); ++i)
		m_children.push_back(new TreeNode(*i, replaceFrom, replaceTo));
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


TreeNode* TreeNode::CreateFunctionNode(Function* func, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_FUNC);
	result->SetType(type);
	result->SetFunction(func);
	return result;
}


TreeNode* TreeNode::CreateRegNode(uint32_t reg, uint32_t regClass, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_REG);
	result->SetType(type);
	result->SetRegister(reg);
	result->SetRegisterClass(regClass);
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


TreeNode* TreeNode::CreateImmediateNode(int64_t immed, TreeNodeType type)
{
	TreeNode* result = new TreeNode(NODE_IMMED);
	result->SetType(type);
	result->SetImmediate(immed);
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
		PrintType();
		break;
	case NODE_SUB:
		fprintf(stderr, "sub");
		PrintType();
		break;
	case NODE_SMUL:
		fprintf(stderr, "smul");
		PrintType();
		break;
	case NODE_UMUL:
		fprintf(stderr, "umul");
		PrintType();
		break;
	case NODE_SDIV:
		fprintf(stderr, "sdiv");
		PrintType();
		break;
	case NODE_UDIV:
		fprintf(stderr, "udiv");
		PrintType();
		break;
	case NODE_SMOD:
		fprintf(stderr, "smod");
		PrintType();
		break;
	case NODE_UMOD:
		fprintf(stderr, "umod");
		PrintType();
		break;
	case NODE_AND:
		fprintf(stderr, "and");
		PrintType();
		break;
	case NODE_OR:
		fprintf(stderr, "or");
		PrintType();
		break;
	case NODE_XOR:
		fprintf(stderr, "xor");
		PrintType();
		break;
	case NODE_SHL:
		fprintf(stderr, "shl");
		PrintType();
		break;
	case NODE_SHR:
		fprintf(stderr, "shr");
		PrintType();
		break;
	case NODE_SAR:
		fprintf(stderr, "sar");
		PrintType();
		break;
	case NODE_NEG:
		fprintf(stderr, "neg");
		PrintType();
		break;
	case NODE_NOT:
		fprintf(stderr, "not");
		PrintType();
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
		fprintf(stderr, "call");
		PrintType();
		break;
	case NODE_CALLVOID:
		fprintf(stderr, "callvoid");
		break;
	case NODE_SYSCALL:
		fprintf(stderr, "syscall");
		PrintType();
		break;
	case NODE_SYSCALLVOID:
		fprintf(stderr, "syscallvoid");
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
	case NODE_ALLOCA:
		fprintf(stderr, "alloca");
		PrintType();
		break;
	case NODE_MEMCPY:
		fprintf(stderr, "memcpy");
		break;
	case NODE_MEMSET:
		fprintf(stderr, "memset");
		break;
	case NODE_STRLEN:
		fprintf(stderr, "strlen");
		PrintType();
		break;
	case NODE_RDTSC:
		fprintf(stderr, "rdtsc");
		PrintType();
		break;
	case NODE_RDTSC_LOW:
		fprintf(stderr, "rdtsc_low");
		PrintType();
		break;
	case NODE_RDTSC_HIGH:
		fprintf(stderr, "rdtsc_high");
		PrintType();
		break;
	case NODE_PEB:
		fprintf(stderr, "peb");
		break;
	case NODE_TEB:
		fprintf(stderr, "teb");
		break;
	case NODE_VARARG:
		fprintf(stderr, "vararg");
		PrintType();
		break;
	case NODE_BYTESWAP:
		fprintf(stderr, "byteswap");
		PrintType();
		break;
	case NODE_BREAKPOINT:
		fprintf(stderr, "breakpoint");
		break;
	case NODE_POW:
		fprintf(stderr, "pow");
		PrintType();
		break;
	case NODE_FLOOR:
		fprintf(stderr, "floor");
		PrintType();
		break;
	case NODE_CEIL:
		fprintf(stderr, "ceil");
		PrintType();
		break;
	case NODE_SQRT:
		fprintf(stderr, "sqrt");
		PrintType();
		break;
	case NODE_SIN:
		fprintf(stderr, "sin");
		PrintType();
		break;
	case NODE_COS:
		fprintf(stderr, "cos");
		PrintType();
		break;
	case NODE_TAN:
		fprintf(stderr, "tan");
		PrintType();
		break;
	case NODE_ASIN:
		fprintf(stderr, "asin");
		PrintType();
		break;
	case NODE_ACOS:
		fprintf(stderr, "acos");
		PrintType();
		break;
	case NODE_ATAN:
		fprintf(stderr, "atan");
		PrintType();
		break;
	case NODE_PUSH:
		fprintf(stderr, "push");
		break;
	case NODE_NORETURN:
		fprintf(stderr, "noreturn");
		break;
	case NODE_INPUT:
		fprintf(stderr, "input");
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

