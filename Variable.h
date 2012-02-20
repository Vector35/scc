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

#ifndef __VARIABLE_H__
#define __VARIABLE_H__

#include <map>
#include "Type.h"
#include "Expr.h"
#include "Output.h"


enum VariableClass
{
	VAR_TEMP,
	VAR_LOCAL,
	VAR_PARAM,
	VAR_GLOBAL,
	VAR_FILE_SCOPE,
	VAR_EXTERN
};

class Variable: public RefCountObject
{
	VariableClass m_class;
	size_t m_paramIndex;
	Ref<Type> m_type;
	std::string m_name;
	Location m_location;

	size_t m_dataSectionOffset;
	OutputBlock m_data;

	bool m_written;

	size_t m_tagCount;

	int64_t m_serializationIndex;
	bool m_serializationIndexValid;
	static size_t m_nextSerializationIndex;
	static std::map< size_t, Ref<Variable> > m_serializationMap;

	bool DeserializeInternal(InputBlock* input);

public:
	Variable();
	Variable(VariableClass cls, Type* type, const std::string& name);
	Variable(size_t paramIndex, Type* type, const std::string& name);

	Variable* Duplicate(DuplicateContext& dup);

	VariableClass GetClass() const { return m_class; }
	bool IsParameter() const { return m_class == VAR_PARAM; }
	bool IsTempVariable() const { return m_class == VAR_TEMP; }
	bool IsLocalScope() const { return m_class == VAR_FILE_SCOPE; }
	bool IsGlobal() const { return (m_class == VAR_GLOBAL) || (m_class == VAR_FILE_SCOPE) || (m_class == VAR_EXTERN); }
	bool IsExternal() const { return m_class == VAR_EXTERN; }
	size_t GetParameterIndex() const { return m_paramIndex; }
	Type* GetType() const { return m_type; }
	const std::string& GetName() const { return m_name; }

	const Location& GetLocation() const { return m_location; }
	void SetLocation(const Location& loc) { m_location = loc; }

	size_t GetDataSectionOffset() const { return m_dataSectionOffset; }
	void SetDataSectionOffset(size_t offset) { m_dataSectionOffset = offset; }
	OutputBlock& GetData() { return m_data; }

	void ResetTagCount() { m_tagCount = 0; }
	size_t GetTagCount() const { return m_tagCount; }
	void TagReference() { m_tagCount++; }

	bool IsWritten() const { return m_written; }
	void SetWritten(bool written) { m_written = written; }

	void Serialize(OutputBlock* output);
	static Variable* Deserialize(InputBlock* input);
};


#endif

