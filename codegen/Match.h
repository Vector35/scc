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

#ifndef __MATCH_H__
#define __MATCH_H__

#include "RefCountObject.h"
#include <vector>
#include "TreeNode.h"
#include "CodeBlock.h"


class Match: public RefCountObject
{
	Ref<TreeNode> m_match;
	Ref<TreeNode> m_result;
	std::vector< Ref<TreeNode> > m_temps;
	Ref<CodeBlock> m_code;

public:
	Match(TreeNode* match, TreeNode* result, const std::vector< Ref<TreeNode> >& temp, CodeBlock* code);

	TreeNode* GetMatch() const { return m_match; }
	TreeNode* GetResult() const { return m_result; }
	const std::vector< Ref<TreeNode> >& GetTemps() const { return m_temps; }
	CodeBlock* GetCode() const { return m_code; }
};


#endif

