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


void OutputGenerator::WriteCodeBlock(CodeBlock* code)
{
	string result;

	for (vector<CodeToken>::const_iterator i = code->GetTokens().begin(); i != code->GetTokens().end(); ++i)
	{
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
			}
			else if (i->name == "}")
			{
				if (result.size() != 0)
				{
					WriteLine("%s", result.c_str());
					result = "";
				}
				EndBlock();
			}
			else if (i->name == ";")
			{
				result += ";";
				WriteLine("%s", result.c_str());
				result = "";
			}
			else
			{
				result += i->name;
				result += " ";
			}
			break;
		case TOKEN_INSTR_START:
			result += "out->AddInstruction(";
			result += m_parser->GetArchName();
			result += i->name;
			result += "(";
			break;
		case TOKEN_INSTR_END:
			result += "))";
			break;
		default:
			fprintf(stderr, "error: invalid token in code block\n");
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


string OutputGenerator::GenerateMatchForNode(const string& prefix, TreeNode* node)
{
	switch (node->GetClass())
	{
	case NODE_REG:
		if (node->GetTypeName() == "BLOCK")
			return prefix + "->GetClass() == NODE_BLOCK";
		else if (node->GetTypeName() == "IMM")
			return string("(") + prefix + "->GetClass() == NODE_IMMED)" + GenerateTypeMatchCode(prefix, node);
		else if (node->GetTypeName() == "STACKVAR")
			return string("(") + prefix + "->GetClass() == NODE_STACK_VAR)" + GenerateTypeMatchCode(prefix, node);
		else if (node->GetTypeName() == "GLOBALVAR")
			return string("(") + prefix + "->GetClass() == NODE_GLOBAL_VAR)" + GenerateTypeMatchCode(prefix, node);
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
				fprintf(stderr, "error: invalid register class '%s'\n", node->GetTypeName().c_str());
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
				fprintf(stderr, "error: invalid use of temporary register class '%s'\n", node->GetTypeName().c_str());
				m_errors++;
				return "false";
			}
		}

		fprintf(stderr, "error: invalid match type '%s'\n", node->GetTypeName().c_str());
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
	default:
		fprintf(stderr, "error: invalid node type in match\n");
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

		string code = GenerateMatchForNode(info.prefix, info.node);
		if (first)
			code = string("if ((") + code + string(")");
		else
			code = string("\t&& (") + code + string(")");
		first = false;
		WriteLine("%s", code.c_str());

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
	EndBlock();
}


bool OutputGenerator::Generate(string& output)
{
	WriteLine("#include \"Output.h\"");
	WriteLine("#include \"Struct.h\"");
	WriteLine("#include \"TreeBlock.h\"");
	WriteLine("#include \"%sSymInstr.h\"", m_parser->GetArchName().c_str());
	WriteLine("#include <stdio.h>");
	WriteLine("#include <stdlib.h>");
	WriteLine("#include <string.h>");
	WriteLine("#include <string>");
	WriteLine("#include <vector>");
	WriteLine("#include <map>");
	WriteLine("#include <set>");
	WriteLine("#include <queue>");

	WriteLine("#define TEMP_REGISTER(cls) m_symFunc->AddRegister(cls)");
	WriteLine("#define UNSAFE_STACK_PIVOT 0x1000");

	WriteLine("using namespace std;");

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
		EndBlockSemicolon();

		WriteLine("struct RegisterInfo");
		BeginBlock();
			WriteLine("uint32_t reg, cls;");
		EndBlockSemicolon();

		WriteLine("struct MatchState");
		BeginBlock();
			WriteLine("Ref<TreeNode> node;");
			WriteLine("vector<MatchInfo> matches;");
			WriteLine("vector<RegisterInfo> regs;");
		EndBlockSemicolon();

		WriteLine("Function* m_func;");
		WriteLine("%sSymInstrFunction* m_symFunc;", m_parser->GetArchName().c_str());
		WriteLine("TreeBlock* m_currentBlock;");
		WriteLine("VariableAssignments m_vars;");
		WriteLine("vector<IncomingParameterCopy> m_paramCopy;");
		WriteLine("bool m_framePointerEnabled;");

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

		// TODO: Add syntax for declaring compatible register classes (e.g. X86REGCLASS_EDX is a subset of X86REGCLASS_INTEGER)
		WriteLine("bool IsRegisterClassCompatible(uint32_t regClass, uint32_t matchClass)");
		BeginBlock();
			WriteLine("return regClass == matchClass;");
		EndBlock();

		WriteLine("uint32_t AddRegisterForMatch(MatchState& state, uint32_t cls)");
		BeginBlock();
			WriteLine("RegisterInfo info;");
			WriteLine("info.reg = m_symFunc->GetPossibleMatchRegister(state.regs.size());");
			WriteLine("info.cls = cls;");
			WriteLine("state.regs.push_back(info);");
			WriteLine("return info.reg;");
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
			WriteLine("state.node = node;");
			WriteLine("possible.push(state);");

			WriteLine("while (possible.size() != 0)");
			BeginBlock();
				WriteLine("state = possible.front();");
				WriteLine("possible.pop();");

				WriteLine("ProcessPossibleMatch(state, possible, matches);");
			EndBlock();
		
			WriteLine("return false;");
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
					WriteLine("case NODE_IFTRUE:");
					WriteLine("case NODE_IFSLT:");
					WriteLine("case NODE_IFULT:");
					WriteLine("case NODE_IFSLE:");
					WriteLine("case NODE_IFULE:");
					WriteLine("case NODE_IFE:");
					WriteLine("case NODE_GOTO:");
					WriteLine("case NODE_NORETURN:");
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
			WriteLine("%sSymInstrFunction symFunc(m_settings, func);", m_parser->GetArchName().c_str());
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

			// Generate stack frame
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

			// Generate tree IL for code generation
			WriteLine("if (!func->GenerateTreeIL(m_settings, m_vars))");
			BeginBlock();
				WriteLine("fprintf(stderr, \"error: unable to generate tree IL for function '%%s'\\n\",");
				WriteLine("\tfunc->GetName().c_str());");
				WriteLine("return false;");
			EndBlock();

			// Generate code
			WriteLine("bool first = true;");
			WriteLine("for (vector< Ref<TreeBlock> >::const_iterator i = func->GetTreeIL().begin();");
			WriteLine("\ti != func->GetTreeIL().end(); i++)");
			BeginBlock();
				WriteLine("SymInstrBlock* out = m_symFunc->GetBlock((*i)->GetSource());");

				WriteLine("if (first)");
				BeginBlock();
					WriteLine("if (!GenerateFunctionStart(out))");
					WriteLine("\treturn false;");
					WriteLine("first = false;");
				EndBlock();

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

