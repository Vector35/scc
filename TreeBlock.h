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

#ifndef __TREEBLOCK_H__
#define __TREEBLOCK_H__

#include "TreeNode.h"
#include "ILBlock.h"
#include "Settings.h"
#include "Struct.h"
#include "Variable.h"


struct VariableAssignments;
class Output;

class TreeBlock: public RefCountObject
{
	ILBlock* m_source;
	size_t m_index;
	std::vector< Ref<TreeNode> > m_nodes;
	size_t m_errors;
	TreeNodeType m_ptrType;

	bool GetMemberVariableAndOffset(ILParameter& operand, Variable*& var, size_t& offset, Type*& type);
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
	const std::vector< Ref<TreeNode> >& GetNodes() const { return m_nodes; }

	bool GenerateFromILBlock(ILBlock* il, std::vector< Ref<TreeBlock> >& blocks, const VariableAssignments& vars,
		const Settings& settings, Output* output);

	void Print() const;
};


#endif

