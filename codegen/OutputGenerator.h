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

#ifndef __OUTPUTGENERATOR_H__
#define __OUTPUTGENERATOR_H__

#include "ParserState.h"
#include "CodeBlock.h"
#include "Match.h"
#include <string>


class OutputGenerator
{
	struct MatchNodeInfo
	{
		std::string prefix;
		TreeNode* node;
	};

	enum MatchVariableType
	{
		MATCHVAR_IMM,
		MATCHVAR_REG,
		MATCHVAR_LARGE_REG,
		MATCHVAR_STACK,
		MATCHVAR_GLOBAL,
		MATCHVAR_BLOCK,
		MATCHVAR_FUNC,
		MATCHVAR_INPUT
	};

	ParserState* m_parser;
	std::string m_output;
	int m_errors;

	int m_indentLevel;

	std::string m_className;

	void Indent();
	void BeginBlock();
	void WriteLine(const char* fmt, ...);
	void WriteUnindented(const char* fmt, ...);
	void EndBlock();
	void EndBlockSemicolon();
	void WriteEncodingParams(Encoding* encoding, const CodeToken& token, std::map<std::string, MatchVariableType> vars);
	void WriteCodeBlock(CodeBlock* code, std::map<std::string, MatchVariableType> vars = std::map<std::string, MatchVariableType>());

	std::string GenerateTypeMatchCode(const std::string& prefix, TreeNode* node);
	std::string GenerateMatchForNode(Match* match, const std::string& prefix, TreeNode* node);
	void GenerateMatchCode(Match* match);
	void GenerateOutputCode(Match* match);

public:
	OutputGenerator(ParserState* parser);

	bool Generate(std::string& output);
};


#endif

