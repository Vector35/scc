#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stack>
#include <map>
#include "Type.h"
#include "Expr.h"
#include "Variable.h"
#include "BitVector.h"


enum SubarchitectureType
{
	SUBARCH_DEFAULT,
	SUBARCH_X86,
	SUBARCH_X64
};

struct FunctionInfo
{
	Ref<Type> returnValue;
	CallingConvention callingConvention;
	std::string name, module;
	std::vector< std::pair< Ref<Type>, std::string > > params;
	SubarchitectureType subarch;
	bool noReturn, imported;
	Location location;

	void CombineFunctionAttributes(const FunctionInfo& other);
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

enum ParameterLocationType
{
	PARAM_STACK,
	PARAM_REG
};

struct ParameterLocation
{
	ParameterLocationType type;
	uint32_t reg;
};

class SymInstrFunction;

struct VariableAssignments
{
	uint32_t stackVariableBase;
	std::map<Variable*, int32_t> stackVariables;
	std::map<Variable*, uint32_t> registerVariables;
	std::map<Variable*, uint32_t> highRegisterVariables;
	SymInstrFunction* function;
};

class TreeBlock;
class Output;

class Function: public RefCountObject
{
	Ref<Type> m_returnValue;
	CallingConvention m_callingConvention;
	SubarchitectureType m_subarch;
	std::vector<ParameterLocation> m_paramLocations;
	bool m_returns;
	std::string m_name;
	std::vector<FunctionParameter> m_params;
	bool m_variableArguments;
	Location m_location;
	std::vector< Ref<Variable> > m_vars;
	Ref<Expr> m_body;
	std::vector<ILBlock*> m_ilBlocks;
	std::vector< Ref<TreeBlock> > m_treeBlocks;
	std::map<std::string, ILBlock*> m_labels;
	std::stack<ILBlock*> m_breakStack, m_continueStack, m_defaultStack;
	std::map<int64_t, ILBlock*> m_switchLabels;
	std::stack< std::map<int64_t, ILBlock*> > m_switchLabelsStack;
	std::map< std::string, std::vector<LabelFixup> > m_labelFixups;
	ILBlock* m_defaultBlock;
	uint32_t m_nextTempId;
	bool m_localScope;
	bool m_variableSizedStackFrame;
	bool m_imported;
	std::string m_importModule;

	std::set<ILBlock*> m_exitBlocks;
	BitVector m_exitReachingDefs;
	std::map< Ref<Variable>, std::vector<size_t> > m_varDefs;
	std::vector< std::pair<ILBlock*, size_t> > m_defLocs;

	size_t m_tagCount;

	bool m_isFixedAddress, m_isFixedAddressDeref;
	uint64_t m_fixedAddress;

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

	void SetSubarchitecture(SubarchitectureType type) { m_subarch = type; }
	void SetDoesReturn(bool returns) { m_returns = returns; }

	Type* GetReturnValue() const { return m_returnValue; }
	CallingConvention GetCallingConvention() const { return m_callingConvention; }
	SubarchitectureType GetSubarchitecture() const { return m_subarch; }
	void SetParameterLocations(const std::vector<ParameterLocation>& locs) { m_paramLocations = locs; }
	ParameterLocation GetParameterLocation(size_t i) const;
	bool DoesReturn() const { return m_returns; }
	const std::string& GetName() const { return m_name; }
	const std::vector<FunctionParameter>& GetParameters() const { return m_params; }
	bool HasVariableArguments() const { return m_variableArguments; }
	Location GetLocation() const { return m_location; }
	const std::vector< Ref<Variable> >& GetVariables() { return m_vars; }
	Expr* GetBody() const { return m_body; }

	bool IsFullyDefined() const { return (m_body != NULL) || m_isFixedAddress; }
	bool IsLocalScope() const { return m_localScope; }
	bool IsCompatible(const FunctionInfo& info);
	bool IsCompatible(Type* returnValue, CallingConvention callingConvention,
		const std::vector< std::pair< Ref<Type>, std::string > >& params,
		bool variableArguments);

	Type* GetType() const;

	const std::vector<ILBlock*>& GetIL() const { return m_ilBlocks; }
	const std::vector< Ref<TreeBlock> >& GetTreeIL() const { return m_treeBlocks; }
	void GenerateIL(ParserState* state);
	bool GenerateTreeIL(const Settings& settings, const VariableAssignments& vars, Output* output);
	ILBlock* CreateILBlock();
	void RemoveILBlock(ILBlock* block);
	ILParameter CreateTempVariable(Type* type);
	void AddVariable(Variable* var) { m_vars.push_back(var); }

	bool IsVariableSizedStackFrame() const { return m_variableSizedStackFrame; }
	void MarkVariableSizedStackFrame() { m_variableSizedStackFrame = true; }
	size_t GetApproxStackFrameSize();

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
	void CheckForVariableWrites();

	void ClearExitBlocks() { m_exitBlocks.clear(); }
	void AddExitBlock(ILBlock* block) { m_exitBlocks.insert(block); }
	void RemoveExitBlock(ILBlock* block) { m_exitBlocks.erase(block); }
	const std::set<ILBlock*>& GetExitBlocks() { return m_exitBlocks; }

	BitVector& GetExitReachingDefinitions() { return m_exitReachingDefs; }
	const std::map< Ref<Variable>, std::vector<size_t> >& GetLocalVariableDefinitions() const { return m_varDefs; }
	const std::vector< std::pair<ILBlock*, size_t> >& GetDefinitionLocations() const { return m_defLocs; }
	void SetDefinitions(const std::map< Ref<Variable>, std::vector<size_t> >& varDefs,
		const std::vector< std::pair<ILBlock*, size_t> >& defLocs);

	void ResetTagCount() { m_tagCount = 0; }
	size_t GetTagCount() const { return m_tagCount; }
	void TagReferences();

	bool IsImportedFunction() const { return m_imported; }
	const std::string& GetImportModule() const { return m_importModule; }

	void ReplaceWithFixedAddress(uint64_t addr);
	void ReplaceWithFixedPointer(uint64_t addr);
	bool IsFixedAddress() const { return m_isFixedAddress; }
	bool IsFixedAddressDeref() const { return m_isFixedAddressDeref; }
	uint64_t GetFixedAddress() const { return m_fixedAddress; }

	void Serialize(OutputBlock* output);
	static Function* Deserialize(InputBlock* input);

#ifndef WIN32
	void Print();
	void PrintPrototype();
#endif
};


#endif

