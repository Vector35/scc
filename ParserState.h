#ifndef __PARSERSTATE_H__
#define __PARSERSTATE_H__

#include "Enum.h"
#include "Expr.h"
#include "Function.h"
#include "Scope.h"
#include "Settings.h"
#include "Struct.h"
#include "Type.h"
#include <map>
#include <string>
#include <vector>


struct FunctionInfo;

struct VarInitInfo
{
	std::string name;
	bool initialized;
	Ref<Expr> value;
	bool array;
	size_t elements;
	Location location;
};

struct DuplicateContext
{
	std::map<Type*, Ref<Type>> types;
	std::map<Struct*, Ref<Struct>> structs;
	std::map<Enum*, Ref<Enum>> enums;
	std::map<Variable*, Ref<Variable>> vars;
	std::map<Function*, Ref<Function>> funcs;
};


class ParserState
{
	std::string m_fileName;
	int m_line;
	void* m_scanner;
	void* m_lvalue;
	int m_errors;

	std::map<std::string, Ref<Type>> m_types;
	std::map<std::string, Ref<Type>> m_structTypes;
	std::map<std::string, Ref<Type>> m_unionTypes;
	std::map<std::string, Ref<Type>> m_enumTypes;
	std::map<std::string, uint32_t> m_enumMembers;

	Ref<Expr> m_initExpression;
	std::map<std::string, Ref<Function>> m_functions;
	std::map<std::string, uint64_t> m_funcAddrs, m_funcPtrs;

	std::map<float, Variable*> m_floatImmed;
	std::map<double, Variable*> m_doubleImmed;

	Scope* m_globalScope;
	Scope* m_currentScope;

	Settings m_settings;

 public:
	ParserState(const Settings& settings, const std::string& name, void* scanner);
	ParserState(ParserState* parent, const std::string& name, void* scanner);
	~ParserState();

	const std::string& GetFileName() const { return m_fileName; }
	int GetLineNumber() const { return m_line; }
	Location GetLocation();
	void SetLocation(const std::string& name, int line)
	{
		m_fileName = name;
		m_line = line;
	}

	void* GetScanner() { return m_scanner; }
	void SetScanner(void* scanner) { m_scanner = scanner; }
	void* GetLValue() const { return m_lvalue; }
	void SetLValue(void* lvalue) { m_lvalue = lvalue; }

	void Error() { m_errors++; }
	bool HasErrors() const { return m_errors != 0; }

	Expr* GetInitExpression() const { return m_initExpression; }
	void SetInitExpression(Expr* expr) { m_initExpression = expr; }
	const std::map<std::string, Ref<Function>>& GetFunctions() const { return m_functions; }
	Scope* GetGlobalScope() const { return m_globalScope; }

	static std::string ProcessEscapedString(const std::string& str);
	static int64_t CharStringToValue(const std::string& str);

	void DefineType(const std::string& name, Type* type);
	bool IsTypeDefined(const std::string& name) const { return m_types.find(name) != m_types.end(); }
	Type* GetType(const std::string& name);

	void DefineStructType(const std::string& name, Type* type);
	void DefineUnionType(const std::string& name, Type* type);
	void DefineEnumType(const std::string& name, Type* type);
	Type* GetStructType(const std::string& name);
	Type* GetUnionType(const std::string& name);
	Type* GetEnumType(const std::string& name);

	void AddEnumMember(const std::string& name, uint32_t value);
	bool IsEnumMember(const std::string& name)
	{
		return m_enumMembers.find(name) != m_enumMembers.end();
	}
	uint32_t GetEnumMemberValue(const std::string& name);

	void BeginFunctionScope(const FunctionInfo& func);
	void PushScope();
	void PopScope();

	Expr* DeclareVariable(Type* type, const VarInitInfo& info, bool isStatic, bool isExtern);

	void DefineFunction(FunctionInfo& func, Expr* body, bool isLocalScope);
	void DefineFunctionPrototype(FunctionInfo& func, bool isLocalScope);
	void SetFunctionFixedAddress(const std::string& name, uint64_t addr);
	void SetFunctionFixedPointer(const std::string& name, uint64_t addr);

