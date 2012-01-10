#ifndef __ILBLOCK_H__
#define __ILBLOCK_H__

#include "Type.h"


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
	ILOP_MULT,
	ILOP_DIV,
	ILOP_MOD,
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
	ILOP_CONVERT,
	ILOP_RETURN,
	ILOP_RETURN_VOID,
	ILOP_ALLOCA,
	ILOP_MEMCPY,
	ILOP_MEMSET,
	ILOP_SYSCALL
} ILOperation;

typedef enum
{
	ILPARAM_VOID,
	ILPARAM_INT,
	ILPARAM_FLOAT,
	ILPARAM_STRING,
	ILPARAM_NAME,
	ILPARAM_BOOL,
	ILPARAM_VAR,
	ILPARAM_FUNC,
	ILPARAM_BLOCK,
	ILPARAM_MEMBER,
	ILPARAM_UNDEFINED
} ILParameterClass;


class Variable;
class Function;
class ILBlock;

struct ILParameter
{
	ILParameterClass cls;
	Ref<Type> type;
	std::string stringValue;
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
	ILParameter(Type* t, int64_t i);
	ILParameter(Type* t, double f);
	ILParameter(const std::string& str, ILParameterClass c = ILPARAM_STRING);
	ILParameter(Variable* var);
	ILParameter(Function* func);
	ILParameter(ILBlock* b);
	ILParameter(const ILParameter& obj, const std::string& str);
	ILParameter(const ILParameter& param);
	~ILParameter();
	ILParameter& operator=(const ILParameter& param);

	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);
	bool IsConstant() const;
	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
	void Print() const;
};


struct ILInstruction
{
	ILOperation operation;
	std::vector<ILParameter> params;
	size_t offset;

	ILInstruction();
	ILInstruction(ILOperation op);
	ILInstruction(ILOperation op, const ILParameter& a);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c);
	ILInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c, const ILParameter& d);
	ILInstruction(ILOperation op, const std::vector<ILParameter>& list);
	ILInstruction(const ILInstruction& instr);
	ILInstruction& operator=(const ILInstruction& instr);

	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);
	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
	void Print() const;
};


struct OutputBlock;

class ILBlock
{
	size_t m_index;
	std::vector<ILInstruction> m_instrs;
	OutputBlock* m_output;
	size_t m_addr;

	static std::map<size_t, ILBlock*> m_serializationMapping;

public:
	ILBlock();
	ILBlock(size_t i);
	~ILBlock();

	size_t GetIndex() const { return m_index; }
	void SetIndex(size_t i) { m_index = i; }

	std::vector<ILInstruction>& GetInstructions() { return m_instrs; }
	const std::vector<ILInstruction>& GetInstructions() const { return m_instrs; }
	void AddInstruction(const ILInstruction& instr);
	void AddInstruction(ILOperation op) { AddInstruction(ILInstruction(op)); }
	void AddInstruction(ILOperation op, const ILParameter& a) { AddInstruction(ILInstruction(op, a)); }
	void AddInstruction(ILOperation op, const ILParameter& a,
		const ILParameter& b) { AddInstruction(ILInstruction(op, a, b)); }
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b,
		const ILParameter& c) { AddInstruction(ILInstruction(op, a, b, c)); }
	void AddInstruction(ILOperation op, const ILParameter& a, const ILParameter& b, const ILParameter& c,
		const ILParameter& d) { AddInstruction(ILInstruction(op, a, b, c, d)); }
	void AddInstruction(ILOperation op, const std::vector<ILParameter>& list) { AddInstruction(ILInstruction(op, list)); }
	void SetInstructionParameter(size_t i, size_t param, const ILParameter& value) { m_instrs[i].params[param] = value; }

	void ReplaceFunction(Function* from, Function* to);
	void ReplaceVariable(Variable* from, Variable* to);
	void CheckForUndefinedReferences(size_t& errors);

	OutputBlock* GetOutputBlock() const { return m_output; }
	void SetOutputBlock(OutputBlock* output);
	uint64_t GetAddress() const { return m_addr; }
	void SetAddress(size_t addr) { m_addr = addr; }
	bool ResolveRelocations();

	bool EndsWithReturn() const;

	void Serialize(OutputBlock* output);
	bool Deserialize(InputBlock* input);
	static ILBlock* GetSerializationMapping(size_t i);
	static void SetSerializationMapping(size_t i, ILBlock* block);

	void Print() const;
};


#endif

