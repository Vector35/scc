#ifndef __FUNCTION__
#define __FUNCTION__

#include <stack>
#include <map>
#include "Type.h"
#include "Expr.h"
#include "Variable.h"


struct FunctionInfo
{
	Ref<Type> returnValue;
	CallingConvention callingConvention;
	std::string name;
	std::vector< std::pair< Ref<Type>, std::string > > params;
	Location location;
};

struct FunctionParameter
{
	Ref<Type> type;
	std::string name;
};

struct LabelFixup
{
	ILBlock* block;
	size_t index;
	Location location;
};


class Function: public RefCountObject
{
	Ref<Type> m_returnValue;
	CallingConvention m_callingConvention;
	std::string m_name;
	std::vector<FunctionParameter> m_params;
	bool m_variableArguments;
	Location m_location;
	std::vector< Ref<Variable> > m_vars;
	Ref<Expr> m_body;
	std::vector<ILBlock*> m_ilBlocks;
	std::map<std::string, ILBlock*> m_labels;
	std::stack<ILBlock*> m_breakStack, m_continueStack, m_defaultStack;
	std::map<int64_t, ILBlock*> m_switchLabels;
	std::stack< std::map<int64_t, ILBlock*> > m_switchLabelsStack;
	std::map< std::string, std::vector<LabelFixup> > m_labelFixups;
	ILBlock* m_defaultBlock;
	uint32_t m_nextTempId;
	bool m_localScope;
	bool m_variableSizedStackFrame;

	size_t m_tagCount;

	size_t m_serializationIndex;
	bool m_serializationIndexValid;
	static size_t m_nextSerializationIndex;
	static std::map< size_t, Ref<Function> > m_serializationMap;

	bool DeserializeInternal(InputBlock* input);

public:
	Function();
	Function(const FunctionInfo& info, bool isLocalScope);
	Function(const FunctionInfo& info, const std::vector< Ref<Variable> >& vars, Expr* body, bool isLocalScope);
	virtual ~Function();

	Function* Duplicate(DuplicateContext& dup);

	void SetVariables(const std::vector< Ref<Variable> >& vars) { m_vars = vars; }
	void SetBody(Expr* body) { m_body = body; }
	void SetLocation(const Location& loc) { m_location = loc; }

	Type* GetReturnValue() const { return m_returnValue; }
	CallingConvention GetCallingConvention() const { return m_callingConvention; }
	const std::string& GetName() const { return m_name; }
	const std::vector<FunctionParameter>& GetParameters() const { return m_params; }
	bool HasVariableArguments() const { return m_variableArguments; }
	Location GetLocation() const { return m_location; }
	const std::vector< Ref<Variable> >& GetVariables() { return m_vars; }
	Expr* GetBody() const { return m_body; }

	bool IsFullyDefined() const { return m_body != NULL; }
	bool IsLocalScope() const { return m_localScope; }
	bool IsCompatible(const FunctionInfo& info);
	bool IsCompatible(Type* returnValue, CallingConvention callingConvention,
		const std::vector< std::pair< Ref<Type>, std::string > >& params,
		bool variableArguments);

	Type* GetType() const;

	const std::vector<ILBlock*>& GetIL() const { return m_ilBlocks; }
	void GenerateIL(ParserState* state);
	ILBlock* CreateILBlock();
	ILParameter CreateTempVariable(Type* type);

	bool IsVariableSizedStackFrame() const { return m_variableSizedStackFrame; }
	void MarkVariableSizedStackFrame() { m_variableSizedStackFrame = true; }

	void SetLabel(const std::string& name, ILBlock* block);
	ILBlock* GetLabel(const std::string& name) const;

	void AddLabelFixup(ILBlock* block, size_t i, const Location& loc, const std::string& name);
	void ReportUndefinedLabels(ParserState* state);

	ILBlock* GetBreakBlock() const;
	ILBlock* GetContinueBlock() const;
	void PushBreakBlock(ILBlock* block) { m_breakStack.push(block); }
	void PushContinueBlock(ILBlock* block) { m_continueStack.push(block); }
	void PopBreakBlock() { m_breakStack.pop(); }
	void PopContinueBlock() { m_continueStack.pop(); }

	void PushSwitchLabels();
	void SetDefaultBlock(ILBlock* block) { m_defaultBlock = block; }
	ILBlock* GetDefaultBlock() const { return m_defaultBlock; }
	void AddSwitchLabel(int64_t value, ILBlock* block) { m_switchLabels[value] = block; }
	const std::map<int64_t, ILBlock*>& GetSwitchLabels() const { return m_switchLabels; }
	void PopSwitchLabels();

	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);

	void CheckForUndefinedReferences(size_t& errors);

	void ResetTagCount() { m_tagCount = 0; }
	size_t GetTagCount() const { return m_tagCount; }
	void TagReferences();

	void Serialize(OutputBlock* output);
	static Function* Deserialize(InputBlock* input);

#ifndef WIN32
	void Print();
#endif
};


#endif

