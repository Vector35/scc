// Copyright (c) 2011-2012 Rusty Wagner
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

#ifndef __EXPR_H__
#define __EXPR_H__

#include "Type.h"
#include "ILBlock.h"


enum ExprClass
{
	EXPR_SEQUENCE,
	EXPR_INITIALIZER,
	EXPR_INT,
	EXPR_FLOAT,
	EXPR_STRING,
	EXPR_TRUE,
	EXPR_FALSE,
	EXPR_VARIABLE,
	EXPR_FUNCTION,
	EXPR_DOT,
	EXPR_ARROW,
	EXPR_ADDRESS_OF,
	EXPR_DEREF,
	EXPR_PRE_INCREMENT,
	EXPR_PRE_DECREMENT,
	EXPR_POST_INCREMENT,
	EXPR_POST_DECREMENT,
	EXPR_ARRAY_INDEX,
	EXPR_PLUS,
	EXPR_MINUS,
	EXPR_MULT,
	EXPR_DIV,
	EXPR_MOD,
	EXPR_AND,
	EXPR_OR,
	EXPR_XOR,
	EXPR_SHIFT_LEFT,
	EXPR_SHIFT_RIGHT,
	EXPR_NEG,
	EXPR_NOT,
	EXPR_LOGICAL_AND,
	EXPR_LOGICAL_OR,
	EXPR_LOGICAL_NOT,
	EXPR_LESS_THAN,
	EXPR_LESS_EQUAL,
	EXPR_EQUAL,
	EXPR_NOT_EQUAL,
	EXPR_GREATER_EQUAL,
	EXPR_GREATER_THAN,
	EXPR_ASSIGN,
	EXPR_INIT_ASSIGN,
	EXPR_IF,
	EXPR_IF_ELSE,
	EXPR_IF_ELSE_VALUE,
	EXPR_FOR,
	EXPR_WHILE,
	EXPR_DO_WHILE,
	EXPR_CALL,
	EXPR_MIN,
	EXPR_MAX,
	EXPR_ABS,
	EXPR_ALLOCA,
	EXPR_MEMCPY,
	EXPR_MEMSET,
	EXPR_STRLEN,
	EXPR_CAST,
	EXPR_RETURN,
	EXPR_RETURN_VOID,
	EXPR_LABEL,
	EXPR_GOTO_LABEL,
	EXPR_COMPUTED_GOTO,
	EXPR_BREAK,
	EXPR_CONTINUE,
	EXPR_SWITCH,
	EXPR_CASE,
	EXPR_DEFAULT,
	EXPR_UNDEFINED,
	EXPR_SYSCALL,
	EXPR_SYSCALL2,
	EXPR_RDTSC,
	EXPR_RDTSC_LOW,
	EXPR_RDTSC_HIGH,
	EXPR_NEXT_ARG,
	EXPR_PREV_ARG,
	EXPR_BYTESWAP
};

struct Location
{
	std::string fileName;
	int lineNumber;
};


class ParserState;
class Variable;
class Function;

class Expr: public RefCountObject
{
	ExprClass m_class;
	Location m_location;
	int64_t m_intValue;
	double m_floatValue;
	std::string m_stringValue;
	std::vector< Ref<Expr> > m_children;
	Ref<Variable> m_variable;
	Ref<Function> m_function;

	Ref<Type> m_type;

	bool DeserializeInternal(InputBlock* input);

public:
	Expr(ExprClass cls);
	Expr(const Location& loc, ExprClass cls);

	Expr* Duplicate(DuplicateContext& dup);

	void SetClass(ExprClass cls) { m_class = cls; }
	void SetIntValue(int64_t value) { m_intValue = value; }
	void SetFloatValue(double value) { m_floatValue = value; }
	void SetStringValue(const std::string& value) { m_stringValue = value; }
	void AddChild(Expr* expr) { m_children.push_back(expr); }
	void CopyChildren(Expr* expr) { m_children.insert(m_children.end(), expr->m_children.begin(), expr->m_children.end()); }

	ExprClass GetClass() const { return m_class; }
	int64_t GetIntValue() const { return m_intValue; }
	double GetFloatValue() const { return m_floatValue; }
	const std::string& GetStringValue() const { return m_stringValue; }
	const std::vector< Ref<Expr> >& GetChildren() const { return m_children; }
	Variable* GetVariable() const;
	Function* GetFunction() const;

	void SetType(Type* type) { m_type = type; }
	Type* GetType() const { return m_type; }

	int64_t ComputeIntegerValue(ParserState* state);
	Type* ComputeType(ParserState* state, Function* func);
	Expr* Simplify(ParserState* state);
	Expr* ConvertToBool(ParserState* state);
	Type* PromotedType(ParserState* state, Expr* a, Expr* b);
	Expr* ConvertToType(ParserState* state, Type* type);
	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);

	void GenerateConditionalIL(ParserState* state, Function* func, ILBlock* block, ILBlock* trueBlock, ILBlock* falseBlock);
	ILParameter GenerateArrayAccessIL(ParserState* state, Function* func, ILBlock*& block);
	ILParameter GenerateIL(ParserState* state, Function* func, ILBlock*& block);

	static Expr* BoolExpr(const Location& loc, bool value);
	static Expr* IntExpr(const Location& loc, int64_t value);
	static Expr* FloatExpr(const Location& loc, double value);
	static Expr* StringExpr(const Location& loc, const std::string& value);
	static Expr* VariableExpr(const Location& loc, Variable* var);
	static Expr* FunctionExpr(const Location& loc, Function* func);
	static Expr* DotExpr(const Location& loc, Expr* left, const std::string& right);
	static Expr* ArrowExpr(const Location& loc, Expr* left, const std::string& right);
	static Expr* UnaryExpr(const Location& loc, ExprClass cls, Expr* expr);
	static Expr* BinaryExpr(const Location& loc, ExprClass cls, Expr* left, Expr* right);
	static Expr* IfExpr(const Location& loc, Expr* cond, Expr* ifTrue);
	static Expr* IfElseExpr(const Location& loc, Expr* cond, Expr* ifTrue, Expr* ifFalse);
	static Expr* IfElseValueExpr(const Location& loc, Expr* cond, Expr* ifTrue, Expr* ifFalse);
	static Expr* ForExpr(const Location& loc, Expr* init, Expr* cond, Expr* update, Expr* body);
	static Expr* WhileExpr(const Location& loc, Expr* cond, Expr* body);
	static Expr* DoWhileExpr(const Location& loc, Expr* cond, Expr* body);
	static Expr* CallExpr(const Location& loc, Expr* func, const std::vector< Ref<Expr> >& params);
	static Expr* BuiltinCallExpr(const Location& loc, ExprClass cls, const std::vector< Ref<Expr> >& params);
	static Expr* CastExpr(const Location& loc, Type* type, Expr* value);
	static Expr* LabelExpr(const Location& loc, const std::string& value);
	static Expr* GotoLabelExpr(const Location& loc, const std::string& value);

	void Serialize(OutputBlock* output);
	static Expr* Deserialize(InputBlock* input);

#ifndef WIN32
	void Print(size_t indent);
#endif
};


#endif

