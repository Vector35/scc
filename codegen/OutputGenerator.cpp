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

#include "OutputGenerator.h"
#include <queue>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

using namespace std;


OutputGenerator::OutputGenerator(ParserState* parser): m_parser(parser), m_errors(0), m_indentLevel(0)
{
	m_className = m_parser->GetArchName() + "CodeGen";
}


void OutputGenerator::Indent()
{
	for (int i = 0; i < m_indentLevel; i++)
		m_output += "\t";
}


void OutputGenerator::BeginBlock()
{
	Indent();
	m_output += "{\n";
	m_indentLevel++;
}


void OutputGenerator::WriteLine(const char* fmt, ...)
{
	Indent();

	char* str;
	va_list args;
	va_start(args, fmt);
	vasprintf(&str, fmt, args);
	va_end(args);

	m_output += str;
	free(str);

	m_output += "\n";
}


void OutputGenerator::WriteUnindented(const char* fmt, ...)
{
	char* str;
	va_list args;
	va_start(args, fmt);
	vasprintf(&str, fmt, args);
	va_end(args);

	m_output += str;
	free(str);

	m_output += "\n";
}


void OutputGenerator::EndBlock()
{
	m_indentLevel--;
	Indent();
	m_output += "}\n";
}


void OutputGenerator::EndBlockSemicolon()
{
	m_indentLevel--;
	Indent();
	m_output += "};\n";
}


void OutputGenerator::WriteEncodingParams(Encoding* encoding, const CodeToken& token, map<string, MatchVariableType>& vars)
{
	map< string, Ref<CodeBlock> >::const_iterator operand;
	bool first = true;
	for (vector<EncodingField>::const_iterator i = encoding->GetFields().begin(); i != encoding->GetFields().end(); ++i)
	{
		switch (i->type)
		{
		case FIELD_NORMAL:
		case FIELD_DEFAULT_VALUE:
			operand = token.operands.find(i->name);
			if (operand == token.operands.end())
			{
				map<string, MatchVariableType>::iterator var = vars.find(i->name);

				if ((var == vars.end()) || ((var->second != MATCHVAR_IMM) && (var->second != MATCHVAR_REG)))
				{
					// No operand for this field
					if (i->type == FIELD_DEFAULT_VALUE)
					{
						// Field has default value, use it
						if (first)
						{
							first = false;
							if (encoding->GetWidth() == 64)
								WriteLine("0x%" PRIx64 "LL", i->value);
							else
								WriteLine("0x%" PRIx32, (uint32_t)i->value);
						}
						else
						{
							if (encoding->GetWidth() == 64)
								WriteLine(", 0x%" PRIx64 "LL", i->value);
							else
								WriteLine(", 0x%" PRIx32, (uint32_t)i->value);
						}
					}
					else
					{
						// Field not defined, use zero as a default
						// TODO: Should use these fields to satisfy blacklist restrictions
						if (first)
						{
							first = false;
							WriteLine("0");
						}
						else
						{
							WriteLine(", 0");
						}
					}
				}
				else
				{
					// Register or immediate operand with this name is present, use the value of the operand automatically
					if (first)
					{
						first = false;
						WriteLine("%s", i->name.c_str());
					}
					else
					{
						WriteLine(", %s", i->name.c_str());
					}
				}
			}
			else
			{
				// Encoding field specified directly, output code block for the field
				if (first)
					first = false;
				else
					WriteLine(",");
				WriteCodeBlock(operand->second, vars);
			}
			break;

		default:
			break;
		}
	}
}


