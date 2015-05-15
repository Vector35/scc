#ifndef __TREENODE_H__
#define __TREENODE_H__

#include "RefCountObject.h"
#include <string>
#include <vector>
#include <inttypes.h>


enum TreeNodeClass
{
	NODE_NOP,
	NODE_STACK_VAR,
	NODE_GLOBAL_VAR,
	NODE_REG,
	NODE_LARGE_REG,
	NODE_IMMED,
	NODE_BLOCK,
	NODE_FUNC,
	NODE_UNDEFINED,
	NODE_ASSIGN,
	NODE_LOAD,
	NODE_STORE,
	NODE_REF,
	NODE_ADD,
	NODE_SUB,
	NODE_SMUL,
	NODE_UMUL,
	NODE_SDIV,
	NODE_UDIV,
	NODE_SMOD,
	NODE_UMOD,
	NODE_AND,
	NODE_OR,
	NODE_XOR,
	NODE_SHL,
	NODE_SHR,
	NODE_SAR,
	NODE_NEG,
	NODE_NOT,
	NODE_IFTRUE,
	NODE_IFSLT,
	NODE_IFULT,
	NODE_IFSLE,
	NODE_IFULE,
	NODE_IFE,
	NODE_GOTO,
	NODE_CALL,
	NODE_CALLVOID,
	NODE_SYSCALL,
	NODE_SYSCALLVOID,
	NODE_SCONVERT,
	NODE_UCONVERT,
	NODE_RETURN,
	NODE_RETURNVOID,
	NODE_ALLOCA,
	NODE_MEMCPY,
	NODE_MEMSET,
	NODE_STRLEN,
	NODE_RDTSC,
	NODE_RDTSC_LOW,
	NODE_RDTSC_HIGH,
	NODE_PEB,
	NODE_TEB,
	NODE_VARARG,
	NODE_BYTESWAP,
	NODE_BREAKPOINT,
	NODE_POW,
	NODE_FLOOR,
	NODE_CEIL,
	NODE_SQRT,
	NODE_SIN,
	NODE_COS,
	NODE_TAN,
	NODE_ASIN,
	NODE_ACOS,
	NODE_ATAN,
	NODE_PUSH,
	NODE_NORETURN,
	NODE_INPUT
};

enum TreeNodeType
{
	NODETYPE_UNDEFINED,
	NODETYPE_U8,
	NODETYPE_S8,
	NODETYPE_U16,
	NODETYPE_S16,
	NODETYPE_U32,
	NODETYPE_S32,
	NODETYPE_U64,
	NODETYPE_S64,
	NODETYPE_U128,
	NODETYPE_S128,
	NODETYPE_F32,
	NODETYPE_F64
};

class TreeBlock;
class Function;

class TreeNode: public RefCountObject
{
	TreeNodeClass m_class;
	TreeNodeType m_type;
	Ref<TreeBlock> m_block;
	Ref<Function> m_func;
	uint32_t m_reg, m_highReg, m_var;
	uint32_t m_regClass, m_highRegClass;
	int64_t m_immediate;
	std::vector< Ref<TreeNode> > m_children;

	void PrintType() const;

public:
	TreeNode(TreeNodeClass cls);
	TreeNode(const TreeNode& copy);
	TreeNode(const TreeNode* copy, TreeNode* replaceFrom, TreeNode* replaceTo);

	TreeNodeClass GetClass() const { return m_class; }
	TreeNodeType GetType() const { return m_type; }
	uint32_t GetRegister() const { return m_reg; }
	uint32_t GetHighRegister() const { return m_highReg; }
	uint32_t GetRegisterClass() const { return m_regClass; }
	uint32_t GetHighRegisterClass() const { return m_highRegClass; }
	uint32_t GetVariable() const { return m_var; }
	int64_t GetImmediate() const { return m_immediate; }
	void SetType(TreeNodeType type) { m_type = type; }
	void SetRegister(uint32_t reg) { m_reg = reg; }
	void SetHighRegister(uint32_t reg) { m_highReg = reg; }
	void SetRegisterClass(uint32_t reg) { m_regClass = reg; }
	void SetHighRegisterClass(uint32_t reg) { m_highRegClass = reg; }
	void SetVariable(uint32_t var) { m_var = var; }
	void SetImmediate(int64_t immed) { m_immediate = immed; }
	void SetBlock(TreeBlock* block);
	void SetFunction(Function* func);
	TreeBlock* GetBlock() const;
	Function* GetFunction() const;

	void AddChildNode(TreeNode* node) { m_children.push_back(node); }
	void AddChildNodes(const std::vector< Ref<TreeNode> >& nodes) { m_children.insert(m_children.end(), nodes.begin(), nodes.end()); }
	void ReplaceChildNode(size_t i, TreeNode* node) { m_children[i] = node; }
	const std::vector< Ref<TreeNode> >& GetChildNodes() const { return m_children; }
	const Ref<TreeNode>& operator[](size_t i) const { return m_children[i]; }

	static TreeNode* CreateStackVarNode(uint32_t base, int32_t var, int64_t offset, TreeNodeType type);
	static TreeNode* CreateGlobalVarNode(int64_t offset, TreeNodeType type);
	static TreeNode* CreateBlockNode(TreeBlock* block);
	static TreeNode* CreateFunctionNode(Function* func, TreeNodeType type);
	static TreeNode* CreateRegNode(uint32_t reg, uint32_t regClass, TreeNodeType type);
	static TreeNode* CreateLargeRegNode(uint32_t low, uint32_t high, uint32_t lowClass, uint32_t highClass, TreeNodeType type);
	static TreeNode* CreateImmediateNode(int64_t immed, TreeNodeType type);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNodeType type);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a, TreeNode* b);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a, TreeNode* b, TreeNode* c);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNodeType type, TreeNode* a, TreeNode* b, TreeNode* c, TreeNode* d);

	void Print() const;
};


#endif