	Expr* ResolveIdentifierExpr(const std::string& name);

	void AddInitExpression(Expr* expr);

	bool HasIntrinsicStrlen();
	bool HasIntrinsicMemcpy();
	bool HasIntrinsicMemset();
	bool HasIntrinsicDivide64();
	bool HasIntrinsicShift64();
	bool HasIntrinsicPow();
	bool HasIntrinsicByteSwap();
	bool HasIntrinsicDivide();

	bool IsValidFloatImmediate(double value);
	Variable* GetFloatImmediateVariable(Type* type, double value);

	const std::map<std::string, uint64_t>& GetFixedFunctionAddresses() const { return m_funcAddrs; }
	const std::map<std::string, uint64_t>& GetFixedFunctionPointers() const { return m_funcPtrs; }

	Expr* BasicExpr(ExprClass cls) { return new Expr(GetLocation(), cls); }
	Expr* BoolExpr(bool value) { return Expr::BoolExpr(GetLocation(), value); }
	Expr* IntExpr(int64_t value) { return Expr::IntExpr(GetLocation(), value); }
	Expr* Int64Expr(int64_t value) { return Expr::Int64Expr(GetLocation(), value); }
	Expr* FloatExpr(double value, size_t size) { return Expr::FloatExpr(GetLocation(), value, size); }
	Expr* StringExpr(const std::string& value) { return Expr::StringExpr(GetLocation(), value); }
	Expr* VariableExpr(Variable* var) { return Expr::VariableExpr(GetLocation(), var); }
	Expr* FunctionExpr(Function* func) { return Expr::FunctionExpr(GetLocation(), func); }
	Expr* DotExpr(Expr* left, const std::string& right)
	{
		return Expr::DotExpr(GetLocation(), left, right);
	}
	Expr* ArrowExpr(Expr* left, const std::string& right)
	{
		return Expr::ArrowExpr(GetLocation(), left, right);
	}
	Expr* UnaryExpr(ExprClass cls, Expr* expr) { return Expr::UnaryExpr(GetLocation(), cls, expr); }
	Expr* BinaryExpr(ExprClass cls, Expr* left, Expr* right)
	{
		return Expr::BinaryExpr(GetLocation(), cls, left, right);
	}
	Expr* IfExpr(Expr* cond, Expr* ifTrue) { return Expr::IfExpr(GetLocation(), cond, ifTrue); }
	Expr* IfElseExpr(Expr* cond, Expr* ifTrue, Expr* ifFalse)
	{
		return Expr::IfElseExpr(GetLocation(), cond, ifTrue, ifFalse);
	}
	Expr* IfElseValueExpr(Expr* cond, Expr* ifTrue, Expr* ifFalse)
	{
		return Expr::IfElseValueExpr(GetLocation(), cond, ifTrue, ifFalse);
	}
	Expr* ForExpr(Expr* init, Expr* cond, Expr* update, Expr* body)
	{
		return Expr::ForExpr(GetLocation(), init, cond, update, body);
	}
	Expr* WhileExpr(Expr* cond, Expr* body) { return Expr::WhileExpr(GetLocation(), cond, body); }
	Expr* DoWhileExpr(Expr* cond, Expr* body) { return Expr::DoWhileExpr(GetLocation(), cond, body); }
	Expr* CallExpr(Expr* func, const std::vector<Ref<Expr>>& params)
	{
		return Expr::CallExpr(GetLocation(), func, params);
	}
	Expr* BuiltinCallExpr(ExprClass cls, const std::vector<Ref<Expr>>& params)
	{
		return Expr::BuiltinCallExpr(GetLocation(), cls, params);
	}
	Expr* CastExpr(Type* type, Expr* value) { return Expr::CastExpr(GetLocation(), type, value); }
	Expr* LabelExpr(const std::string& value) { return Expr::LabelExpr(GetLocation(), value); }
	Expr* GotoLabelExpr(const std::string& value)
	{
		return Expr::GotoLabelExpr(GetLocation(), value);
	}

	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);

#ifndef WIN32
	void Print();
#endif
};


#endif