void OutputGenerator::WriteCodeBlock(CodeBlock* code, map<string, MatchVariableType> vars, CodeBlockType codeType)
{
	string result;
	map<string, MatchVariableType>::iterator var;
	Encoding* encoding;
	char offsetStr[32];
	bool first = true;

	for (vector<CodeToken>::const_iterator i = code->GetTokens().begin(); i != code->GetTokens().end(); ++i)
	{
		if (first)
		{
			WriteLine("#line %d \"%s\"", i->line, i->fileName.c_str());
			first = false;
		}

		switch (i->type)
		{
		case TOKEN_TEXT:
			if (i->name == "{")
			{
				if (result.size() != 0)
				{
					WriteLine("%s", result.c_str());
					result = "";
				}
				BeginBlock();
				first = true;
			}
			else if (i->name == "}")
			{
				if (result.size() != 0)
				{
					WriteLine("%s", result.c_str());
					result = "";
				}
				EndBlock();
				first = true;
			}
			else if (i->name == ";")
			{
				result += ";";
				WriteLine("%s", result.c_str());
				result = "";
				first = true;
			}
			else
			{
				result += i->name;
				result += " ";
			}
			break;
		case TOKEN_INSTR_START:
			switch (codeType)
			{
			case CODEBLOCK_UPDATE:
				result += "replacement.push_back(";
				break;
			case CODEBLOCK_ARCH:
				result += "code.push_back(";
				break;
			default:
				result += "out->AddInstruction(";
				break;
			}
			result += m_parser->GetArchName();
			result += "_";
			result += i->name;
			result += "(";
			break;
		case TOKEN_INSTR_END:
			result += "))";
			break;
		case TOKEN_VAR:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_IMM:
			case MATCHVAR_REG:
			case MATCHVAR_BLOCK:
			case MATCHVAR_FUNC:
			case MATCHVAR_INPUT:
				result += i->name;
				result += " ";
				break;
			case MATCHVAR_LARGE_REG:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: large register variable '%s' used without :low or :high\n",
					i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			case MATCHVAR_STACK:
				result += i->name + "_base, " + i->name + "_var, " + i->name + "_offset, " + i->name + "_scratch ";
				break;
			case MATCHVAR_GLOBAL:
				result += "m_globalBaseReg, " + i->name + "_offset, " + i->name + "_scratch ";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: invalid match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_LOW:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_LARGE_REG:
				result += i->name + "_low ";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a large register\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_HIGH:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_LARGE_REG:
				result += i->name + "_high ";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a large register\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_INT:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}

			sprintf(offsetStr, "%+d", i->offset);

			switch (var->second)
			{
			case MATCHVAR_STACK:
				result += i->name + "_base, " + i->name + "_var, " + i->name + "_offset" + offsetStr + ", " + i->name + "_scratch ";
				break;
			case MATCHVAR_GLOBAL:
				result += "m_globalBaseReg, " + i->name + "_offset" + offsetStr + ", " + i->name + "_scratch ";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a stack or global variable\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_BASE:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_VAR_COMPONENTS:
				result += i->name + "_base";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a stack or global variable\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_OFFSET:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_VAR_COMPONENTS:
				result += i->name + "_offset";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a stack or global variable\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_TEMP:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_VAR_COMPONENTS:
				result += i->name + "_temp";
				break;
			default:
				result += "0 ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a stack or global variable\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_FUNCTION:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_FUNC_AND_BLOCK:
				result += i->name + "_func";
				break;
			default:
				result += "NULL ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a function\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_VAR_BLOCK:
			var = vars.find(i->name);
			if (var == vars.end())
			{
				result += "0 ";
				fprintf(stderr, "%s:%d: error: undefined match variable '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (var->second)
			{
			case MATCHVAR_FUNC_AND_BLOCK:
				result += i->name + "_block";
				break;
			default:
				result += "NULL ";
				fprintf(stderr, "%s:%d: error: variable '%s' is not a function\n", i->fileName.c_str(),
					i->line, i->name.c_str());
				m_errors++;
				break;
			}
			break;
		case TOKEN_INSTR_ENCODING:
			encoding = m_parser->GetEncoding(i->name);
			if (!encoding)
			{
				fprintf(stderr, "%s:%d: error: invalid encoding '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			switch (encoding->GetWidth())
			{
			case 8:
				result += string("out->WriteUInt8(ENCODING_VALUE_") + i->name + "(";
				break;
			case 16:
				result += string("out->WriteUInt16(ENCODING_VALUE_") + i->name + "(";
				break;
			case 32:
				result += string("out->WriteUInt32(ENCODING_VALUE_") + i->name + "(";
				break;
			case 64:
				result += string("out->WriteUInt64(ENCODING_VALUE_") + i->name + "(";
				break;
			default:
				result += string("(ENCODING_VALUE_") + i->name + "(";
				fprintf(stderr, "%s:%d: error: encoding '%s' has invalid size %d\n", i->fileName.c_str(),
					i->line, i->name.c_str(), encoding->GetWidth());
				m_errors++;
				break;
			}
			WriteLine("%s", result.c_str());
			result = "";
			m_indentLevel++;
			WriteEncodingParams(encoding, *i, vars);
			WriteLine("))");
			m_indentLevel--;
			break;
		case TOKEN_INSTR_ENCODING_VALUE:
			encoding = m_parser->GetEncoding(i->name);
			if (!encoding)
			{
				fprintf(stderr, "%s:%d: error: invalid encoding '%s'\n", i->fileName.c_str(), i->line, i->name.c_str());
				m_errors++;
				break;
			}
			result += string("ENCODING_VALUE_") + i->name + "(";
			WriteLine("%s", result.c_str());
			result = "";
			m_indentLevel++;
			WriteEncodingParams(encoding, *i, vars);
			WriteLine(")");
			m_indentLevel--;
			break;
		default:
			fprintf(stderr, "%s:%d: error: invalid token in code block\n", i->fileName.c_str(), i->line);
			m_errors++;
			break;
		}
	}

	WriteLine("%s", result.c_str());
}


string OutputGenerator::GenerateTypeMatchCode(const string& prefix, TreeNode* node)
{
	vector<string> types;

	if (node->GetSizeFlags() == 0)
		return "false";
	if (node->GetSizeFlags() == REG_MATCH_ALL)
		return "";

	if (node->GetSizeFlags() & REG_MATCH_U8)
		types.push_back(prefix + "->GetType() == NODETYPE_U8");
	if (node->GetSizeFlags() & REG_MATCH_S8)
		types.push_back(prefix + "->GetType() == NODETYPE_S8");
	if (node->GetSizeFlags() & REG_MATCH_U16)
		types.push_back(prefix + "->GetType() == NODETYPE_U16");
	if (node->GetSizeFlags() & REG_MATCH_S16)
		types.push_back(prefix + "->GetType() == NODETYPE_S16");
	if (node->GetSizeFlags() & REG_MATCH_U32)
		types.push_back(prefix + "->GetType() == NODETYPE_U32");
	if (node->GetSizeFlags() & REG_MATCH_S32)
		types.push_back(prefix + "->GetType() == NODETYPE_S32");
	if (node->GetSizeFlags() & REG_MATCH_U64)
		types.push_back(prefix + "->GetType() == NODETYPE_U64");
	if (node->GetSizeFlags() & REG_MATCH_S64)
		types.push_back(prefix + "->GetType() == NODETYPE_S64");
	if (node->GetSizeFlags() & REG_MATCH_U128)
		types.push_back(prefix + "->GetType() == NODETYPE_U128");
	if (node->GetSizeFlags() & REG_MATCH_S128)
		types.push_back(prefix + "->GetType() == NODETYPE_S128");
	if (node->GetSizeFlags() & REG_MATCH_F32)
		types.push_back(prefix + "->GetType() == NODETYPE_F32");
	if (node->GetSizeFlags() & REG_MATCH_F64)
		types.push_back(prefix + "->GetType() == NODETYPE_F64");

	string result = " && (";
	for (vector<string>::iterator i = types.begin(); i != types.end(); ++i)
	{
		if (i != types.begin())
			result += " || ";
		result += string("(") + *i + ")";
	}
	result += ")";
	return result;
}


string OutputGenerator::GenerateMatchForNode(Match* match, const string& prefix, TreeNode* node)
{
	switch (node->GetClass())
	{
	case NODE_REG:
		if (node->GetTypeName() == "BLOCK")
			return prefix + "->GetClass() == NODE_BLOCK";
		else if (node->GetTypeName() == "FUNCTION")
			return prefix + "->GetClass() == NODE_FUNC";
		else if (node->GetTypeName() == "IMM")
			return string("(") + prefix + "->GetClass() == NODE_IMMED)" + GenerateTypeMatchCode(prefix, node);
		else if (node->GetTypeName() == "STACKVAR")
			return string("(") + prefix + "->GetClass() == NODE_STACK_VAR)" + GenerateTypeMatchCode(prefix, node);
		else if (node->GetTypeName() == "GLOBALVAR")
			return string("(") + prefix + "->GetClass() == NODE_GLOBAL_VAR)" + GenerateTypeMatchCode(prefix, node);
		else if (node->GetTypeName() == "INPUT")
			return prefix + "->GetClass() == NODE_INPUT";
		else if (m_parser->IsImmediateClass(node->GetTypeName()))
		{
			return string("(") + prefix + "->GetClass() == NODE_IMMED) && (MatchImmClass_" +
				node->GetTypeName() + "(" + prefix + "->GetImmediate()))" + GenerateTypeMatchCode(prefix, node);
		}
		else if (m_parser->IsRegisterClass(node->GetTypeName()))
		{
			RegisterClass* reg = m_parser->GetRegisterClass(node->GetTypeName());
			if (!reg)
			{
				fprintf(stderr, "%s:%d: error: invalid register class '%s'\n", match->GetFileName().c_str(),
					match->GetLineNumber(), node->GetTypeName().c_str());
				m_errors++;
				return "false";
			}

			if (reg->GetClassType() == REGCLASS_NORMAL)
			{
				return string("(") + prefix + "->GetClass() == NODE_REG) && " +
					"IsRegisterClassCompatible(" + prefix + "->GetRegisterClass(), " + reg->GetRegisterClassName() + ") " +
					GenerateTypeMatchCode(prefix, node);
			}
			else if (reg->GetClassType() == REGCLASS_LARGE)
			{
				return string("(") + prefix + "->GetClass() == NODE_LARGE_REG) && " +
					"IsRegisterClassCompatible(" + prefix + "->GetRegisterClass(), " + reg->GetLowRegisterClassName() + ") && " +
					"IsRegisterClassCompatible(" + prefix + "->GetHighRegisterClass(), " + reg->GetHighRegisterClassName() + ") " +
					GenerateTypeMatchCode(prefix, node);
			}
			else
			{
				fprintf(stderr, "%s:%d: error: invalid use of temporary register class '%s'\n", match->GetFileName().c_str(),
					match->GetLineNumber(), node->GetTypeName().c_str());
				m_errors++;
				return "false";
			}
		}

		fprintf(stderr, "%s:%d: error: invalid match type '%s'\n", match->GetFileName().c_str(),
			match->GetLineNumber(), node->GetTypeName().c_str());
		m_errors++;
		return "false";

	case NODE_ASSIGN:
		return prefix + "->GetClass() == NODE_ASSIGN";
	case NODE_LOAD:
		return string("(") + prefix + "->GetClass() == NODE_LOAD)" + GenerateTypeMatchCode(prefix, node);
	case NODE_STORE:
		return string("(") + prefix + "->GetClass() == NODE_STORE)" + GenerateTypeMatchCode(prefix, node);
	case NODE_REF:
		return prefix + "->GetClass() == NODE_REF";
	case NODE_ADD:
		return string("(") + prefix + "->GetClass() == NODE_ADD)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SUB:
		return string("(") + prefix + "->GetClass() == NODE_SUB)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SMUL:
		return string("(") + prefix + "->GetClass() == NODE_SMUL)" + GenerateTypeMatchCode(prefix, node);
	case NODE_UMUL:
		return string("(") + prefix + "->GetClass() == NODE_UMUL)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SDIV:
		return string("(") + prefix + "->GetClass() == NODE_SDIV)" + GenerateTypeMatchCode(prefix, node);
	case NODE_UDIV:
		return string("(") + prefix + "->GetClass() == NODE_UDIV)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SMOD:
		return string("(") + prefix + "->GetClass() == NODE_SMOD)" + GenerateTypeMatchCode(prefix, node);
	case NODE_UMOD:
		return string("(") + prefix + "->GetClass() == NODE_UMOD)" + GenerateTypeMatchCode(prefix, node);
	case NODE_AND:
		return string("(") + prefix + "->GetClass() == NODE_AND)" + GenerateTypeMatchCode(prefix, node);
	case NODE_OR:
		return string("(") + prefix + "->GetClass() == NODE_OR)" + GenerateTypeMatchCode(prefix, node);
	case NODE_XOR:
		return string("(") + prefix + "->GetClass() == NODE_XOR)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SHL:
		return string("(") + prefix + "->GetClass() == NODE_SHL)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SHR:
		return string("(") + prefix + "->GetClass() == NODE_SHR)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SAR:
		return string("(") + prefix + "->GetClass() == NODE_SAR)" + GenerateTypeMatchCode(prefix, node);
	case NODE_NEG:
		return string("(") + prefix + "->GetClass() == NODE_NEG)" + GenerateTypeMatchCode(prefix, node);
	case NODE_NOT:
		return string("(") + prefix + "->GetClass() == NODE_NOT)" + GenerateTypeMatchCode(prefix, node);
	case NODE_IFTRUE:
		return prefix + "->GetClass() == NODE_IFTRUE";
	case NODE_IFSLT:
		return prefix + "->GetClass() == NODE_IFSLT";
	case NODE_IFULT:
		return prefix + "->GetClass() == NODE_IFULT";
	case NODE_IFSLE:
		return prefix + "->GetClass() == NODE_IFSLE";
	case NODE_IFULE:
		return prefix + "->GetClass() == NODE_IFULE";
	case NODE_IFE:
		return prefix + "->GetClass() == NODE_IFE";
	case NODE_GOTO:
		return prefix + "->GetClass() == NODE_GOTO";
	case NODE_CALL:
		return prefix + "->GetClass() == NODE_CALL";
	case NODE_CALLVOID:
		return prefix + "->GetClass() == NODE_CALLVOID";
	case NODE_SYSCALL:
		return prefix + "->GetClass() == NODE_SYSCALL";
	case NODE_SYSCALLVOID:
		return prefix + "->GetClass() == NODE_SYSCALLVOID";
	case NODE_SCONVERT:
		return string("(") + prefix + "->GetClass() == NODE_SCONVERT)" + GenerateTypeMatchCode(prefix, node);
	case NODE_UCONVERT:
		return string("(") + prefix + "->GetClass() == NODE_UCONVERT)" + GenerateTypeMatchCode(prefix, node);
	case NODE_RETURN:
		return prefix + "->GetClass() == NODE_RETURN";
	case NODE_RETURNVOID:
		return prefix + "->GetClass() == NODE_RETURNVOID";
	case NODE_ALLOCA:
		return prefix + "->GetClass() == NODE_ALLOCA";
	case NODE_MEMCPY:
		return prefix + "->GetClass() == NODE_MEMCPY";
	case NODE_MEMSET:
		return prefix + "->GetClass() == NODE_MEMSET";
	case NODE_STRLEN:
		return string("(") + prefix + "->GetClass() == NODE_STRLEN)" + GenerateTypeMatchCode(prefix, node);
	case NODE_RDTSC:
		return prefix + "->GetClass() == NODE_RDTSC";
	case NODE_RDTSC_LOW:
		return prefix + "->GetClass() == NODE_RDTSC_LOW";
	case NODE_RDTSC_HIGH:
		return prefix + "->GetClass() == NODE_RDTSC_HIGH";
	case NODE_VARARG:
		return prefix + "->GetClass() == NODE_VARARG";
	case NODE_BYTESWAP:
		return string("(") + prefix + "->GetClass() == NODE_BYTESWAP)" + GenerateTypeMatchCode(prefix, node);
	case NODE_BREAKPOINT:
		return prefix + "->GetClass() == NODE_BREAKPOINT";
	case NODE_POW:
		return string("(") + prefix + "->GetClass() == NODE_POW)" + GenerateTypeMatchCode(prefix, node);
	case NODE_FLOOR:
		return string("(") + prefix + "->GetClass() == NODE_FLOOR)" + GenerateTypeMatchCode(prefix, node);
	case NODE_CEIL:
		return string("(") + prefix + "->GetClass() == NODE_CEIL)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SQRT:
		return string("(") + prefix + "->GetClass() == NODE_SQRT)" + GenerateTypeMatchCode(prefix, node);
	case NODE_SIN:
		return string("(") + prefix + "->GetClass() == NODE_SIN)" + GenerateTypeMatchCode(prefix, node);
	case NODE_COS:
		return string("(") + prefix + "->GetClass() == NODE_COS)" + GenerateTypeMatchCode(prefix, node);
	case NODE_TAN:
		return string("(") + prefix + "->GetClass() == NODE_TAN)" + GenerateTypeMatchCode(prefix, node);
	case NODE_ASIN:
		return string("(") + prefix + "->GetClass() == NODE_ASIN)" + GenerateTypeMatchCode(prefix, node);
	case NODE_ACOS:
		return string("(") + prefix + "->GetClass() == NODE_ACOS)" + GenerateTypeMatchCode(prefix, node);
	case NODE_ATAN:
		return string("(") + prefix + "->GetClass() == NODE_ATAN)" + GenerateTypeMatchCode(prefix, node);
	case NODE_PUSH:
		return string("(") + prefix + "->GetClass() == NODE_PUSH)" + GenerateTypeMatchCode(prefix, node);
	default:
		fprintf(stderr, "%s:%d: error: invalid node type in match\n", match->GetFileName().c_str(), match->GetLineNumber());
		m_errors++;
		return "false";
	}
}


void OutputGenerator::GenerateMatchCode(Match* match)
{
	bool first = true;

	queue<MatchNodeInfo> nodes;
	MatchNodeInfo info;
	info.prefix = "node";
	info.node = match->GetMatch();
	nodes.push(info);

	while (nodes.size() != 0)
	{
		info = nodes.front();
		nodes.pop();

		string code = GenerateMatchForNode(match, info.prefix, info.node);
		if (first)
			code = string("if ((") + code + string(")");
		else
			code = string("\t&& (") + code + string(")");
		first = false;
		WriteLine("%s", code.c_str());

		if (info.node->GetClass() == NODE_SYSCALL)
			WriteLine("\t&& (%s->GetChildNodes().size() == %d)", info.prefix.c_str(), (int)info.node->GetChildNodes().size());

		for (size_t i = 0; i < info.node->GetChildNodes().size(); i++)
		{
			MatchNodeInfo child;
			char indexStr[32];
			sprintf(indexStr, "%d", (int)i);
			child.prefix = string("(*") + info.prefix + ")[" + indexStr + "]";
			child.node = info.node->GetChildNodes()[i];
			nodes.push(child);
		}
	}

	WriteLine("\t)");
	BeginBlock();
		// TODO: Implement override of default cost calculation, and use actual instruction sizes
		// on architectures with variable width instructions
		size_t cost = 0;
		for (vector<CodeToken>::const_iterator i = match->GetCode()->GetTokens().begin();
			i != match->GetCode()->GetTokens().end(); ++i)
		{
			if (i->type == TOKEN_INSTR_START)
				cost++;
		}

		WriteLine("MatchState nextState;");
		WriteLine("nextState = state;");
		WriteLine("nextState.cost += %d;", (int)cost);

		WriteLine("vector<uint32_t> temps;");
		for (vector< Ref<TreeNode> >::const_iterator i = match->GetTemps().begin(); i != match->GetTemps().end(); ++i)
		{
			RegisterClass* reg = m_parser->GetRegisterClass((*i)->GetTypeName());
			if (!reg)
			{
				fprintf(stderr, "%s:%d: error: invalid register class '%s' for '%s'\n", match->GetFileName().c_str(),
					match->GetLineNumber(), (*i)->GetTypeName().c_str(), (*i)->GetName().c_str());
				m_errors++;
			}
			else if ((reg->GetClassType() == REGCLASS_NORMAL) || (reg->GetClassType() == REGCLASS_TEMP))
			{
				WriteLine("temps.push_back(AddRegisterForMatch(nextState, %s));", reg->GetRegisterClassName().c_str());
			}
			else if (reg->GetClassType() == REGCLASS_LARGE)
			{
				WriteLine("temps.push_back(AddRegisterForMatch(nextState, %s));", reg->GetLowRegisterClassName().c_str());
				WriteLine("temps.push_back(AddRegisterForMatch(nextState, %s));", reg->GetHighRegisterClassName().c_str());
			}
			else
			{
				fprintf(stderr, "%s:%d: error: invalid register class '%s' for '%s'\n", match->GetFileName().c_str(),
					match->GetLineNumber(), (*i)->GetTypeName().c_str(), (*i)->GetName().c_str());
				m_errors++;
			}
		}

		if (!match->GetResult())
			WriteLine("AddMatchNoResult(state, nextState, node, matches, %d, temps);", (int)match->GetIndex());
		else
		{
			RegisterClass* reg = m_parser->GetRegisterClass(match->GetResult()->GetTypeName());
			if (!reg)
			{
				fprintf(stderr, "%s:%d: error: invalid register class '%s' in result\n", match->GetFileName().c_str(),
					match->GetLineNumber(), match->GetResult()->GetTypeName().c_str());
				m_errors++;
			}
			else if (reg->GetClassType() == REGCLASS_NORMAL)
			{
				WriteLine("AddMatchNormalRegisterResult(state, nextState, node, possible, %d, %s, temps);", (int)match->GetIndex(),
					reg->GetRegisterClassName().c_str());
			}
			else if (reg->GetClassType() == REGCLASS_LARGE)
			{
				WriteLine("AddMatchLargeRegisterResult(state, nextState, node, possible, %d, %s, %s, temps);", (int)match->GetIndex(),
					reg->GetLowRegisterClassName().c_str(), reg->GetHighRegisterClassName().c_str());
			}
			else
			{
				fprintf(stderr, "%s:%d: internal error: temporary register class '%s' not fully expanded\n",
					match->GetFileName().c_str(), match->GetLineNumber(), match->GetResult()->GetTypeName().c_str());
				m_errors++;
			}
		}
	EndBlock();
}


void OutputGenerator::GenerateOutputCode(Match* match)
{
	queue<MatchNodeInfo> nodes;
	MatchNodeInfo info;
	info.prefix = "_match";
	info.node = match->GetMatch();
	nodes.push(info);

	map<string, MatchVariableType> varTypes;

	while (nodes.size() != 0)
	{
		info = nodes.front();
		nodes.pop();

		if (info.node->GetClass() == NODE_REG)
		{
			if (m_parser->IsImmediateClass(info.node->GetTypeName()))
			{
				WriteLine("int64_t %s = %s->GetImmediate();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("(void)%s;", info.node->GetName().c_str());
				varTypes[info.node->GetName()] = MATCHVAR_IMM;
			}
			else if (info.node->GetTypeName() == "IMM")
			{
				WriteLine("int64_t %s = %s->GetImmediate();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("(void)%s;", info.node->GetName().c_str());
				varTypes[info.node->GetName()] = MATCHVAR_IMM;
			}
			else if (info.node->GetTypeName() == "STACKVAR")
			{
				WriteLine("uint32_t %s_base = %s->GetRegister();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("int32_t %s_var = %s->GetVariable();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("int64_t %s_offset = %s->GetImmediate();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("uint32_t %s_scratch = m_symFunc->AddRegister(%s);", info.node->GetName().c_str(),
					m_parser->GetDefaultRegisterClass()->GetRegisterClassName().c_str());
				WriteLine("(void)%s_base; (void)%s_var; (void)%s_offset; (void)%s_scratch;",
					info.node->GetName().c_str(), info.node->GetName().c_str(), info.node->GetName().c_str(),
					info.node->GetName().c_str());
				varTypes[info.node->GetName()] = MATCHVAR_STACK;
			}
			else if (info.node->GetTypeName() == "GLOBALVAR")
			{
				WriteLine("int64_t %s_offset = %s->GetImmediate();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("uint32_t %s_scratch = m_symFunc->AddRegister(%s);", info.node->GetName().c_str(),
					m_parser->GetDefaultRegisterClass()->GetRegisterClassName().c_str());
				WriteLine("(void)%s_offset; (void)%s_scratch;", info.node->GetName().c_str(), info.node->GetName().c_str());
				varTypes[info.node->GetName()] = MATCHVAR_GLOBAL;
			}
			else if (info.node->GetTypeName() == "BLOCK")
			{
				WriteLine("TreeBlock* %s = %s->GetBlock();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("(void)%s;", info.node->GetName().c_str());
				varTypes[info.node->GetName()] = MATCHVAR_BLOCK;
			}
			else if (info.node->GetTypeName() == "FUNCTION")
			{
				WriteLine("Function* %s = %s->GetFunction();", info.node->GetName().c_str(), info.prefix.c_str());
				WriteLine("(void)%s;", info.node->GetName().c_str());
				varTypes[info.node->GetName()] = MATCHVAR_FUNC;
			}
			else if (info.node->GetTypeName() == "INPUT")
			{
				WriteLine("vector<uint32_t> %s;", info.node->GetName().c_str());
				WriteLine("for (vector< Ref<TreeNode> >::const_iterator _i = %s->GetChildNodes().begin();", info.prefix.c_str());
				WriteLine("\t_i != %s->GetChildNodes().end(); ++_i)", info.prefix.c_str());
				BeginBlock();
					WriteLine("%s.push_back((*_i)->GetRegister());", info.node->GetName().c_str());
				EndBlock();
				varTypes[info.node->GetName()] = MATCHVAR_INPUT;
			}
			else
			{
				RegisterClass* reg = m_parser->GetRegisterClass(info.node->GetTypeName());
				if (!reg)
				{
					fprintf(stderr, "%s:%d: error: invalid register class '%s' for '%s'\n", match->GetFileName().c_str(),
						match->GetLineNumber(), info.node->GetTypeName().c_str(), info.node->GetName().c_str());
					m_errors++;
				}
				else if (reg->GetClassType() == REGCLASS_NORMAL)
				{
					WriteLine("uint32_t %s = %s->GetRegister();", info.node->GetName().c_str(), info.prefix.c_str());
					WriteLine("(void)%s;", info.node->GetName().c_str());
					varTypes[info.node->GetName()] = MATCHVAR_REG;
				}
				else if (reg->GetClassType() == REGCLASS_LARGE)
				{
					WriteLine("uint32_t %s_low = %s->GetRegister();", info.node->GetName().c_str(), info.prefix.c_str());
					WriteLine("uint32_t %s_high = %s->GetHighRegister();", info.node->GetName().c_str(), info.prefix.c_str());
					WriteLine("(void)%s_low; (void)%s_high;", info.node->GetName().c_str(), info.node->GetName().c_str());
					varTypes[info.node->GetName()] = MATCHVAR_LARGE_REG;
				}
				else
				{
					fprintf(stderr, "%s:%d: error: invalid register class '%s' for '%s'\n", match->GetFileName().c_str(),
						match->GetLineNumber(), info.node->GetTypeName().c_str(), info.node->GetName().c_str());
					m_errors++;
				}
			}
		}

		for (size_t i = 0; i < info.node->GetChildNodes().size(); i++)
		{
			MatchNodeInfo child;
			char indexStr[32];
			sprintf(indexStr, "%d", (int)i);
			child.prefix = string("(*") + info.prefix + ")[" + indexStr + "]";
			child.node = info.node->GetChildNodes()[i];
			nodes.push(child);
		}
	}

	size_t tempIndex = 0;
	for (vector< Ref<TreeNode> >::const_iterator i = match->GetTemps().begin(); i != match->GetTemps().end(); ++i)
	{
		RegisterClass* reg = m_parser->GetRegisterClass((*i)->GetTypeName());
		if (!reg)
		{
			fprintf(stderr, "%s:%d: error: invalid register class '%s' for temporary '%s'\n", match->GetFileName().c_str(),
				match->GetLineNumber(), (*i)->GetTypeName().c_str(), (*i)->GetName().c_str());
			m_errors++;
		}
		else if ((reg->GetClassType() == REGCLASS_NORMAL) || (reg->GetClassType() == REGCLASS_TEMP))
		{
			WriteLine("uint32_t %s = _temps[%d];", (*i)->GetName().c_str(), (int)tempIndex++);
			WriteLine("(void)%s;", (*i)->GetName().c_str());
			varTypes[(*i)->GetName()] = MATCHVAR_REG;
		}
		else if (reg->GetClassType() == REGCLASS_LARGE)
		{
			WriteLine("uint32_t %s_low = _temps[%d];", (*i)->GetName().c_str(), (int)tempIndex++);
			WriteLine("uint32_t %s_high = _temps[%d];", (*i)->GetName().c_str(), (int)tempIndex++);
			WriteLine("(void)%s_low; (void)%s_high;", (*i)->GetName().c_str(), (*i)->GetName().c_str());
			varTypes[(*i)->GetName()] = MATCHVAR_LARGE_REG;
		}
		else
		{
			fprintf(stderr, "%s:%d: error: invalid register class '%s' for temporary '%s'\n", match->GetFileName().c_str(),
				match->GetLineNumber(), (*i)->GetTypeName().c_str(), (*i)->GetName().c_str());
			m_errors++;
		}
	}

	if (match->GetResult())
	{
		RegisterClass* reg = m_parser->GetRegisterClass(match->GetResult()->GetTypeName());
		if (!reg)
		{
			fprintf(stderr, "%s:%d: error: invalid register class '%s' for result\n", match->GetFileName().c_str(),
				match->GetLineNumber(), match->GetResult()->GetTypeName().c_str());
			m_errors++;
		}
		else if ((reg->GetClassType() == REGCLASS_NORMAL) || (reg->GetClassType() == REGCLASS_TEMP))
		{
			WriteLine("uint32_t %s = _result->GetRegister();", match->GetResult()->GetName().c_str());
			WriteLine("(void)%s;", match->GetResult()->GetName().c_str());
			varTypes[match->GetResult()->GetName()] = MATCHVAR_REG;
		}
		else if (reg->GetClassType() == REGCLASS_LARGE)
		{
			WriteLine("uint32_t %s_low = _result->GetRegister();", match->GetResult()->GetName().c_str());
			WriteLine("uint32_t %s_high = _result->GetHighRegister();", match->GetResult()->GetName().c_str());
			WriteLine("(void)%s_low; (void)%s_high;", match->GetResult()->GetName().c_str(),
				match->GetResult()->GetName().c_str());
			varTypes[match->GetResult()->GetName()] = MATCHVAR_LARGE_REG;
		}
		else
		{
			fprintf(stderr, "%s:%d: error: invalid register class '%s' for result\n", match->GetFileName().c_str(),
				match->GetLineNumber(), match->GetResult()->GetTypeName().c_str());
			m_errors++;
		}
	}

	WriteCodeBlock(match->GetCode(), varTypes);
}


void OutputGenerator::GenerateEncodingMacro(const string& name, Encoding* encoding)
{
	string size;
	switch (encoding->GetWidth())
	{
	case 8:
		size = "uint8_t";
		break;
	case 16:
		size = "uint16_t";
		break;
	case 32:
		size = "uint32_t";
		break;
	case 64:
		size = "uint64_t";
		break;
	default:
		fprintf(stderr, "error: invalid encoding size %d for '%s'\n", (int)encoding->GetWidth(), name.c_str());
		m_errors++;
		return;
	}

	string value;
	string def = string("#define ENCODING_VALUE_") + name + "(";
	bool firstName = true;
	for (vector<EncodingField>::const_iterator j = encoding->GetFields().begin(); j != encoding->GetFields().end(); ++j)
	{
		if ((j->type == FIELD_FIXED_VALUE) && (j->value == 0))
			continue;

		if (j != encoding->GetFields().begin())
			value += " | ";

		char maskStr[32], shiftStr[32];
		if (encoding->GetWidth() == 64)
			sprintf(maskStr, "0x%" PRIx64 "LL", ((uint64_t)1 << j->width) - 1);
		else
			sprintf(maskStr, "0x%" PRIx32, ((uint32_t)1 << j->width) - 1);
		sprintf(shiftStr, "%d", (int)j->start);

		if (j->type == FIELD_FIXED_VALUE)
		{
			char valueStr[32];
			uint64_t fixedVal = j->value;
			fixedVal &= (1LL << j->width) - 1;
			if (encoding->GetWidth() == 64)
				sprintf(valueStr, "0x%" PRIx64 "LL", fixedVal);
			else
				sprintf(valueStr, "0x%" PRIx32, (uint32_t)fixedVal);
			if (j->start == 0)
				value += string("((") + size + ")(" + valueStr + "))";
			else
				value += string("(((") + size + ")(" + valueStr + ")) << " + shiftStr + ")";
		}
		else
		{
			if (firstName)
				firstName = false;
			else
				def += ", ";
			def += j->name;
			if (j->start == 0)
				value += string("(((") + size + ")(" + j->name + ")) & " + maskStr + ")";
			else
				value += string("((((") + size + ")(" + j->name + ")) & " + maskStr + ") << " + shiftStr + ")";
		}
	}

	if (value.size() == 0)
		value = string("(") + size + ")0";

	def += ") (" + value + ")";

	WriteLine("%s", def.c_str());
}


void OutputGenerator::WriteInstructionTokenVariables(Instruction* instr, std::map<std::string, MatchVariableType>& vars)
{
	for (vector<InstructionToken>::const_iterator i = instr->GetTokens().begin(); i != instr->GetTokens().end(); ++i)
	{
		if (i->type == INSTRTOKEN_TEXT)
			continue;

		switch (i->operand)
		{
		case OPERAND_REG:
			vars[i->name] = MATCHVAR_REG;
			WriteLine("uint32_t %s = m_operands[m_%s_index].reg;", i->name.c_str(), i->name.c_str());
			WriteLine("(void)%s;", i->name.c_str());
			break;
		case OPERAND_IMMED:
			vars[i->name] = MATCHVAR_IMM;
			WriteLine("int64_t %s = m_operands[m_%s_index].immed;", i->name.c_str(), i->name.c_str());
			WriteLine("(void)%s;", i->name.c_str());
			break;
		case OPERAND_FUNCTION:
			vars[i->name] = MATCHVAR_FUNC_AND_BLOCK;
			WriteLine("Function* %s_func = m_operands[m_%s_index].func;", i->name.c_str(), i->name.c_str());
			WriteLine("ILBlock* %s_block = m_operands[m_%s_index].block;", i->name.c_str(), i->name.c_str());
			WriteLine("(void)%s_func; (void)%s_block;", i->name.c_str(), i->name.c_str());
			break;
		case OPERAND_TEMP:
			vars[i->name] = MATCHVAR_REG;
			WriteLine("uint32_t %s = m_operands[m_%s_index].reg;", i->name.c_str(), i->name.c_str());
			WriteLine("(void)%s;", i->name.c_str());
			break;
		case OPERAND_REG_LIST:
			break;
		case OPERAND_STACK_VAR:
			vars[i->name] = MATCHVAR_VAR_COMPONENTS;
			WriteLine("uint32_t %s_base = m_operands[m_%s_index].reg;", i->name.c_str(), i->name.c_str());
			WriteLine("uint32_t %s_offset = func->GetStackVarOffset(m_operands[m_%s_index + 1].reg) +",
				i->name.c_str(), i->name.c_str());
			WriteLine("\tm_operands[m_%s_index + 1].immed;", i->name.c_str());
			WriteLine("uint32_t %s_temp = m_operands[m_%s_index + 2].reg;", i->name.c_str(), i->name.c_str());
			WriteLine("(void)%s_base; (void)%s_offset; (void)%s_temp;", i->name.c_str(), i->name.c_str(), i->name.c_str());
			break;
		case OPERAND_GLOBAL_VAR:
			vars[i->name] = MATCHVAR_VAR_COMPONENTS;
			WriteLine("uint32_t %s_base = m_operands[m_%s_index].reg;", i->name.c_str(), i->name.c_str());
			WriteLine("uint32_t %s_offset = m_operands[m_%s_index + 1].immed;", i->name.c_str(), i->name.c_str());
			WriteLine("uint32_t %s_temp = m_operands[m_%s_index + 2].reg;", i->name.c_str(), i->name.c_str());
			WriteLine("(void)%s_base; (void)%s_offset; (void)%s_temp;", i->name.c_str(), i->name.c_str(), i->name.c_str());
			break;
		default:
			fprintf(stderr, "error: invalid operand '%s' for instruction '%s'\n",
				i->name.c_str(), instr->GetName().c_str());
			m_errors++;
			return;
		}
	}
}


void OutputGenerator::GenerateInstructionClass(size_t i, Instruction* instr)
{
	WriteLine("class %s_SymInstr_%d: public %s_SymInstr", m_parser->GetArchName().c_str(),
		(int)i, m_parser->GetArchName().c_str());
	BeginBlock();
		// Write out variables to hold operand index for each token that contains an operand
		for (vector<InstructionToken>::const_iterator j = instr->GetTokens().begin(); j != instr->GetTokens().end(); ++j)
		{
			if (j->type == INSTRTOKEN_TEXT)
				continue;
			if (j->operand == OPERAND_REG_LIST)
				WriteLine("int32_t m_%s_count;", j->name.c_str());
			WriteLine("int32_t m_%s_index;", j->name.c_str());
		}

		WriteUnindented("public:");

		// Write out constructor
		WriteLine("%s_SymInstr_%d(", m_parser->GetArchName().c_str(), (int)i);
		m_indentLevel++;
		bool first = true;
		for (vector<InstructionToken>::const_iterator j = instr->GetTokens().begin(); j != instr->GetTokens().end(); ++j)
		{
			if (j->type == INSTRTOKEN_TEXT)
				continue;
			const char* comma = "";
			if (first)
				first = false;
			else
				comma = ", ";
			switch (j->operand)
			{
			case OPERAND_REG:
				WriteLine("%suint32_t %s", comma, j->name.c_str());
				break;
			case OPERAND_REG_LIST:
				WriteLine("%sconst vector<uint32_t>& %s", comma, j->name.c_str());
				break;
			case OPERAND_IMMED:
				WriteLine("%sint64_t %s", comma, j->name.c_str());
				break;
			case OPERAND_STACK_VAR:
				WriteLine("%suint32_t %s_base, uint32_t %s_var, int64_t %s_offset, uint32_t %s_scratch",
					comma, j->name.c_str(), j->name.c_str(), j->name.c_str(), j->name.c_str());
				break;
			case OPERAND_GLOBAL_VAR:
				WriteLine("%suint32_t %s_base, int64_t %s_offset, uint32_t %s_scratch",
					comma, j->name.c_str(), j->name.c_str(), j->name.c_str());
				break;
			case OPERAND_FUNCTION:
				WriteLine("%sFunction* %s_func, ILBlock* %s_block", comma, j->name.c_str(), j->name.c_str());
				break;
			case OPERAND_TEMP:
				WriteLine("%suint32_t %s", comma, j->name.c_str());
				break;
			default:
				fprintf(stderr, "error: invalid operand '%s' for instruction '%s'\n", j->name.c_str(), instr->GetName().c_str());
				m_errors++;
				return;
			}
		}
		WriteLine(")");
		m_indentLevel--;
		BeginBlock();
			if (instr->GetFlags() & SYMFLAG_WRITES_FLAGS)
				WriteLine("EnableFlag(SYMFLAG_WRITES_FLAGS);");
			if (instr->GetFlags() & SYMFLAG_USES_FLAGS)
				WriteLine("EnableFlag(SYMFLAG_USES_FLAGS);");
			if (instr->GetFlags() & SYMFLAG_MEMORY_BARRIER)
				WriteLine("EnableFlag(SYMFLAG_MEMORY_BARRIER);");
			if (instr->GetFlags() & (SYMFLAG_CONTROL_FLOW | SYMFLAG_CALL))
				WriteLine("EnableFlag(SYMFLAG_CONTROL_FLOW);");
			if (instr->GetFlags() & SYMFLAG_CALL)
				WriteLine("EnableFlag(SYMFLAG_CALL);");
			if (instr->GetFlags() & SYMFLAG_COPY)
				WriteLine("EnableFlag(SYMFLAG_COPY);");
			if (instr->GetFlags() & SYMFLAG_STACK)
				WriteLine("EnableFlag(SYMFLAG_STACK);");

			for (vector<InstructionToken>::const_iterator j = instr->GetTokens().begin(); j != instr->GetTokens().end(); ++j)
			{
				if (j->type == INSTRTOKEN_TEXT)
					continue;

				WriteLine("m_%s_index = (int)m_operands.size();", j->name.c_str());

				switch (j->operand)
				{
				case OPERAND_REG:
					WriteLine("if (%s == SYMREG_NONE)", j->name.c_str());
					WriteLine("\tm_%s_index = -1;", j->name.c_str());
					WriteLine("else");
					switch (j->access)
					{
					case ACCESS_READ:
						WriteLine("\tAddReadRegisterOperand(%s);", j->name.c_str());
						break;
					case ACCESS_WRITE:
						WriteLine("\tAddWriteRegisterOperand(%s);", j->name.c_str());
						break;
					case ACCESS_READ_WRITE:
						WriteLine("\tAddReadWriteRegisterOperand(%s);", j->name.c_str());
						break;
					default:
						fprintf(stderr, "error: invalid access type on operand '%s' for instruction '%s'\n",
							j->name.c_str(), instr->GetName().c_str());
						m_errors++;
						return;
					}
					break;
				case OPERAND_REG_LIST:
					switch (j->access)
					{
					case ACCESS_READ:
						WriteLine("for (vector<uint32_t>::const_iterator i = %s.begin(); i != %s.end(); ++i)",
							j->name.c_str(), j->name.c_str());
						WriteLine("\tAddReadRegisterOperand(*i);");
						break;
					case ACCESS_WRITE:
						WriteLine("for (vector<uint32_t>::const_iterator i = %s.begin(); i != %s.end(); ++i)",
							j->name.c_str(), j->name.c_str());
						WriteLine("\tAddWriteRegisterOperand(*i);");
						break;
					case ACCESS_READ_WRITE:
						WriteLine("for (vector<uint32_t>::const_iterator i = %s.begin(); i != %s.end(); ++i)",
							j->name.c_str(), j->name.c_str());
						WriteLine("\tAddReadWriteRegisterOperand(*i);");
						break;
					default:
						fprintf(stderr, "error: invalid access type on operand '%s' for instruction '%s'\n",
							j->name.c_str(), instr->GetName().c_str());
						m_errors++;
						return;
					}
					WriteLine("m_%s_count = (int32_t)%s.size();", j->name.c_str(), j->name.c_str());
					break;
				case OPERAND_IMMED:
					WriteLine("AddImmediateOperand(%s);", j->name.c_str());
					break;
				case OPERAND_STACK_VAR:
					WriteLine("AddReadRegisterOperand(%s_base);", j->name.c_str());
					WriteLine("AddStackVarOperand(%s_var, %s_offset);", j->name.c_str(), j->name.c_str());
					WriteLine("AddTemporaryRegisterOperand(%s_scratch);", j->name.c_str());
					break;
				case OPERAND_GLOBAL_VAR:
					WriteLine("AddReadRegisterOperand(%s_base);", j->name.c_str());
					WriteLine("AddGlobalVarOperand(%s_offset);", j->name.c_str());
					WriteLine("AddTemporaryRegisterOperand(%s_scratch);", j->name.c_str());
					break;
				case OPERAND_FUNCTION:
					WriteLine("AddBlockOperand(%s_func, %s_block);", j->name.c_str(), j->name.c_str());
					break;
				case OPERAND_TEMP:
					WriteLine("AddTemporaryRegisterOperand(%s);", j->name.c_str());
					break;
				default:
					fprintf(stderr, "error: invalid operand '%s' for instruction '%s'\n",
						j->name.c_str(), instr->GetName().c_str());
					m_errors++;
					return;
				}
			}
		EndBlock();

		// Write out function to print the instruction
		WriteLine("virtual void Print(SymInstrFunction* func)");
		BeginBlock();
			for (vector<InstructionToken>::const_iterator j = instr->GetTokens().begin(); j != instr->GetTokens().end(); ++j)
			{
				if (j->type == INSTRTOKEN_TEXT)
				{
					if (j->name == ",")
						WriteLine("fprintf(stderr, \", \");");
					else if (j->name == "\"")
						WriteLine("fprintf(stderr, \"\\\"\");");
					else if (j->name == "%")
						WriteLine("fprintf(stderr, \"%%\");");
					else if (isalnum(j->name[0]) || (j->name[0] == '_'))
						WriteLine("fprintf(stderr, \"%s \");", j->name.c_str());
					else
						WriteLine("fprintf(stderr, \"%s\");", j->name.c_str());
					continue;
				}

				switch (j->operand)
				{
				case OPERAND_REG:
				case OPERAND_IMMED:
				case OPERAND_FUNCTION:
				case OPERAND_TEMP:
					WriteLine("if (m_%s_index == -1)", j->name.c_str());
					WriteLine("\tfprintf(stderr, \"none\");");
					WriteLine("else");
					WriteLine("\tm_operands[m_%s_index].Print(func);", j->name.c_str());
					break;
				case OPERAND_REG_LIST:
					WriteLine("for (int32_t i = 0; i < m_%s_count; i++)", j->name.c_str());
					BeginBlock();
						WriteLine("if (i != 0)");
						WriteLine("\tfprintf(stderr, \":\");");
						WriteLine("m_operands[m_%s_index + i].Print(func);", j->name.c_str());
					EndBlock();
					break;
				case OPERAND_STACK_VAR:
				case OPERAND_GLOBAL_VAR:
					WriteLine("m_operands[m_%s_index].Print(func);", j->name.c_str());
					WriteLine("fprintf(stderr, \":\");");
					WriteLine("m_operands[m_%s_index + 1].Print(func);", j->name.c_str());
					WriteLine("fprintf(stderr, \":\");");
					WriteLine("m_operands[m_%s_index + 2].Print(func);", j->name.c_str());
					break;
				default:
					fprintf(stderr, "error: invalid operand '%s' for instruction '%s'\n",
						j->name.c_str(), instr->GetName().c_str());
					m_errors++;
					return;
				}
			}
		EndBlock();

		// Write out code to emit the instruction's machine code
		WriteLine("virtual bool EmitInstruction(SymInstrFunction* func, OutputBlock* out)");
		BeginBlock();
			map<string, MatchVariableType> vars;
			WriteInstructionTokenVariables(instr, vars);
			WriteCodeBlock(instr->GetCode(), vars);
			WriteLine("return true;");
		EndBlock();

		if (instr->GetUpdate())
		{
			// Write out code to replace instruction with a set of instructions after code generation completes
			WriteLine("virtual bool UpdateInstruction(SymInstrFunction* func, const Settings& settings,");
			WriteLine("\tvector<SymInstr*>& replacement)");
			BeginBlock();
				map<string, MatchVariableType> vars;
				WriteInstructionTokenVariables(instr, vars);
				WriteCodeBlock(instr->GetUpdate(), vars, CODEBLOCK_UPDATE);
			EndBlock();
		}
	EndBlockSemicolon();
}


void OutputGenerator::GenerateInstructionFunction(size_t i, Instruction* instr, bool proto)
{
	WriteLine("SymInstr* %s_%s(", m_parser->GetArchName().c_str(), instr->GetName().c_str());
	m_indentLevel++;
	bool first = true;
	for (vector<InstructionToken>::const_iterator j = instr->GetTokens().begin(); j != instr->GetTokens().end(); ++j)
	{
		if (j->type == INSTRTOKEN_TEXT)
			continue;
		const char* comma = "";
		if (first)
			first = false;
		else
			comma = ", ";
		switch (j->operand)
		{
		case OPERAND_REG:
			WriteLine("%suint32_t %s", comma, j->name.c_str());
			break;
		case OPERAND_REG_LIST:
			WriteLine("%sconst vector<uint32_t>& %s", comma, j->name.c_str());
			break;
		case OPERAND_IMMED:
			WriteLine("%sint64_t %s", comma, j->name.c_str());
			break;
		case OPERAND_STACK_VAR:
			WriteLine("%suint32_t %s_base, uint32_t %s_var, int64_t %s_offset, uint32_t %s_scratch",
				comma, j->name.c_str(), j->name.c_str(), j->name.c_str(), j->name.c_str());
			break;
		case OPERAND_GLOBAL_VAR:
			WriteLine("%suint32_t %s_base, int64_t %s_offset, uint32_t %s_scratch",
				comma, j->name.c_str(), j->name.c_str(), j->name.c_str());
			break;
		case OPERAND_FUNCTION:
			WriteLine("%sFunction* %s_func, ILBlock* %s_block", comma, j->name.c_str(), j->name.c_str());
			break;
		case OPERAND_TEMP:
			WriteLine("%suint32_t %s", comma, j->name.c_str());
			break;
		default:
			fprintf(stderr, "error: invalid operand '%s' for instruction '%s'\n", j->name.c_str(), instr->GetName().c_str());
			m_errors++;
			return;
		}
	}
	m_indentLevel--;
	if (proto)
	{
		WriteLine(");");
		return;
	}
	WriteLine(")");
	BeginBlock();
		WriteLine("return new %s_SymInstr_%d(", m_parser->GetArchName().c_str(), (int)i);
		m_indentLevel++;
		first = true;
		for (vector<InstructionToken>::const_iterator j = instr->GetTokens().begin(); j != instr->GetTokens().end(); ++j)
		{
			if (j->type == INSTRTOKEN_TEXT)
				continue;
			const char* comma = "";
			if (first)
				first = false;
			else
				comma = ", ";
			switch (j->operand)
			{
			case OPERAND_REG:
			case OPERAND_REG_LIST:
			case OPERAND_IMMED:
			case OPERAND_TEMP:
				WriteLine("%s%s", comma, j->name.c_str());
				break;
			case OPERAND_STACK_VAR:
				WriteLine("%s%s_base, %s_var, %s_offset, %s_scratch",
					comma, j->name.c_str(), j->name.c_str(), j->name.c_str(), j->name.c_str());
				break;
			case OPERAND_GLOBAL_VAR:
				WriteLine("%s%s_base, %s_offset, %s_scratch",
					comma, j->name.c_str(), j->name.c_str(), j->name.c_str());
				break;
			case OPERAND_FUNCTION:
				WriteLine("%s%s_func, %s_block", comma, j->name.c_str(), j->name.c_str());
				break;
			default:
				fprintf(stderr, "error: invalid operand '%s' for instruction '%s'\n", j->name.c_str(), instr->GetName().c_str());
				m_errors++;
				return;
			}
		}
		WriteLine(");");
		m_indentLevel--;
	EndBlock();
}


bool OutputGenerator::Generate(string& output)
{
	WriteLine("#include \"Output.h\"");
	WriteLine("#include \"Struct.h\"");
	WriteLine("#include \"TreeBlock.h\"");
	WriteLine("#include \"SymInstr.h\"");
	WriteLine("#include <stdio.h>");
	WriteLine("#include <stdlib.h>");
	WriteLine("#include <string.h>");
	WriteLine("#include <string>");
	WriteLine("#include <vector>");
	WriteLine("#include <map>");
	WriteLine("#include <set>");
	WriteLine("#include <queue>");

	for (vector<string>::const_iterator i = m_parser->GetIncludes().begin(); i != m_parser->GetIncludes().end(); ++i)
		WriteLine("#include \"%s\"", i->c_str());

	WriteLine("#define TEMP_REGISTER(cls) m_symFunc->AddRegister(cls)");
	WriteLine("#define UNSAFE_STACK_PIVOT 0x1000");

	// Write out register class enumeration
	WriteLine("enum %s_RegisterClass", m_parser->GetArchName().c_str());
	BeginBlock();
		for (set<string>::const_iterator i = m_parser->GetRegisterClassNames().begin();
			i != m_parser->GetRegisterClassNames().end(); ++i)
		{
			if (i == m_parser->GetRegisterClassNames().begin())
				WriteLine("%s", i->c_str());
			else
				WriteLine(", %s", i->c_str());
		}
	EndBlockSemicolon();

	// Write out each encoding type
	for (map< string, Ref<Encoding> >::const_iterator i = m_parser->GetEncodings().begin();
		i != m_parser->GetEncodings().end(); ++i)
		GenerateEncodingMacro(i->first, i->second);

	WriteLine("using namespace std;");

	// Write out prototypes for instruction generator functions
	for (size_t i = 0; i < m_parser->GetInstructions().size(); i++)
		GenerateInstructionFunction(i, m_parser->GetInstructions()[i], true);

	// Write out base class for instructions
	WriteLine("class %s_SymInstr: public SymInstr", m_parser->GetArchName().c_str());
	BeginBlock();
		WriteUnindented("protected:");

		for (vector< Ref<CodeBlock> >::const_iterator i = m_parser->GetInstrFunctions().begin();
			i != m_parser->GetInstrFunctions().end(); ++i)
		{
			WriteLine("static");
			WriteCodeBlock(*i);
		}

		WriteUnindented("public:");

		WriteLine("%s_SymInstr()", m_parser->GetArchName().c_str());
		BeginBlock();
		EndBlock();
	EndBlockSemicolon();

	// Write out classes for each instruction
	for (size_t i = 0; i < m_parser->GetInstructions().size(); i++)
		GenerateInstructionClass(i, m_parser->GetInstructions()[i]);

	// Write out implementations for instruction generator functions
	for (size_t i = 0; i < m_parser->GetInstructions().size(); i++)
		GenerateInstructionFunction(i, m_parser->GetInstructions()[i], false);

	// Write out architecture-specific function class
	WriteLine("class %s_SymInstrFunction: public SymInstrFunction", m_parser->GetArchName().c_str());
	BeginBlock();
		WriteUnindented("public:");

		WriteLine("%s_SymInstrFunction(const Settings& settings, Function* func): SymInstrFunction(settings, func)",
			m_parser->GetArchName().c_str());
		BeginBlock();
		EndBlock();

		WriteLine("size_t GetNativeSize()");
		BeginBlock();
			WriteLine("return %d;", m_parser->GetArchBits() / 8);
		EndBlock();

		WriteLine("void PrintRegisterClass(uint32_t cls)");
		BeginBlock();
			WriteLine("switch (cls)");
			BeginBlock();
				for (set<string>::const_iterator i = m_parser->GetRegisterClassNames().begin();
					i != m_parser->GetRegisterClassNames().end(); ++i)
				{
					WriteLine("case %s:", i->c_str());
					WriteLine("\tfprintf(stderr, \"%s\");", i->c_str());
					WriteLine("\tbreak;");
				}
				WriteLine("default:");
				WriteLine("\tfprintf(stderr, \"invalid\");");
				WriteLine("\tbreak;");
			EndBlock();
		EndBlock();

		WriteLine("bool IsRegisterClassFixed(uint32_t cls)");
		BeginBlock();
			WriteLine("switch (cls)");
			BeginBlock();
				for (std::map< std::string, Ref<RegisterClass> >::const_iterator i = m_parser->GetRegisterClasses().begin();
					i != m_parser->GetRegisterClasses().end(); ++i)
				{
					if (i->second->GetClassType() != REGCLASS_NORMAL)
						continue;
					if (!i->second->GetFixedRegister())
						continue;
					WriteLine("case %s:", i->first.c_str());
					WriteLine("\treturn true;");
				}
				WriteLine("default:");
				WriteLine("\treturn false;");
			EndBlock();
		EndBlock();

		WriteLine("uint32_t GetFixedRegisterForClass(uint32_t cls)");
		BeginBlock();
			WriteLine("switch (cls)");
			BeginBlock();
				for (std::map< std::string, Ref<RegisterClass> >::const_iterator i = m_parser->GetRegisterClasses().begin();
					i != m_parser->GetRegisterClasses().end(); ++i)
				{
					if (i->second->GetClassType() != REGCLASS_NORMAL)
						continue;
					if (!i->second->GetFixedRegister())
						continue;
					WriteLine("case %s:", i->first.c_str());
					WriteLine("\treturn SYMREG_NATIVE_REG(");
					m_indentLevel += 2;
					WriteCodeBlock(i->second->GetFixedRegister());
					WriteLine(");");
					m_indentLevel -= 2;
				}
				WriteLine("default:");
				WriteLine("\treturn SYMREG_NONE;");
			EndBlock();
		EndBlock();

		WriteLine("uint32_t GetSpecialRegisterAssignment(uint32_t reg)");
		BeginBlock();
			WriteLine("switch (reg)");
			BeginBlock();
				for (std::map< std::string, Ref<CodeBlock> >::const_iterator i = m_parser->GetSpecialRegs().begin();
					i != m_parser->GetSpecialRegs().end(); ++i)
				{
					WriteLine("case %s:", i->first.c_str());
					WriteLine("\treturn SYMREG_NATIVE_REG(");
					m_indentLevel += 2;
					WriteCodeBlock(i->second);
					WriteLine(");");
					m_indentLevel -= 2;
				}
				WriteLine("default:");
				WriteLine("\treturn SYMREG_NONE;");
			EndBlock();
		EndBlock();

		WriteLine("vector<uint32_t> GetCallerSavedRegisters()");
		BeginBlock();
			WriteLine("vector<uint32_t> result;");
			for (std::vector< Ref<CodeBlock> >::const_iterator i = m_parser->GetCallerSavedRegs().begin();
				i != m_parser->GetCallerSavedRegs().end(); ++i)
			{
				WriteLine("result.push_back(SYMREG_NATIVE_REG(");
				m_indentLevel++;
				WriteCodeBlock(*i);
				WriteLine("));");
				m_indentLevel--;
			}
			WriteLine("return result;");
		EndBlock();

		WriteLine("vector<uint32_t> GetCalleeSavedRegisters()");
		BeginBlock();
			WriteLine("vector<uint32_t> result;");
			for (std::vector< Ref<CodeBlock> >::const_iterator i = m_parser->GetCalleeSavedRegs().begin();
				i != m_parser->GetCalleeSavedRegs().end(); ++i)
			{
				WriteLine("result.push_back(SYMREG_NATIVE_REG(");
				m_indentLevel++;
				WriteCodeBlock(*i);
				WriteLine("));");
				m_indentLevel--;
			}
			WriteLine("return result;");
		EndBlock();

		for (vector< Ref<CodeBlock> >::const_iterator i = m_parser->GetArchFunctions().begin();
			i != m_parser->GetArchFunctions().end(); ++i)
		{
			map<string, MatchVariableType> vars;
			WriteCodeBlock(*i, vars, CODEBLOCK_ARCH);
		}
	EndBlockSemicolon();

	WriteLine("class %s: public Output", m_className.c_str());
	BeginBlock();
		WriteLine("struct IncomingParameterCopy");
		BeginBlock();
			WriteLine("Variable* var;");
			WriteLine("uint32_t incomingReg;");
			WriteLine("uint32_t incomingHighReg;");
			WriteLine("uint32_t stackVar;");
		EndBlockSemicolon();

		WriteLine("struct MatchInfo");
		BeginBlock();
			WriteLine("size_t rule;");
			WriteLine("Ref<TreeNode> match, result;");
			WriteLine("vector<uint32_t> temps;");
		EndBlockSemicolon();

		WriteLine("struct RegisterInfo");
		BeginBlock();
			WriteLine("uint32_t reg, cls;");
		EndBlockSemicolon();

		WriteLine("struct MatchState");
		BeginBlock();
			WriteLine("size_t cost;");
			WriteLine("Ref<TreeNode> node;");
			WriteLine("vector<MatchInfo> matches;");
			WriteLine("vector<RegisterInfo> regs;");
		EndBlockSemicolon();

		WriteLine("Function* m_func;");
		WriteLine("%s_SymInstrFunction* m_symFunc;", m_parser->GetArchName().c_str());
		WriteLine("TreeBlock* m_currentBlock;");
		WriteLine("VariableAssignments m_vars;");
		WriteLine("vector<IncomingParameterCopy> m_paramCopy;");
		WriteLine("bool m_framePointerEnabled;");
		WriteLine("uint32_t m_globalBaseReg;");
		WriteLine("uint32_t m_varargStart;");

		for (vector< Ref<CodeBlock> >::const_iterator i = m_parser->GetVariables().begin(); i != m_parser->GetVariables().end(); ++i)
			WriteCodeBlock(*i);

		for (map< string, Ref<CodeBlock> >::const_iterator i = m_parser->GetImmediateClasses().begin();
			i != m_parser->GetImmediateClasses().end(); ++i)
		{
			WriteLine("bool MatchImmClass_%s(int64_t value)", i->first.c_str());
			BeginBlock();
				WriteCodeBlock(i->second);
			EndBlock();
		}

		for (vector< Ref<CodeBlock> >::const_iterator i = m_parser->GetFunctions().begin(); i != m_parser->GetFunctions().end(); ++i)
			WriteCodeBlock(*i);

		for (size_t i = 0; i < m_parser->GetMatches().size(); i++)
		{
			if (m_parser->GetMatches()[i]->GetTemps().size() != 0)
			{
				WriteLine("void OutputRule_%d(SymInstrBlock* out, TreeNode* _match, TreeNode* _result,", (int)i);
				WriteLine("\tconst vector<uint32_t>& _temps)");
			}
			else
			{
				WriteLine("void OutputRule_%d(SymInstrBlock* out, TreeNode* _match, TreeNode* _result)", (int)i);
			}
			BeginBlock();
				GenerateOutputCode(m_parser->GetMatches()[i]);
			EndBlock();
		}

		WriteLine("bool OutputRule(SymInstrBlock* out, size_t rule, TreeNode* match, TreeNode* result,");
		WriteLine("\tconst vector<uint32_t>& temps)");
		BeginBlock();
			WriteLine("switch (rule)");
			BeginBlock();
				for (size_t i = 0; i < m_parser->GetMatches().size(); i++)
				{
					WriteLine("case %d:", (int)i);
					if (m_parser->GetMatches()[i]->GetTemps().size() != 0)
						WriteLine("\tOutputRule_%d(out, match, result, temps);", (int)i);
					else
						WriteLine("\tOutputRule_%d(out, match, result);", (int)i);
					WriteLine("\treturn true;");
				}
				WriteLine("default:");
				WriteLine("\treturn false;");
			EndBlock();
		EndBlock();

		WriteLine("bool IsRegisterClassCompatible(uint32_t regClass, uint32_t matchClass)");
		BeginBlock();
			WriteLine("if (regClass == matchClass)");
			WriteLine("\treturn true;");

			WriteLine("switch (matchClass)");
			BeginBlock();
				for (map< string, vector<string> >::const_iterator i = m_parser->GetRegisterSubclasses().begin();
					i != m_parser->GetRegisterSubclasses().end(); ++i)
				{
					WriteLine("case %s:", i->first.c_str());
					for (vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
					{
						WriteLine("\tif (regClass == %s)", j->c_str());
						WriteLine("\t\treturn true;");
					}
					WriteLine("\treturn false;");
				}
				WriteLine("default:");
				WriteLine("\treturn false;");
			EndBlock();
		EndBlock();

		WriteLine("uint32_t AddRegisterForMatch(MatchState& state, uint32_t cls)");
		BeginBlock();
			WriteLine("RegisterInfo info;");
			WriteLine("info.reg = m_symFunc->GetPossibleMatchRegister(state.regs.size());");
			WriteLine("info.cls = cls;");
			WriteLine("state.regs.push_back(info);");
			WriteLine("return info.reg;");
		EndBlock();

		WriteLine("void AddMatchNoResult(MatchState& state, MatchState& nextState, Ref<TreeNode> node,");
		WriteLine("\tvector<MatchState>& matches, size_t idx, const vector<uint32_t>& temps)");
		BeginBlock();
			WriteLine("MatchInfo match;");
			WriteLine("match.rule = idx;");
			WriteLine("match.match = node;");
			WriteLine("match.temps = temps;");

			WriteLine("match.result = new TreeNode(NODE_NOP);");

			WriteLine("nextState.node = new TreeNode(state.node, match.match, match.result);");
			WriteLine("nextState.matches.push_back(match);");

			WriteLine("matches.push_back(nextState);");

			WriteLine("if (m_settings.internalDebug)");
			BeginBlock();
				WriteLine("fprintf(stderr, \"Rule %%d: \", (int)idx);");
				WriteLine("state.node->Print();");
				WriteLine("fprintf(stderr, \" (complete)\\n\");");
			EndBlock();
		EndBlock();

		WriteLine("void AddMatchNormalRegisterResult(MatchState& state, MatchState& nextState, Ref<TreeNode> node,");
		WriteLine("\tqueue<MatchState>& possible, size_t idx, uint32_t regClass, const vector<uint32_t>& temps)");
		BeginBlock();
			WriteLine("MatchInfo match;");
			WriteLine("match.rule = idx;");
			WriteLine("match.match = node;");
			WriteLine("match.temps = temps;");

			WriteLine("match.result = TreeNode::CreateRegNode(AddRegisterForMatch(nextState, regClass), regClass, node->GetType());");

			WriteLine("nextState.node = new TreeNode(state.node, match.match, match.result);");
			WriteLine("nextState.matches.push_back(match);");

			WriteLine("possible.push(nextState);");

			WriteLine("if (m_settings.internalDebug)");
			BeginBlock();
				WriteLine("fprintf(stderr, \"Rule %%d: \", (int)idx);");
				WriteLine("state.node->Print();");
				WriteLine("fprintf(stderr, \" => \");");
				WriteLine("nextState.node->Print();");
				WriteLine("fprintf(stderr, \"\\n\");");
			EndBlock();
		EndBlock();

		WriteLine("void AddMatchLargeRegisterResult(MatchState& state, MatchState& nextState, Ref<TreeNode> node,");
		WriteLine("\tqueue<MatchState>& possible, size_t idx, uint32_t lowClass, uint32_t highClass,");
		WriteLine("\tconst vector<uint32_t>& temps)");
		BeginBlock();
			WriteLine("MatchInfo match;");
			WriteLine("match.rule = idx;");
			WriteLine("match.match = node;");
			WriteLine("match.temps = temps;");

			WriteLine("uint32_t low = AddRegisterForMatch(nextState, lowClass);");
			WriteLine("uint32_t high = AddRegisterForMatch(nextState, highClass);");
			WriteLine("match.result = TreeNode::CreateLargeRegNode(low, high, lowClass, highClass, node->GetType());");

			WriteLine("nextState.node = new TreeNode(state.node, match.match, match.result);");
			WriteLine("nextState.matches.push_back(match);");

			WriteLine("possible.push(nextState);");

			WriteLine("if (m_settings.internalDebug)");
			BeginBlock();
				WriteLine("fprintf(stderr, \"Rule %%d: \", (int)idx);");
				WriteLine("state.node->Print();");
				WriteLine("fprintf(stderr, \" => \");");
				WriteLine("nextState.node->Print();");
				WriteLine("fprintf(stderr, \"\\n\");");
			EndBlock();
		EndBlock();

		WriteLine("void MatchAndReduceSingleNode(MatchState& state, Ref<TreeNode> node, queue<MatchState>& possible,");
		WriteLine("\tvector<MatchState>& matches)");
		BeginBlock();
			for (vector< Ref<Match> >::const_iterator i = m_parser->GetMatches().begin(); i != m_parser->GetMatches().end(); ++i)
				GenerateMatchCode(*i);
		EndBlock();

		WriteLine("void ProcessPossibleMatch(MatchState& state, queue<MatchState>& possible, vector<MatchState>& matches)");
		BeginBlock();
			WriteLine("queue< Ref<TreeNode> > nodes;");
			WriteLine("nodes.push(state.node);");

			WriteLine("while (nodes.size() > 0)");
			BeginBlock();
				WriteLine("Ref<TreeNode> node = nodes.front();");
				WriteLine("nodes.pop();");

				WriteLine("MatchAndReduceSingleNode(state, node, possible, matches);");

				WriteLine("for (vector< Ref<TreeNode> >::const_iterator i = node->GetChildNodes().begin();");
				WriteLine("\ti != node->GetChildNodes().end(); ++i)");
					WriteLine("nodes.push(*i);");
			EndBlock();
		EndBlock();

		WriteLine("bool GenerateInstructions(SymInstrBlock* out, TreeNode* node)");
		BeginBlock();
			WriteLine("vector<MatchState> matches;");
			WriteLine("queue<MatchState> possible;");
			WriteLine("MatchState state;");
			WriteLine("state.cost = 0;");
			WriteLine("state.node = node;");
			WriteLine("possible.push(state);");

			WriteLine("if (m_settings.internalDebug)");
			BeginBlock();
				WriteLine("fprintf(stderr, \"Matching IL: \");");
				WriteLine("node->Print();");
				WriteLine("fprintf(stderr, \"\\n\");");
			EndBlock();

			// Match nodes until there are no more possible matches
			WriteLine("while (possible.size() != 0)");
			BeginBlock();
				WriteLine("state = possible.front();");
				WriteLine("possible.pop();");

				WriteLine("ProcessPossibleMatch(state, possible, matches);");
			EndBlock();

			WriteLine("if (matches.size() == 0)");
			WriteLine("\treturn false;");

			WriteLine("MatchState* best = NULL;");
			WriteLine("if (m_settings.polymorph)");
			BeginBlock();
				// Polymorph enabled, pick a random match
				WriteLine("best = &matches[rand() %% matches.size()];");
			EndBlock();
			WriteLine("else");
			BeginBlock();
				// Pick the smallest node
				WriteLine("for (vector<MatchState>::iterator i = matches.begin(); i != matches.end(); ++i)");
				BeginBlock();
					WriteLine("if (!best)");
					WriteLine("\tbest = &*i;");
					WriteLine("else if (i->cost < best->cost)");
					WriteLine("\tbest = &*i;");
				EndBlock();
			EndBlock();

			WriteLine("if (m_settings.internalDebug)");
			BeginBlock();
				WriteLine("fprintf(stderr, \"Selected: \");");
				WriteLine("for (vector<MatchInfo>::iterator i = best->matches.begin(); i != best->matches.end(); ++i)");
				BeginBlock();
					WriteLine("if (i != best->matches.begin())");
					WriteLine("\tfprintf(stderr, \", \");");
					WriteLine("fprintf(stderr, \"%%d\", (int)i->rule);");
				EndBlock();
				WriteLine("fprintf(stderr, \"\\n\");");
			EndBlock();

			// Add any temporary registers needed to the function
			WriteLine("for (vector<RegisterInfo>::iterator i = best->regs.begin(); i != best->regs.end(); ++i)");
			WriteLine("\tm_symFunc->AddRegister(i->cls);");

			// Output code for each rule selected
			WriteLine("for (vector<MatchInfo>::iterator i = best->matches.begin(); i != best->matches.end(); ++i)");
			BeginBlock();
				WriteLine("if (!OutputRule(out, i->rule, i->match, i->result, i->temps))");
				BeginBlock();
					WriteLine("fprintf(stderr, \"internal error: generated rule that does not exist\\n\");");
					WriteLine("return false;");
				EndBlock();
			EndBlock();

			WriteLine("return true;");
		EndBlock();

		WriteLine("bool GenerateCodeBlock(SymInstrBlock* out, TreeBlock* block)");
		BeginBlock();
			WriteLine("m_currentBlock = block;");

			WriteLine("vector< Ref<TreeNode> >::const_iterator i;");
			WriteLine("for (i = block->GetNodes().begin(); i != block->GetNodes().end(); ++i)");
			BeginBlock();
				WriteLine("if (m_settings.antiDisasm && ((rand() %% m_settings.antiDisasmFrequency) == 0))");
				WriteLine("\tGenerateAntiDisassembly(out);");

				WriteLine("bool end = false;");
				WriteLine("switch ((*i)->GetClass())");
				BeginBlock();
					WriteLine("case NODE_NOP:");
					WriteLine("\tbreak;");
					WriteLine("case NODE_NORETURN:");
					WriteLine("\tend = true;");
					WriteLine("\tbreak;");
					WriteLine("case NODE_IFTRUE:");
					WriteLine("case NODE_IFSLT:");
					WriteLine("case NODE_IFULT:");
					WriteLine("case NODE_IFSLE:");
					WriteLine("case NODE_IFULE:");
					WriteLine("case NODE_IFE:");
					WriteLine("case NODE_GOTO:");
					WriteLine("case NODE_RETURN:");
					WriteLine("case NODE_RETURNVOID:");
					WriteLine("\tend = true;");
					WriteLine("default:");
					WriteLine("\tif (!GenerateInstructions(out, *i))");
					WriteLine("\t\tgoto fail;");
					WriteLine("\tbreak;");
				EndBlock();

				WriteLine("if (end)");
				WriteLine("\tbreak;");
			EndBlock();

			WriteLine("return true;");

			WriteUnindented("fail:");
			WriteLine("fprintf(stderr, \"error: unable to generate code for IL: \");");
			WriteLine("(*i)->Print();");
			WriteLine("fprintf(stderr, \"\\n\");");
			WriteLine("return false;");
		EndBlock();

		WriteUnindented("public:");

		WriteLine("%s(const Settings& settings, Function* startFunc): Output(settings, startFunc)", m_className.c_str());
		BeginBlock();
		EndBlock();

		WriteLine("virtual bool GenerateCode(Function* func)");
		BeginBlock();
			WriteLine("%s_SymInstrFunction symFunc(m_settings, func);", m_parser->GetArchName().c_str());
			WriteLine("m_func = func;");
			WriteLine("m_symFunc = &symFunc;");

			WriteLine("m_vars.function = &symFunc;");
			WriteLine("m_vars.stackVariables.clear();");
			WriteLine("m_vars.registerVariables.clear();");
			WriteLine("m_vars.highRegisterVariables.clear();");

			WriteLine("symFunc.InitializeBlocks(func);");

			// TODO: Stack variable system does not yet support frames without a frame pointer
			WriteLine("m_framePointerEnabled = true;");
			WriteLine("m_vars.stackVariableBase = SYMREG_BP;");

			if (m_parser->GetSpecialRegs().find("SYMREG_IP") == m_parser->GetSpecialRegs().end())
				WriteLine("m_globalBaseReg = SYMREG_NONE;");
			else
				WriteLine("m_globalBaseReg = SYMREG_IP;");

			// Generate stack frame
			WriteLine("m_paramCopy.clear();");
			WriteLine("for (vector< Ref<Variable> >::const_iterator i = m_func->GetVariables().begin();");
			WriteLine("\ti != m_func->GetVariables().end(); i++)");
			BeginBlock();
				WriteLine("if ((*i)->IsParameter())");
				WriteLine("\tcontinue;");

				WriteLine("if (((*i)->GetType()->GetClass() != TYPE_STRUCT) && ((*i)->GetType()->GetClass() != TYPE_ARRAY))");
				BeginBlock();
					// If the variable has its address taken, it cannot be stored in a register
					WriteLine("bool addressTaken = false;");
					WriteLine("for (vector<ILBlock*>::const_iterator j = m_func->GetIL().begin(); j != m_func->GetIL().end(); j++)");
					BeginBlock();
						WriteLine("for (vector<ILInstruction>::const_iterator k = (*j)->GetInstructions().begin();");
						WriteLine("\tk != (*j)->GetInstructions().end(); k++)");
						BeginBlock();
							WriteLine("if (k->operation != ILOP_ADDRESS_OF)");
							WriteLine("\tcontinue;");

							WriteLine("if (k->params[1].variable == *i)");
							BeginBlock();
								WriteLine("addressTaken = true;");
								WriteLine("break;");
							EndBlock();
						EndBlock();
					EndBlock();

					WriteLine("if (addressTaken)");
					BeginBlock();
						WriteLine("m_vars.stackVariables[*i] = m_symFunc->AddStackVar(0, false, (*i)->GetType()->GetWidth(),");
						WriteLine("\tILParameter::ReduceType((*i)->GetType()));");
						WriteLine("continue;");
					EndBlock();

					// Variable can be stored in a register
					WriteLine("AssignRegisterVariable(*i);");
				EndBlock();
				WriteLine("else");
				BeginBlock();
					// Structures and arrays cannot be stored in a register
					WriteLine("m_vars.stackVariables[*i] = m_symFunc->AddStackVar(0, false,");
					WriteLine("\t(*i)->GetType()->GetWidth(), ILTYPE_VOID);");
				EndBlock();
			EndBlock();

			WriteLine("AssignParameters();");

			// Generate function start
			WriteLine("SymInstrBlock* startOutput = m_symFunc->GetBlock(func->GetIL()[0]);");
			WriteLine("if (!GenerateFunctionStart(startOutput))");
			WriteLine("\treturn false;");

			// Generate tree IL for code generation
			WriteLine("if (!func->GenerateTreeIL(m_settings, m_vars, this))");
			BeginBlock();
				WriteLine("fprintf(stderr, \"error: unable to generate tree IL for function '%%s'\\n\",");
				WriteLine("\tfunc->GetName().c_str());");
				WriteLine("return false;");
			EndBlock();

			// Generate code
			WriteLine("for (vector< Ref<TreeBlock> >::const_iterator i = func->GetTreeIL().begin();");
			WriteLine("\ti != func->GetTreeIL().end(); i++)");
			BeginBlock();
				WriteLine("SymInstrBlock* out = m_symFunc->GetBlock((*i)->GetSource());");
				WriteLine("if (!GenerateCodeBlock(out, *i))");
				WriteLine("\treturn false;");
			EndBlock();

			WriteLine("if (m_settings.internalDebug)");
			WriteLine("\tfprintf(stderr, \"\\n%%s:\\n\", func->GetName().c_str());");

			// Allocate registers for symbolic code to produce final assembly
			WriteLine("if (!m_symFunc->AllocateRegisters())");
			BeginBlock();
				WriteLine("if (m_settings.internalDebug)");
				BeginBlock();
					WriteLine("fprintf(stderr, \"\\n%%s:\\n\", func->GetName().c_str());");
					WriteLine("m_symFunc->Print();");
				EndBlock();
				WriteLine("return false;");
			EndBlock();

			// Emit machine code for each block
			WriteLine("for (vector<ILBlock*>::const_iterator i = m_func->GetIL().begin(); i != m_func->GetIL().end(); i++)");
			BeginBlock();
				WriteLine("OutputBlock* out = new OutputBlock;");
				WriteLine("out->code = NULL;");
				WriteLine("out->len = 0;");
				WriteLine("out->maxLen = 0;");
				WriteLine("out->randomLen = 0;");
				WriteLine("out->bigEndian = m_settings.bigEndian;");

				WriteLine("if (!m_symFunc->GetBlock(*i)->EmitCode(m_symFunc, out))");
				BeginBlock();
					WriteLine("delete out;");
					WriteLine("return false;");
				EndBlock();

				WriteLine("(*i)->SetOutputBlock(out);");
			EndBlock();

			WriteLine("if (m_settings.internalDebug)");
			BeginBlock();
				WriteLine("fprintf(stderr, \"\\n%%s:\\n\", func->GetName().c_str());");
				WriteLine("m_symFunc->Print();");
			EndBlock();

			WriteLine("return true;");
		EndBlock();
	EndBlockSemicolon();

	WriteLine("Output* Create%s(const Settings& settings, Function* startFunc)", m_className.c_str());
	BeginBlock();
		WriteLine("return new %s(settings, startFunc);", m_className.c_str());
	EndBlock();

	output = m_output;
	return m_errors == 0;
}

