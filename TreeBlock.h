#ifndef __TREEBLOCK_H__
#define __TREEBLOCK_H__

#include "ILBlock.h"
#include "Settings.h"
#include "Struct.h"
#include "TreeNode.h"
#include "Variable.h"


struct VariableAssignments;
class Output;

class TreeBlock : public RefCountObject
{
	ILBlock* m_source;
	size_t m_index;
	std::vector<Ref<TreeNode>> m_nodes;
	size_t m_errors;
	TreeNodeType m_ptrType;

	bool GetMemberVariableAndOffset(
	    ILParameter& operand, Variable*& var, size_t& offset, Type*& type);
	TreeNodeType GetPtrType();
	TreeNodeType VariableTypeToNodeType(Type* type);
	TreeNodeType OperandToNodeType(ILParameter& operand);
	TreeNode* OperandToNode(const VariableAssignments& vars, ILParameter& operand);
	TreeNode* OperandRefToNode(const VariableAssignments& vars, ILParameter& operand);
	void StoreToOperand(const VariableAssignments& vars, ILParameter& dest, TreeNode* value);
	TreeNode* ConstantMultiplyNode(TreeNode* val, uint64_t mult);
	TreeNode* ConstantDivideNode(TreeNode* val, uint64_t div);
	TreeNodeType DoubleTypeSize(TreeNodeType type);

 public:
	TreeBlock(ILBlock* source, size_t idx);

	ILBlock* GetSource() const { return m_source; }
	size_t GetIndex() const { return m_index; }

	void AddNode(TreeNode* node);
	const std::vector<Ref<TreeNode>>& GetNodes() const { return m_nodes; }

	bool GenerateFromILBlock(ILBlock* il, std::vector<Ref<TreeBlock>>& blocks,
	    const VariableAssignments& vars, const Settings& settings, Output* output);

	void Print() const;
};


#endif
