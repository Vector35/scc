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

#include "TreeNode.h"

using namespace std;


TreeNode::TreeNode(TreeNodeClass cls) : m_class(cls), m_sizeFlags(REG_MATCH_ALL) {}


TreeNode::TreeNode(const TreeNode& copy)
{
	m_class = copy.m_class;
	m_name = copy.m_name;
	m_typeName = copy.m_typeName;
	m_sizeFlags = copy.m_sizeFlags;
	for (vector<Ref<TreeNode>>::const_iterator i = copy.m_children.begin();
	     i != copy.m_children.end(); ++i)
		m_children.push_back(new TreeNode(**i));
}


TreeNode* TreeNode::CreateRegNode(
    const std::string& name, const std::string& typeName, uint32_t sizeFlags)
{
	TreeNode* result = new TreeNode(NODE_REG);
	result->SetName(name);
	result->SetTypeName(typeName);
	result->SetSizeFlags(sizeFlags);
	return result;
}


TreeNode* TreeNode::CreateLoadNode(TreeNode* src, uint32_t sizeFlags)
{
	TreeNode* result = new TreeNode(NODE_LOAD);
	result->SetSizeFlags(sizeFlags);
	result->AddChildNode(src);
	return result;
}


TreeNode* TreeNode::CreateStoreNode(TreeNode* dest, TreeNode* val, uint32_t sizeFlags)
{
	TreeNode* result = new TreeNode(NODE_STORE);
	result->SetSizeFlags(sizeFlags);
	result->AddChildNode(dest);
	result->AddChildNode(val);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls)
{
	TreeNode* result = new TreeNode(cls);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNode* a)
{
	TreeNode* result = new TreeNode(cls);
	result->AddChildNode(a);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNode* a, TreeNode* b)
{
	TreeNode* result = new TreeNode(cls);
	result->AddChildNode(a);
	result->AddChildNode(b);
	return result;
}


TreeNode* TreeNode::CreateNode(TreeNodeClass cls, TreeNode* a, TreeNode* b, TreeNode* c)
{
	TreeNode* result = new TreeNode(cls);
	result->AddChildNode(a);
	result->AddChildNode(b);
	result->AddChildNode(c);
	return result;
}


TreeNode* TreeNode::CreateNode(
    TreeNodeClass cls, TreeNode* a, TreeNode* b, TreeNode* c, TreeNode* d)
{
	TreeNode* result = new TreeNode(cls);
	result->AddChildNode(a);
	result->AddChildNode(b);
	result->AddChildNode(c);
	result->AddChildNode(d);
	return result;
}
