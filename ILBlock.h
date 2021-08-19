#ifndef __ILBLOCK_H__
#define __ILBLOCK_H__

#include "BitVector.h"
#include "Type.h"
#include <set>
#include <stack>


typedef enum
{
	ILOP_ASSIGN,
	ILOP_ADDRESS_OF,
	ILOP_ADDRESS_OF_MEMBER,
	ILOP_DEREF,
	ILOP_DEREF_MEMBER,
	ILOP_DEREF_ASSIGN,
	ILOP_DEREF_MEMBER_ASSIGN,
	ILOP_ARRAY_INDEX,
	ILOP_ARRAY_INDEX_ASSIGN,
	ILOP_PTR_ADD,
	ILOP_PTR_SUB,
	ILOP_PTR_DIFF,
	ILOP_ADD,
	ILOP_SUB,
	ILOP_SMULT,
	ILOP_UMULT,
	ILOP_SDIV,
	ILOP_UDIV,
	ILOP_SMOD,
	ILOP_UMOD,
	ILOP_AND,
	ILOP_OR,
	ILOP_XOR,
	ILOP_SHL,
	ILOP_SHR,
	ILOP_SAR,
	ILOP_NEG,
	ILOP_NOT,
	ILOP_IF_TRUE,
	ILOP_IF_LESS_THAN,
	ILOP_IF_LESS_EQUAL,
	ILOP_IF_BELOW,
	ILOP_IF_BELOW_EQUAL,
	ILOP_IF_EQUAL,
	ILOP_GOTO,
	ILOP_CALL,
	ILOP_NORETURN,
	ILOP_SCONVERT,
	ILOP_UCONVERT,
	ILOP_RETURN,
	ILOP_RETURN_VOID,
	ILOP_ALLOCA,
	ILOP_MEMCPY,
	ILOP_MEMSET,
	ILOP_STRLEN,
	ILOP_SYSCALL,
	ILOP_SYSCALL2,
	ILOP_RDTSC,
	ILOP_RDTSC_LOW,
	ILOP_RDTSC_HIGH,
	ILOP_PEB,
	ILOP_TEB,
	ILOP_INITIAL_VARARG,
	ILOP_NEXT_ARG,
	ILOP_PREV_ARG,
	ILOP_BYTESWAP,
	ILOP_BREAKPOINT,
	ILOP_POW,
	ILOP_FLOOR,
	ILOP_CEIL,
	ILOP_SQRT,
	ILOP_SIN,
	ILOP_COS,
	ILOP_TAN,
	ILOP_ASIN,
	ILOP_ACOS,
	ILOP_ATAN
} ILOperation;

typedef enum
{
	ILPARAM_VOID,
	ILPARAM_INT,
	ILPARAM_FLOAT,
	ILPARAM_STRING,
	ILPARAM_FIELD,
	ILPARAM_BOOL,
	ILPARAM_VAR,
	ILPARAM_FUNC,
	ILPARAM_BLOCK,
	ILPARAM_MEMBER,
	ILPARAM_UNDEFINED
} ILParameterClass;

typedef enum
{
	ILTYPE_VOID,
	ILTYPE_INT8,
	ILTYPE_INT16,
	ILTYPE_INT32,
	ILTYPE_INT64,
	ILTYPE_FLOAT,
	ILTYPE_DOUBLE
} ILParameterType;


class Variable;
class Function;
class ILBlock;
struct RelocationReference;

struct ILParameter
{
	ILParameterClass cls;
	ILParameterType type;
	std::string stringValue;
	Ref<Struct> structure;
	Ref<Variable> variable;
	Ref<Function> function;
	union
	{
		int64_t integerValue;
		double floatValue;
		bool boolValue;
		ILParameter* parent;
		ILBlock* block;
	};

	ILParameter();
	ILParameter(bool b);
	ILParameter(ILParameterType t, int64_t i);
	ILParameter(Type* t, int64_t i);
	ILParameter(ILParameterType t, double f);
	ILParameter(Type* t, double f);
	ILParameter(const std::string& str);
	ILParameter(Struct* s, const std::string& name);
	ILParameter(Variable* var);
	ILParameter(ILParameterType t, Variable* var);
	ILParameter(Function* func);
	ILParameter(ILBlock* b);
	ILParameter(const ILParameter& obj, Type* objType, const std::string& str);
	ILParameter(const ILParameter& param);
	~ILParameter();
	ILParameter& operator=(const ILParameter& param);

	static ILParameterType ReduceType(Type* t);

	bool IsFloat() const { return (type == ILTYPE_FLOAT) || (type == ILTYPE_DOUBLE); }
	size_t GetWidth() const;
	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);
	void ConvertStringsToVariables(std::map<std::string, Ref<Variable>>& stringMap);
	void ResolveImportedFunction(Function* from, Variable* importTable);
	bool IsConstant() const;
	void TagReferences();
	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
	void Print() const;
};


struct ILInstruction
{
	ILOperation operation;
	std::vector<ILParameter> params;
	size_t dataFlowBit;

	ILInstruction();
	ILInstruction(ILOperation op);
	ILInstruction(ILOperation op, const ILParameter& a);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c,
	    const ILParameter& d);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c,
	    const ILParameter& d, const ILParameter& e);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c,
	    const ILParameter& d, const ILParameter& e, const ILParameter& f);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c,
	    const ILParameter& d, const ILParameter& e, const ILParameter& f, const ILParameter& g);
	ILInstruction(ILOperation op, const std::vector<ILParameter>& list);
	ILInstruction(const ILInstruction& instr);
	ILInstruction& operator=(const ILInstruction& instr);

	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);
	void ConvertStringsToVariables(std::map<std::string, Ref<Variable>>& stringMap);
	void ResolveImportedFunction(Function* from, Variable* importTable);
	void TagReferences();
	bool WritesToFirstParameter() const;
	void MarkWrittenVariables();
	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
	void Print() const;
};


