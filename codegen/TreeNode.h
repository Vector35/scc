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

#ifndef __TREENODE_H__
#define __TREENODE_H__

#include "RefCountObject.h"
#include <string>
#include <vector>
#include <inttypes.h>

#define REG_MATCH_U8   1
#define REG_MATCH_S8   2
#define REG_MATCH_U16  4
#define REG_MATCH_S16  8
#define REG_MATCH_U32  0x10
#define REG_MATCH_S32  0x20
#define REG_MATCH_U64  0x40
#define REG_MATCH_S64  0x80
#define REG_MATCH_U128 0x100
#define REG_MATCH_S128 0x200
#define REG_MATCH_F32  0x400
#define REG_MATCH_F64  0x800
#define REG_MATCH_ALL  0xfff


enum TreeNodeClass
{
	NODE_REG,
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
	NODE_PUSH
};

class TreeNode: public RefCountObject
{
	TreeNodeClass m_class;
	std::string m_name, m_typeName;
	uint32_t m_sizeFlags;
	std::vector< Ref<TreeNode> > m_children;

public:
	TreeNode(TreeNodeClass cls);
	TreeNode(const TreeNode& copy);

	void SetName(const std::string& name) { m_name = name; }
	void SetTypeName(const std::string& type) { m_typeName = type; }
	void SetSizeFlags(uint32_t flags) { m_sizeFlags = flags; }
	TreeNodeClass GetClass() const { return m_class; }
	const std::string& GetName() const { return m_name; }
	const std::string& GetTypeName() const { return m_typeName; }
	const uint32_t GetSizeFlags() const { return m_sizeFlags; }
	bool MatchesSize(uint32_t size) const { return (m_sizeFlags & size) != 0; }

	void AddChildNode(TreeNode* node) { m_children.push_back(node); }
	void AddChildNodes(const std::vector< Ref<TreeNode> >& nodes) { m_children.insert(m_children.end(), nodes.begin(), nodes.end()); }
	const std::vector< Ref<TreeNode> >& GetChildNodes() const { return m_children; }

	static TreeNode* CreateRegNode(const std::string& name, const std::string& typeName, uint32_t sizeFlags);
	static TreeNode* CreateLoadNode(TreeNode* src, uint32_t sizeFlags);
	static TreeNode* CreateStoreNode(TreeNode* dest, TreeNode* val, uint32_t sizeFlags);
	static TreeNode* CreateNode(TreeNodeClass cls);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNode* a);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNode* a, TreeNode* b);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNode* a, TreeNode* b, TreeNode* c);
	static TreeNode* CreateNode(TreeNodeClass cls, TreeNode* a, TreeNode* b, TreeNode* c, TreeNode* d);
};


#endif