struct OutputBlock;

class ILBlock
{
	size_t m_index, m_globalIndex;
	std::vector<ILInstruction> m_instrs;
	OutputBlock* m_output;
	uint64_t m_addr;
	bool m_blockEnded;

	std::set<ILBlock*> m_entryBlocks, m_exitBlocks;
	BitVector m_defPreserve, m_defGenerate, m_defReachIn, m_defReachOut;
	std::vector<std::vector<std::pair<ILBlock*, size_t>>> m_useDefChains;
	std::vector<std::vector<std::pair<ILBlock*, size_t>>> m_defUseChains;

	static std::map<size_t, ILBlock*> m_serializationMapping;
	static std::stack<std::map<size_t, ILBlock*>> m_savedSerializationMappings;

 public:
	ILBlock();
	ILBlock(size_t i);
	~ILBlock();

	size_t GetIndex() const { return m_index; }
	void SetIndex(size_t i) { m_index = i; }

	size_t GetGlobalIndex() const { return m_globalIndex; }
	void SetGlobalIndex(size_t i) { m_globalIndex = i; }

	std::vector<ILInstruction>& GetInstructions() { return m_instrs; }
	const std::vector<ILInstruction>& GetInstructions() const { return m_instrs; }
	void AddInstruction(const ILInstruction& instr);
	void AddInstruction(ILOperation op) { AddInstruction(ILInstruction(op)); }
	void AddInstruction(ILOperation op, const ILParameter& a)
	{
		AddInstruction(ILInstruction(op, a));
	}
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b)
	{
		AddInstruction(ILInstruction(op, a, b));
	}
	void AddInstruction(
	    ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c)
	{
		AddInstruction(ILInstruction(op, a, b, c));
	}
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b,
	    const ILParameter& c, const ILParameter& d)
	{
		AddInstruction(ILInstruction(op, a, b, c, d));
	}
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b,
	    const ILParameter& c, const ILParameter& d, const ILParameter& e)
	{
		AddInstruction(ILInstruction(op, a, b, c, d, e));
	}
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b,
	    const ILParameter& c, const ILParameter& d, const ILParameter& e, const ILParameter& f)
	{
		AddInstruction(ILInstruction(op, a, b, c, d, e, f));
	}
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b,
	    const ILParameter& c, const ILParameter& d, const ILParameter& e, const ILParameter& f,
	    const ILParameter& g)
	{
		AddInstruction(ILInstruction(op, a, b, c, d, e, f, g));
	}
	void AddInstruction(ILOperation op, const std::vector<ILParameter>& list)
	{
		AddInstruction(ILInstruction(op, list));
	}
	void SetInstructionParameter(size_t i, size_t param, const ILParameter& value)
	{
		m_instrs[i].params[param] = value;
	}
	void SetInstructionDataFlowBit(size_t i, size_t bit) { m_instrs[i].dataFlowBit = bit; }

	void RemoveLastInstruction();
	void SplitBlock(size_t firstToMove, ILBlock* target);

	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);
	void ConvertStringsToVariables(std::map<std::string, Ref<Variable>>& stringMap);
	void ResolveImportedFunction(Function* from, Variable* importTable);

	OutputBlock* GetOutputBlock() const { return m_output; }
	void SetOutputBlock(OutputBlock* output);
	uint64_t GetAddress() const { return m_addr; }
	void SetAddress(uint64_t addr) { m_addr = addr; }
	bool CheckRelocations(uint64_t codeSectionBase, uint64_t dataSectionBase,
	    std::vector<RelocationReference>& overflows);
	bool ResolveRelocations(uint64_t codeSectionBase, uint64_t dataSectionBase);

	bool EndsWithReturn() const;

	void TagReferences();

	void ClearEntryAndExitBlocks();
	void ClearExitBlocks();
	void AddEntryBlock(ILBlock* block) { m_entryBlocks.insert(block); }
	void AddExitBlock(ILBlock* block) { m_exitBlocks.insert(block); }
	void RemoveEntryBlock(ILBlock* block) { m_entryBlocks.erase(block); }
	void RemoveExitBlock(ILBlock* block) { m_exitBlocks.erase(block); }
	const std::set<ILBlock*>& GetEntryBlocks() const { return m_entryBlocks; }
	const std::set<ILBlock*>& GetExitBlocks() const { return m_exitBlocks; }

	void ResetDataFlowInfo(size_t bits);
	BitVector& GetPreservedDefinitions() { return m_defPreserve; }
	BitVector& GetGeneratedDefinitions() { return m_defGenerate; }
	BitVector& GetReachingDefinitionsInput() { return m_defReachIn; }
	BitVector& GetReachingDefinitionsOutput() { return m_defReachOut; }
	std::vector<std::vector<std::pair<ILBlock*, size_t>>>& GetUseDefinitionChains()
	{
		return m_useDefChains;
	}
	std::vector<std::vector<std::pair<ILBlock*, size_t>>>& GetDefinitionUseChains()
	{
		return m_defUseChains;
	}

	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
	static ILBlock* GetSerializationMapping(size_t i);
	static void SetSerializationMapping(size_t i, ILBlock* block);
	static void SaveSerializationMapping();
	static void RestoreSerializationMapping();

	void Print() const;
};


#endif
