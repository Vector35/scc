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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Expr.h"
#include "Struct.h"
#include "ParserState.h"
#include "Variable.h"
#include "Function.h"
#include "Output.h"

using namespace std;


Expr::Expr(ExprClass cls)
{
	m_class = cls;
	m_location.lineNumber = 0;
	m_intValue = 0;
	m_floatValue = 0;
}


Expr::Expr(const Location& loc, ExprClass cls)
{
	m_class = cls;
	m_location = loc;
	m_intValue = 0;
	m_floatValue = 0;
}


Expr* Expr::Duplicate(DuplicateContext& dup)
{
	Expr* expr = new Expr(m_class);
	expr->m_location = m_location;
	expr->m_intValue = m_intValue;
	expr->m_floatValue = m_floatValue;
	expr->m_stringValue = m_stringValue;
	expr->m_variable = (m_variable != NULL) ? m_variable->Duplicate(dup) : NULL;
	expr->m_function = (m_function != NULL) ? m_function->Duplicate(dup) : NULL;
	expr->m_type = (m_type != NULL) ? m_type->Duplicate(dup) : NULL;

	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		expr->m_children.push_back((*i)->Duplicate(dup));

	return expr;
}


Variable* Expr::GetVariable() const
{
	return m_variable;
}


Function* Expr::GetFunction() const
{
	return m_function;
}


int64_t Expr::ComputeIntegerValue(ParserState* state)
{
	int64_t value;
	switch (m_class)
	{
	case EXPR_INT:
		return m_intValue;
	case EXPR_PLUS:
		return m_children[0]->ComputeIntegerValue(state) + m_children[1]->ComputeIntegerValue(state);
	case EXPR_MINUS:
		return m_children[0]->ComputeIntegerValue(state) - m_children[1]->ComputeIntegerValue(state);
	case EXPR_MULT:
		return m_children[0]->ComputeIntegerValue(state) * m_children[1]->ComputeIntegerValue(state);
	case EXPR_DIV:
		value = m_children[1]->ComputeIntegerValue(state);
		if (value == 0)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: division by zero\n", m_location.fileName.c_str(), m_location.lineNumber);
			return 0;
		}
		return m_children[0]->ComputeIntegerValue(state) / value;
	case EXPR_MOD:
		value = m_children[1]->ComputeIntegerValue(state);
		if (value == 0)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: division by zero\n", m_location.fileName.c_str(), m_location.lineNumber);
			return 0;
		}
		return m_children[0]->ComputeIntegerValue(state) % value;
	case EXPR_AND:
		return m_children[0]->ComputeIntegerValue(state) & m_children[1]->ComputeIntegerValue(state);
	case EXPR_OR:
		return m_children[0]->ComputeIntegerValue(state) | m_children[1]->ComputeIntegerValue(state);
	case EXPR_XOR:
		return m_children[0]->ComputeIntegerValue(state) ^ m_children[1]->ComputeIntegerValue(state);
	case EXPR_SHIFT_LEFT:
		return m_children[0]->ComputeIntegerValue(state) << m_children[1]->ComputeIntegerValue(state);
	case EXPR_SHIFT_RIGHT:
		return m_children[0]->ComputeIntegerValue(state) >> m_children[1]->ComputeIntegerValue(state);
	case EXPR_NEG:
		return -m_children[0]->ComputeIntegerValue(state);
	case EXPR_NOT:
		return ~m_children[0]->ComputeIntegerValue(state);
	case EXPR_MIN:
		value = m_children[0]->ComputeIntegerValue(state);
		for (size_t i = 0; i < m_children.size(); i++)
		{
			int64_t cur = m_children[i]->ComputeIntegerValue(state);
			if (cur < value)
				value = cur;
		}
		return value;
	case EXPR_MAX:
		value = m_children[0]->ComputeIntegerValue(state);
		for (size_t i = 0; i < m_children.size(); i++)
		{
			int64_t cur = m_children[i]->ComputeIntegerValue(state);
			if (cur > value)
				value = cur;
		}
		return value;
	case EXPR_ABS:
		value = m_children[0]->ComputeIntegerValue(state);
		return (value < 0) ? (-value) : value;
	default:
		state->Error();
		fprintf(stderr, "%s:%d: error: expected constant integer expression\n", m_location.fileName.c_str(),
			m_location.lineNumber);
		return 0;
	}
}


Type* Expr::ComputeType(ParserState* state, Function* func)
{
	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		(*i)->ComputeType(state, func);

	switch (m_class)
	{
	case EXPR_SEQUENCE:
		if (m_children.size() == 0)
		{
			m_type = Type::VoidType();
			break;
		}
		m_type = m_children[m_children.size() - 1]->GetType();
		break;
	case EXPR_INITIALIZER:
		m_type = Type::VoidType();
		break;
	case EXPR_INT:
		if (!m_type)
			m_type = Type::IntType(0, true);
		break;
	case EXPR_FLOAT:
		if (!m_type)
			m_type = Type::FloatType(0);
		break;
	case EXPR_STRING:
		m_type = Type::ArrayType(Type::IntType(1, true), m_stringValue.size() + 1); // const char[]
		m_type->SetConst(true);
		break;
	case EXPR_TRUE:
	case EXPR_FALSE:
		m_type = Type::BoolType();
		break;
	case EXPR_VARIABLE:
		m_type = m_variable->GetType();
		break;
	case EXPR_FUNCTION:
		m_type = m_function->GetType();
		break;
	case EXPR_DOT:
		if (m_children[0]->GetType()->GetClass() != TYPE_STRUCT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: not a structure or union for operator '.'\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
			break;
		}
		if (!m_children[0]->GetType()->GetStruct()->HasMember(m_stringValue))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type '%s' has no member named '%s'\n", m_location.fileName.c_str(),
				m_location.lineNumber, m_children[0]->GetType()->GetStruct()->GetName().c_str(),
				m_stringValue.c_str());
			m_type = Type::VoidType();
			break;
		}
		m_type = m_children[0]->GetType()->GetStruct()->GetMember(state, m_stringValue).type;
		break;
	case EXPR_ARROW:
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) ||
			(m_children[0]->GetType()->GetChildType()->GetClass() != TYPE_STRUCT))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: not a pointer to a structure or union for operator '->'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
			m_type = Type::VoidType();
			break;
		}
		if (!m_children[0]->GetType()->GetChildType()->GetStruct()->HasMember(m_stringValue))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type '%s' has no member named '%s'\n", m_location.fileName.c_str(),
				m_location.lineNumber, m_children[0]->GetType()->GetChildType()->GetStruct()->GetName().c_str(),
				m_stringValue.c_str());
			m_type = Type::VoidType();
			break;
		}
		m_type = m_children[0]->GetType()->GetChildType()->GetStruct()->GetMember(state, m_stringValue).type;
		break;
	case EXPR_ADDRESS_OF:
		m_type = Type::PointerType(m_children[0]->GetType(), 1);
		break;
	case EXPR_DEREF:
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) && (m_children[0]->GetType()->GetClass() != TYPE_ARRAY))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: not a pointer for dereferencing\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
			break;
		}
		m_type = m_children[0]->GetType()->GetChildType();
		break;
	case EXPR_PRE_INCREMENT:
	case EXPR_PRE_DECREMENT:
	case EXPR_POST_INCREMENT:
	case EXPR_POST_DECREMENT:
		m_type = m_children[0]->GetType();
		if ((m_type->GetClass() != TYPE_INT) && (m_type->GetClass() != TYPE_POINTER))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_ARRAY_INDEX:
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
			(m_children[0]->GetType()->GetClass() != TYPE_ARRAY))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: not an array or pointer for operator '[]'\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
			break;
		}
		if (m_children[1]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer for array index\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		m_type = m_children[0]->GetType()->GetChildType();
		break;
	case EXPR_PLUS:
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
					m_type = m_children[0]->GetType();
				else
					m_type = m_children[1]->GetType();
			}
			else if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				m_type = m_children[1]->GetType();
			}
			else if (m_children[1]->GetType()->GetClass() == TYPE_POINTER)
			{
				m_type = m_children[1]->GetType();
				m_children[0] = m_children[0]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
				break;
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
				m_type = m_children[0]->GetType();
			else if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
					m_type = m_children[0]->GetType();
				else
					m_type = m_children[1]->GetType();
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				m_type = m_children[0]->GetType();
				m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
				break;
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		m_children[0] = m_children[0]->ConvertToType(state, m_type);
		m_children[1] = m_children[1]->ConvertToType(state, m_type);
		break;
	case EXPR_MINUS:
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
					m_type = m_children[0]->GetType();
				else
					m_type = m_children[1]->GetType();
			}
			else if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				m_type = m_children[1]->GetType();
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
				m_type = m_children[0]->GetType();
			else if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
					m_type = m_children[0]->GetType();
				else
					m_type = m_children[1]->GetType();
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				m_type = m_children[0]->GetType();
				m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
				break;
			}
			else if ((m_children[1]->GetType()->GetClass() == TYPE_POINTER) &&
				((*m_children[0]->GetType()) == (*m_children[1]->GetType())))
			{
				m_type = Type::IntType(GetTargetPointerSize(), true);
				break;
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		m_children[0] = m_children[0]->ConvertToType(state, m_type);
		m_children[1] = m_children[1]->ConvertToType(state, m_type);
		break;
	case EXPR_MULT:
	case EXPR_DIV:
	case EXPR_MOD:
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
					m_type = m_children[0]->GetType();
				else
					m_type = m_children[1]->GetType();
			}
			else if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				m_type = m_children[1]->GetType();
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_INT)
				m_type = m_children[0]->GetType();
			else if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
					m_type = m_children[0]->GetType();
				else
					m_type = m_children[1]->GetType();
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				m_type = Type::VoidType();
			}
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		m_children[0] = m_children[0]->ConvertToType(state, m_type);
		m_children[1] = m_children[1]->ConvertToType(state, m_type);
		break;
	case EXPR_AND:
	case EXPR_OR:
	case EXPR_XOR:
		if ((m_children[0]->GetType()->GetClass() == TYPE_INT) && (m_children[1]->GetType()->GetClass() == TYPE_INT))
		{
			if (m_children[0]->GetType()->GetWidth() > m_children[1]->GetType()->GetWidth())
				m_type = m_children[0]->GetType();
			else
				m_type = m_children[1]->GetType();
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in bitwise operation\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		m_children[0] = m_children[0]->ConvertToType(state, m_type);
		m_children[1] = m_children[1]->ConvertToType(state, m_type);
		break;
	case EXPR_SHIFT_LEFT:
	case EXPR_SHIFT_RIGHT:
		if ((m_children[0]->GetType()->GetClass() == TYPE_INT) && (m_children[1]->GetType()->GetClass() == TYPE_INT))
			m_type = m_children[0]->GetType();
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in shift operation\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		if (m_children[0]->GetType()->GetWidth() == 0)
		{
			// No width for shift, use natural width by default
			m_children[0] = m_children[0]->ConvertToType(state, Type::IntType(GetTargetPointerSize(),
				m_children[0]->GetType()->IsSigned()));
			m_type = m_children[0]->GetType();
		}
		m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(1, false));
		break;
	case EXPR_NEG:
		m_type = m_children[0]->GetType();
		if ((m_type->GetClass() != TYPE_INT) && (m_type->GetClass() != TYPE_FLOAT))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer or float for operator '-'\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_NOT:
		m_type = m_children[0]->GetType();
		if (m_type->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer for operator '~'\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_LOGICAL_AND:
	case EXPR_LOGICAL_OR:
		m_type = Type::BoolType();
		if ((m_children[0]->GetType()->GetClass() != TYPE_BOOL) || (m_children[1]->GetType()->GetClass() != TYPE_BOOL))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected boolean expression\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_LOGICAL_NOT:
		m_type = Type::BoolType();
		if (m_children[0]->GetType()->GetClass() != TYPE_BOOL)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected boolean expression for operator '!'\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_LESS_THAN:
	case EXPR_LESS_EQUAL:
	case EXPR_GREATER_EQUAL:
	case EXPR_GREATER_THAN:
		m_type = Type::BoolType();
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			Type* type = PromotedType(state, m_children[0], m_children[1]);
			if (type->GetClass() != TYPE_VOID)
			{
				m_children[0] = m_children[0]->ConvertToType(state, type);
				m_children[1] = m_children[1]->ConvertToType(state, type);
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			Type* type = PromotedType(state, m_children[0], m_children[1]);
			if (type->GetClass() != TYPE_VOID)
			{
				m_children[0] = m_children[0]->ConvertToType(state, type);
				m_children[1] = m_children[1]->ConvertToType(state, type);
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			if ((*m_children[0]->GetType()) != (*m_children[1]->GetType()))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in comparison\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in comparison\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		break;
	case EXPR_EQUAL:
	case EXPR_NOT_EQUAL:
		m_type = Type::BoolType();
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			if ((m_children[1]->GetType()->GetClass() == TYPE_POINTER) || (m_children[1]->GetType()->GetClass() == TYPE_FUNCTION))
			{
				if ((m_children[0]->GetClass() != EXPR_INT) ||
					(m_children[0]->GetIntValue() != 0))
				{
					state->Error();
					fprintf(stderr, "%s:%d: error: type mismatch in comparison\n", m_location.fileName.c_str(),
						m_location.lineNumber);
				}
			}
			else
			{
				Type* type = PromotedType(state, m_children[0], m_children[1]);
				if (type->GetClass() != TYPE_VOID)
				{
					m_children[0] = m_children[0]->ConvertToType(state, type);
					m_children[1] = m_children[1]->ConvertToType(state, type);
				}
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			Type* type = PromotedType(state, m_children[0], m_children[1]);
			if (type->GetClass() != TYPE_VOID)
			{
				m_children[0] = m_children[0]->ConvertToType(state, type);
				m_children[1] = m_children[1]->ConvertToType(state, type);
			}
		}
		else if ((m_children[0]->GetType()->GetClass() == TYPE_POINTER) || (m_children[0]->GetType()->GetClass() == TYPE_FUNCTION))
		{
			if ((*m_children[0]->GetType()) != (*m_children[1]->GetType()))
			{
				if ((m_children[1]->GetClass() != EXPR_INT) ||
					(m_children[1]->GetIntValue() != 0))
				{
					state->Error();
					fprintf(stderr, "%s:%d: error: type mismatch in comparison\n", m_location.fileName.c_str(),
						m_location.lineNumber);
				}
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_BOOL)
		{
			if (m_children[1]->GetType()->GetClass() != TYPE_BOOL)
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in comparison\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in comparison\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_ASSIGN:
		m_type = m_children[0]->GetType();
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				fprintf(stderr, "%s:%d: warning: implicit conversion to integer may lose precision\n",
					m_location.fileName.c_str(), m_location.lineNumber);
			}
			else if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				if (m_children[1]->GetType()->GetWidth() > m_children[0]->GetType()->GetWidth())
				{
					fprintf(stderr, "%s:%d: warning: implicit conversion may truncate bits\n",
						m_location.fileName.c_str(), m_location.lineNumber);
				}
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			if ((m_children[1]->GetType()->GetClass() != TYPE_INT) &&
				(m_children[1]->GetType()->GetClass() != TYPE_FLOAT))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if ((m_children[0]->GetType()->GetClass() == TYPE_POINTER) || (m_children[0]->GetType()->GetClass() == TYPE_FUNCTION))
		{
			// Special case NULL, which is simply integer constant zero
			if ((!m_children[1]->GetType()->CanAssignTo(*m_children[0]->GetType())) &&
				((m_children[1]->GetClass() != EXPR_INT) || (m_children[1]->GetIntValue() != 0)))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (!m_children[1]->GetType()->CanAssignTo(*m_children[0]->GetType()))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		m_children[1] = m_children[1]->ConvertToType(state, m_type);
		break;
	case EXPR_INIT_ASSIGN:
		m_type = m_children[0]->GetType();
		if (m_children[0]->GetType()->GetClass() == TYPE_INT)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_FLOAT)
			{
				fprintf(stderr, "%s:%d: warning: implicit conversion to integer may lose precision\n",
					m_location.fileName.c_str(), m_location.lineNumber);
			}
			else if (m_children[1]->GetType()->GetClass() == TYPE_INT)
			{
				if (m_children[1]->GetType()->GetWidth() > m_children[0]->GetType()->GetWidth())
				{
					fprintf(stderr, "%s:%d: warning: implicit conversion may truncate bits\n",
						m_location.fileName.c_str(), m_location.lineNumber);
				}
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
		{
			if ((m_children[1]->GetType()->GetClass() != TYPE_INT) &&
				(m_children[1]->GetType()->GetClass() != TYPE_FLOAT))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (m_children[1]->GetClass() == EXPR_INITIALIZER)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: structure/array initializers not yet implemented\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		else if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			// Special case NULL, which is simply integer constant zero
			if ((!m_children[1]->GetType()->CanAssignTo(*m_children[0]->GetType())) &&
				((m_children[1]->GetClass() != EXPR_INT) || (m_children[1]->GetIntValue() != 0)))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (!m_children[1]->GetType()->CanAssignTo(*m_children[0]->GetType()))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in assignment\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		m_children[1] = m_children[1]->ConvertToType(state, m_type);
		break;
	case EXPR_IF:
	case EXPR_IF_ELSE:
	case EXPR_WHILE:
	case EXPR_DO_WHILE:
		m_type = Type::VoidType();
		if (m_children[0]->GetType()->GetClass() != TYPE_BOOL)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected boolean condition\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_FOR:
		m_type = Type::VoidType();
		if (m_children[1]->GetType()->GetClass() != TYPE_BOOL)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected boolean condition\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_IF_ELSE_VALUE:
		if (m_children[0]->GetType()->GetClass() != TYPE_BOOL)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected boolean condition\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		m_type = PromotedType(state, m_children[1], m_children[2]);
		if (m_type->GetClass() != TYPE_VOID)
		{
			m_children[1] = m_children[1]->ConvertToType(state, m_type);
			m_children[2] = m_children[2]->ConvertToType(state, m_type);
		}
		break;
	case EXPR_CALL:
		if (m_children[0]->GetType()->GetClass() != TYPE_FUNCTION)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: not a function in call\n", m_location.fileName.c_str(), m_location.lineNumber);
			m_type = Type::VoidType();
			break;
		}
		if ((m_children.size() - 1) < m_children[0]->GetType()->GetParams().size())
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: too few parameters in call\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
			break;
		}
		m_type = m_children[0]->GetType()->GetChildType();
		for (size_t i = 0; i < m_children[0]->GetType()->GetParams().size(); i++)
		{
			if ((!m_children[i + 1]->GetType()->CanAssignTo(*m_children[0]->GetType()->GetParams()[i])) &&
				(((m_children[0]->GetType()->GetParams()[i]->GetClass() != TYPE_POINTER) &&
				(m_children[0]->GetType()->GetParams()[i]->GetClass() != TYPE_FUNCTION)) ||
				(m_children[i + 1]->GetClass() != EXPR_INT) || (m_children[i + 1]->GetIntValue() != 0)))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in parameter %d\n", m_location.fileName.c_str(),
					m_location.lineNumber, (int)i + 1);
			}
			m_children[i + 1] = m_children[i + 1]->ConvertToType(state, m_children[0]->GetType()->GetParams()[i]);
		}
		if (m_children[0]->GetType()->HasVariableArguments())
		{
			// Promote variable argument integer parameters to their native size, and promote floating point
			// parameters to double
			for (size_t i = m_children[0]->GetType()->GetParams().size(); i < (m_children.size() - 1); i++)
			{
				if ((m_children[i + 1]->GetType()->GetClass() == TYPE_INT) &&
					(m_children[i + 1]->GetType()->GetWidth() < GetTargetPointerSize()))
				{
					m_children[i + 1] = m_children[i + 1]->ConvertToType(state,
						Type::IntType(GetTargetPointerSize(), true));
				}
				else if ((m_children[i + 1]->GetType()->GetClass() == TYPE_FLOAT) &&
					(m_children[i + 1]->GetType()->GetWidth() < 8))
				{
					m_children[i + 1] = m_children[i + 1]->ConvertToType(state, Type::FloatType(8));
				}
			}
		}
		if (((m_children.size() - 1) > m_children[0]->GetType()->GetParams().size()) &&
			(!m_children[0]->GetType()->HasVariableArguments()))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: too many parameters in call\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_MIN:
	case EXPR_MAX:
		m_type = m_children[0]->GetType();
		if ((m_type->GetClass() != TYPE_INT) || (m_type->GetClass() != TYPE_FLOAT))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in function 'min' or 'max'\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		else
		{
			for (size_t i = 1; i < m_children.size(); i++)
			{
				if (m_type->GetClass() == TYPE_INT)
				{
					if (m_children[i]->GetType()->GetClass() == TYPE_INT)
					{
						if (m_children[i]->GetType()->GetWidth() > m_type->GetWidth())
							m_type = m_children[i]->GetType();
					}
					else if (m_children[i]->GetType()->GetClass() == TYPE_FLOAT)
					{
						m_type = m_children[i]->GetType();
					}
					else
					{
						state->Error();
						fprintf(stderr, "%s:%d: error: type mismatch in function 'min' or 'max'\n",
							m_location.fileName.c_str(), m_location.lineNumber);
					}
				}
				else if (m_type->GetClass() == TYPE_FLOAT)
				{
					if (m_children[i]->GetType()->GetClass() == TYPE_FLOAT)
					{
						if (m_children[i]->GetType()->GetWidth() > m_type->GetWidth())
							m_type = m_children[i]->GetType();
					}
					else if (m_children[i]->GetType()->GetClass() != TYPE_INT)
					{
						state->Error();
						fprintf(stderr, "%s:%d: error: type mismatch in function 'min' or 'max'\n",
							m_location.fileName.c_str(), m_location.lineNumber);
					}
				}
			}

			for (size_t i = 0; i < m_children.size(); i++)
				m_children[i] = m_children[i]->ConvertToType(state, m_type);
		}
		break;
	case EXPR_ABS:
		m_type = m_children[0]->GetType();
		if ((m_type->GetClass() != TYPE_INT) && (m_type->GetClass() != TYPE_FLOAT))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in function 'abs'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		break;
	case EXPR_ALLOCA:
		m_type = Type::PointerType(Type::VoidType(), 1);
		if (m_children[0]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer in function 'alloca'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		m_children[0] = m_children[0]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		break;
	case EXPR_MEMCPY:
		m_type = Type::PointerType(Type::VoidType(), 1);
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
			(m_children[0]->GetType()->GetClass() != TYPE_ARRAY))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected pointer for destination in 'memcpy'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		if ((m_children[1]->GetType()->GetClass() != TYPE_POINTER) &&
			(m_children[1]->GetType()->GetClass() != TYPE_ARRAY))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected pointer for source in 'memcpy'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		if (m_children[2]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer for length in 'memcpy'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		m_children[2] = m_children[2]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		break;
	case EXPR_MEMSET:
		m_type = Type::PointerType(Type::VoidType(), 1);
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
			(m_children[0]->GetType()->GetClass() != TYPE_ARRAY))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected pointer for destination in 'memset'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		if (m_children[1]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer for value in 'memset'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		if (m_children[2]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer for length in 'memset'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(1, false));
		m_children[2] = m_children[2]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		break;
	case EXPR_STRLEN:
		m_type = Type::IntType(GetTargetPointerSize(), false);
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
			(m_children[0]->GetType()->GetClass() != TYPE_ARRAY))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected pointer to string in 'strlen'\n",
				m_location.fileName.c_str(), m_location.lineNumber);
		}
		break;
	case EXPR_CAST:
		if (m_type->GetClass() == TYPE_INT)
		{
			if ((m_children[0]->GetType()->GetClass() != TYPE_INT) &&
				(m_children[0]->GetType()->GetClass() != TYPE_FLOAT) &&
				(m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
				(m_children[0]->GetType()->GetClass() != TYPE_FUNCTION))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: invalid cast\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
		}
		else if (m_type->GetClass() == TYPE_FLOAT)
		{
			if ((m_children[0]->GetType()->GetClass() != TYPE_INT) &&
				(m_children[0]->GetType()->GetClass() != TYPE_FLOAT))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: invalid cast\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
		}
		else if ((m_type->GetClass() == TYPE_POINTER) || (m_type->GetClass() == TYPE_FUNCTION))
		{
			if ((m_children[0]->GetType()->GetClass() != TYPE_INT) &&
				(m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
				(m_children[0]->GetType()->GetClass() != TYPE_ARRAY) &&
				(m_children[0]->GetType()->GetClass() != TYPE_FUNCTION))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: invalid cast\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: invalid cast\n", m_location.fileName.c_str(), m_location.lineNumber);
		}
		m_children[0] = m_children[0]->ConvertToType(state, m_type);
		break;
	case EXPR_RETURN:
		if (func->GetReturnValue()->GetClass() == TYPE_INT)
		{
			if (m_children[0]->GetType()->GetClass() == TYPE_FLOAT)
			{
				fprintf(stderr, "%s:%d: warning: implicit conversion to integer may lose precision\n",
					m_location.fileName.c_str(), m_location.lineNumber);
			}
			else if (m_children[0]->GetType()->GetClass() == TYPE_INT)
			{
				if (m_children[0]->GetType()->GetWidth() > m_children[0]->GetType()->GetWidth())
				{
					fprintf(stderr, "%s:%d: warning: implicit conversion may truncate bits\n",
						m_location.fileName.c_str(), m_location.lineNumber);
				}
			}
			else
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in return\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (func->GetReturnValue()->GetClass() == TYPE_FLOAT)
		{
			if ((m_children[0]->GetType()->GetClass() != TYPE_INT) &&
				(m_children[0]->GetType()->GetClass() != TYPE_FLOAT))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in return\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (func->GetReturnValue()->GetClass() == TYPE_POINTER)
		{
			// Special case NULL, which is simply integer constant zero
			if ((!m_children[0]->GetType()->CanAssignTo(*func->GetReturnValue())) &&
				((m_children[0]->GetClass() != EXPR_INT) || (m_children[0]->GetIntValue() != 0)))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: type mismatch in return\n", m_location.fileName.c_str(),
					m_location.lineNumber);
			}
		}
		else if (!m_children[0]->GetType()->CanAssignTo(*func->GetReturnValue()))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in return\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}

		m_type = Type::VoidType();
		if (func)
			m_children[0] = m_children[0]->ConvertToType(state, func->GetReturnValue());
		break;
	case EXPR_RETURN_VOID:
	case EXPR_LABEL:
	case EXPR_GOTO_LABEL:
		m_type = Type::VoidType();
		break;
	case EXPR_COMPUTED_GOTO:
		m_type = Type::VoidType();
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) &&
			(m_children[0]->GetType()->GetClass() != TYPE_FUNCTION))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected pointer in computed goto\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_BREAK:
	case EXPR_CONTINUE:
		m_type = Type::VoidType();
		break;
	case EXPR_SWITCH:
		m_type = Type::VoidType();
		if (m_children[0]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer in switch statement\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		break;
	case EXPR_CASE:
	case EXPR_DEFAULT:
		m_type = Type::VoidType();
		break;
	case EXPR_UNDEFINED:
		m_type = Type::IntType(GetTargetPointerSize(), true);
		break;
	case EXPR_SYSCALL:
		if (m_children[0]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected syscall number\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		for (size_t i = 1; i < m_children.size(); i++)
		{
			if ((m_children[i]->GetType()->GetClass() == TYPE_VOID) ||
				(m_children[i]->GetType()->GetClass() == TYPE_FLOAT) ||
				(m_children[i]->GetType()->GetClass() == TYPE_STRUCT))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: syscall parameter %d invalid\n", m_location.fileName.c_str(),
					m_location.lineNumber, (int)i);
			}
			if (m_children[i]->GetType()->GetWidth() == 0)
				m_children[i] = m_children[i]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		}
		m_children[0] = m_children[0]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		m_type = Type::IntType(GetTargetPointerSize(), false);
		break;
	case EXPR_SYSCALL2:
		if (m_children[1]->GetType()->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected syscall number\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		for (size_t i = 2; i < m_children.size(); i++)
		{
			if ((m_children[i]->GetType()->GetClass() == TYPE_VOID) ||
				(m_children[i]->GetType()->GetClass() == TYPE_FLOAT) ||
				(m_children[i]->GetType()->GetClass() == TYPE_STRUCT))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: syscall parameter %d invalid\n", m_location.fileName.c_str(),
					m_location.lineNumber, (int)i - 1);
			}
			if (m_children[i]->GetType()->GetWidth() == 0)
				m_children[i] = m_children[i]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		}
		m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		m_type = Type::IntType(GetTargetPointerSize(), false);
		break;
	case EXPR_RDTSC:
		m_type = Type::IntType(8, false);
		break;
	case EXPR_RDTSC_LOW:
		m_type = Type::IntType(4, false);
		break;
	case EXPR_RDTSC_HIGH:
		m_type = Type::IntType(4, false);
		break;
	case EXPR_NEXT_ARG:
	case EXPR_PREV_ARG:
		m_type = m_children[0]->GetType();
		if ((m_children[0]->GetType()->GetClass() != TYPE_POINTER) ||
			(m_children[1]->GetType()->GetClass() != TYPE_INT))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch in arithmetic\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		m_children[1] = m_children[1]->ConvertToType(state, Type::IntType(GetTargetPointerSize(), false));
		break;
	case EXPR_BYTESWAP:
		m_type = m_children[0]->GetType();
		if (m_type->GetClass() != TYPE_INT)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected integer in __byteswap\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		else if (m_type->GetWidth() == 0)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: unknown size in __byteswap\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			m_type = Type::VoidType();
		}
		break;
	default:
		state->Error();
		fprintf(stderr, "%s:%d: error: invalid expression in type computation\n", m_location.fileName.c_str(),
			m_location.lineNumber);
		m_type = Type::VoidType();
		break;
	}

	return m_type;
}


Expr* Expr::Simplify(ParserState* state)
{
	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		*i = (*i)->Simplify(state);

	switch (m_class)
	{
	case EXPR_SEQUENCE:
		// Eliminate empty sequences and inline child sequences
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (m_children[i]->GetClass() == EXPR_SEQUENCE)
			{
				Ref<Expr> child = m_children[i];
				m_children.erase(m_children.begin() + i);
				m_children.insert(m_children.begin() + i, child->GetChildren().begin(), child->GetChildren().end());
				i += child->GetChildren().size() - 1;
			}
		}
		// Simplify to the expression itself if there is only one
		if (m_children.size() == 1)
			return m_children[0];
		return this;
	case EXPR_DEREF:
		if (m_children[0]->GetClass() == EXPR_ADDRESS_OF)
			return m_children[0]->m_children[0];
		return this;
	case EXPR_ARRAY_INDEX:
		if ((m_children[1]->GetClass() == EXPR_INT) && (m_children[1]->GetIntValue() == 0) &&
			m_children[0]->GetType() && (m_children[0]->GetType()->GetClass() == TYPE_POINTER))
			return Expr::UnaryExpr(m_location, EXPR_DEREF, m_children[0]);
		return this;
	case EXPR_PLUS:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() + m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, (double)m_children[0]->GetIntValue() + m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() + (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() + m_children[1]->GetFloatValue());
		return this;
	case EXPR_MINUS:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() - m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, (double)m_children[0]->GetIntValue() - m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() - (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() - m_children[1]->GetFloatValue());
		return this;
	case EXPR_MULT:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() * m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, (double)m_children[0]->GetIntValue() * m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() * (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() * m_children[1]->GetFloatValue());
		return this;
	case EXPR_DIV:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
		{
			if (m_children[1]->GetIntValue() == 0)
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: division by zero\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				return Expr::IntExpr(m_location, 0);
			}
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() / m_children[1]->GetIntValue());
		}
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, (double)m_children[0]->GetIntValue() / m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() / (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, m_children[0]->GetFloatValue() / m_children[1]->GetFloatValue());
		return this;
	case EXPR_MOD:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
		{
			if (m_children[1]->GetIntValue() == 0)
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: division by zero\n", m_location.fileName.c_str(),
					m_location.lineNumber);
				return Expr::IntExpr(m_location, 0);
			}
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() % m_children[1]->GetIntValue());
		}
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, fmod(m_children[0]->GetIntValue(), m_children[1]->GetFloatValue()));
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::FloatExpr(m_location, fmod(m_children[0]->GetFloatValue(), m_children[1]->GetIntValue()));
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::FloatExpr(m_location, fmod(m_children[0]->GetFloatValue(), m_children[1]->GetFloatValue()));
		return this;
	case EXPR_AND:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() & m_children[1]->GetIntValue());
		return this;
	case EXPR_OR:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() | m_children[1]->GetIntValue());
		return this;
	case EXPR_XOR:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() ^ m_children[1]->GetIntValue());
		return this;
	case EXPR_SHIFT_LEFT:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() << m_children[1]->GetIntValue());
		return this;
	case EXPR_SHIFT_RIGHT:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::IntExpr(m_location, m_children[0]->GetIntValue() >> m_children[1]->GetIntValue());
		return this;
	case EXPR_NEG:
		if (m_children[0]->GetClass() == EXPR_INT)
			return Expr::IntExpr(m_location, -m_children[0]->GetIntValue());
		if (m_children[0]->GetClass() == EXPR_FLOAT)
			return Expr::FloatExpr(m_location, -m_children[0]->GetFloatValue());
		return this;
	case EXPR_NOT:
		if (m_children[0]->GetClass() == EXPR_INT)
			return Expr::IntExpr(m_location, ~m_children[0]->GetIntValue());
		return this;
	case EXPR_LOGICAL_AND:
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return m_children[0];
		if (m_children[0]->GetClass() == EXPR_TRUE)
			return m_children[1];
		return this;
	case EXPR_LOGICAL_OR:
		if (m_children[0]->GetClass() == EXPR_TRUE)
			return m_children[0];
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return m_children[1];
		return this;
	case EXPR_LOGICAL_NOT:
		if (m_children[0]->GetClass() == EXPR_TRUE)
			return Expr::BoolExpr(m_location, false);
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return Expr::BoolExpr(m_location, true);
		return this;
	case EXPR_LESS_THAN:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetIntValue() < m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, (double)m_children[0]->GetIntValue() < m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() < (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() < m_children[1]->GetFloatValue());
		return this;
	case EXPR_LESS_EQUAL:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetIntValue() <= m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, (double)m_children[0]->GetIntValue() <= m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() <= (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() <= m_children[1]->GetFloatValue());
		return this;
	case EXPR_EQUAL:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetIntValue() == m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, (double)m_children[0]->GetIntValue() == m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() == (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() == m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_TRUE) && m_children[1]->GetType() && (m_children[1]->GetType()->GetClass() == TYPE_BOOL))
			return m_children[1];
		if ((m_children[0]->GetClass() == EXPR_FALSE) && m_children[1]->GetType() && (m_children[1]->GetType()->GetClass() == TYPE_BOOL))
			return Expr::UnaryExpr(m_location, EXPR_LOGICAL_NOT, m_children[1]);
		if ((m_children[1]->GetClass() == EXPR_TRUE) && m_children[0]->GetType() && (m_children[0]->GetType()->GetClass() == TYPE_BOOL))
			return m_children[0];
		if ((m_children[1]->GetClass() == EXPR_FALSE) && m_children[0]->GetType() && (m_children[0]->GetType()->GetClass() == TYPE_BOOL))
			return Expr::UnaryExpr(m_location, EXPR_LOGICAL_NOT, m_children[0]);
		return this;
	case EXPR_NOT_EQUAL:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetIntValue() != m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, (double)m_children[0]->GetIntValue() != m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() != (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() != m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_TRUE) && m_children[1]->GetType() && (m_children[1]->GetType()->GetClass() == TYPE_BOOL))
			return Expr::UnaryExpr(m_location, EXPR_LOGICAL_NOT, m_children[1]);
		if ((m_children[0]->GetClass() == EXPR_FALSE) && m_children[1]->GetType() && (m_children[1]->GetType()->GetClass() == TYPE_BOOL))
			return m_children[1];
		if ((m_children[1]->GetClass() == EXPR_TRUE) && m_children[0]->GetType() && (m_children[0]->GetType()->GetClass() == TYPE_BOOL))
			return Expr::UnaryExpr(m_location, EXPR_LOGICAL_NOT, m_children[0]);
		if ((m_children[1]->GetClass() == EXPR_FALSE) && m_children[0]->GetType() && (m_children[0]->GetType()->GetClass() == TYPE_BOOL))
			return m_children[0];
		return this;
	case EXPR_GREATER_EQUAL:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetIntValue() >= m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, (double)m_children[0]->GetIntValue() >= m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() >= (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() >= m_children[1]->GetFloatValue());
		return this;
	case EXPR_GREATER_THAN:
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetIntValue() > m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_INT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, (double)m_children[0]->GetIntValue() > m_children[1]->GetFloatValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_INT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() > (double)m_children[1]->GetIntValue());
		if ((m_children[0]->GetClass() == EXPR_FLOAT) && (m_children[1]->GetClass() == EXPR_FLOAT))
			return Expr::BoolExpr(m_location, m_children[0]->GetFloatValue() > m_children[1]->GetFloatValue());
		return this;
	case EXPR_IF:
		if (m_children[0]->GetClass() == EXPR_TRUE)
			return m_children[1];
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return new Expr(m_location, EXPR_SEQUENCE);
		if ((m_children[1]->GetClass() == EXPR_SEQUENCE) && (m_children[1]->GetChildren().size() == 0))
			return m_children[1];
		return this;
	case EXPR_IF_ELSE:
		if (m_children[0]->GetClass() == EXPR_TRUE)
			return m_children[1];
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return m_children[2];
		if ((m_children[1]->GetClass() == EXPR_SEQUENCE) && (m_children[1]->GetChildren().size() == 0))
		{
			return Expr::IfExpr(m_location, Expr::UnaryExpr(m_location, EXPR_LOGICAL_NOT, m_children[0]),
				m_children[2])->Simplify(state);
		}
		if ((m_children[2]->GetClass() == EXPR_SEQUENCE) && (m_children[2]->GetChildren().size() == 0))
			return Expr::IfExpr(m_location, m_children[0], m_children[1])->Simplify(state);
		return this;
	case EXPR_IF_ELSE_VALUE:
		if (m_children[0]->GetClass() == EXPR_TRUE)
			return m_children[1];
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return m_children[2];
		return this;
	case EXPR_FOR:
		if (m_children[1]->GetClass() == EXPR_FALSE)
			return m_children[0];
		return this;
	case EXPR_WHILE:
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return new Expr(m_location, EXPR_SEQUENCE);
		return this;
	case EXPR_DO_WHILE:
		if (m_children[0]->GetClass() == EXPR_FALSE)
			return m_children[1];
		return this;
	case EXPR_MIN:
	case EXPR_MAX:
		if (m_children.size() == 1)
			return m_children[0];
		return this;
	case EXPR_ABS:
		if (m_children[0]->GetClass() == EXPR_INT)
			return Expr::IntExpr(m_location, llabs(m_children[0]->GetIntValue()));
		if (m_children[0]->GetClass() == EXPR_FLOAT)
			return Expr::FloatExpr(m_location, fabs(m_children[0]->GetIntValue()));
		return this;
	case EXPR_CAST:
		if (m_children[0]->GetClass() == EXPR_INT)
		{
			if (m_type->GetClass() == TYPE_INT)
			{
				Expr* result = m_children[0];
				result->SetType(m_type);
				return result;
			}
			else if (m_type->GetClass() == TYPE_FLOAT)
			{
				Expr* result = Expr::FloatExpr(m_location, (double)m_children[0]->m_intValue);
				result->SetType(m_type);
				return result;
			}
		}
		else if (m_children[0]->GetClass() == EXPR_FLOAT)
		{
			if (m_type->GetClass() == TYPE_INT)
			{
				Expr* result = Expr::IntExpr(m_location, (int64_t)m_children[0]->m_floatValue);
				result->SetType(m_type);
				return result;
			}
			else if (m_type->GetClass() == TYPE_FLOAT)
			{
				Expr* result = m_children[0];
				result->SetType(m_type);
				return result;
			}
		}
		return this;
	case EXPR_STRLEN:
		if (m_children[0]->GetClass() == EXPR_STRING)
		{
			Expr* result = Expr::IntExpr(m_location, strlen(m_children[0]->GetStringValue().c_str()));
			result->SetType(Type::IntType(GetTargetPointerSize(), false));
			return result;
		}
		return this;
	case EXPR_BYTESWAP:
		if (m_type && (m_type->GetWidth() == 1))
		{
			// Byte swap for a single byte integer is a no-op
			return m_children[0];
		}
		else if ((m_children[0]->GetClass() == EXPR_INT) && m_type && (m_type->GetWidth() == 2))
		{
			uint16_t value = (uint16_t)m_children[0]->GetIntValue();
			value = (value << 8) | (value >> 8);
			Expr* result = Expr::IntExpr(m_location, value);
			result->SetType(m_type);
			return result;
		}
		else if ((m_children[0]->GetClass() == EXPR_INT) && m_type && (m_type->GetWidth() == 4))
		{
			uint32_t value = (uint32_t)m_children[0]->GetIntValue();
			value = (value << 24) | ((value << 8) & 0xff0000) | ((value >> 8) & 0xff00) | (value >> 24);
			Expr* result = Expr::IntExpr(m_location, value);
			result->SetType(m_type);
			return result;
		}
		else if ((m_children[0]->GetClass() == EXPR_INT) && m_type && (m_type->GetWidth() == 8))
		{
			uint64_t value = (uint64_t)m_children[0]->GetIntValue();
			value = (value << 56) | ((value << 40) & 0xff000000000000LL) | ((value << 24) & 0xff0000000000LL) |
				((value << 8) & 0xff00000000LL) | ((value >> 8) & 0xff000000LL) | ((value >> 24) & 0xff0000LL) |
				((value >> 40) & 0xff00LL) | (value >> 56);
			Expr* result = Expr::IntExpr(m_location, value);
			result->SetType(m_type);
			return result;
		}
		return this;
	default:
		return this;
	}
}


Expr* Expr::ConvertToBool(ParserState* state)
{
	Type* type = ComputeType(state, NULL);
	if (type->GetClass() == TYPE_VOID)
		return this;
	if (type->GetClass() == TYPE_BOOL)
		return this;
	return Expr::BinaryExpr(m_location, EXPR_NOT_EQUAL, this, Expr::IntExpr(m_location, 0));
}


Type* Expr::PromotedType(ParserState* state, Expr* a, Expr* b)
{
	if (a->GetType()->GetClass() == TYPE_INT)
	{
		if (b->GetType()->GetClass() == TYPE_INT)
		{
			if (a->GetType()->GetWidth() > b->GetType()->GetWidth())
				return a->GetType();
			else if (b->GetType()->GetWidth() > 0)
				return b->GetType();
			return Type::IntType(GetTargetPointerSize(), b->GetType()->IsSigned());
		}
		else if (b->GetType()->GetClass() == TYPE_FLOAT)
		{
			if (b->GetType()->GetWidth() > 0)
				return b->GetType();
			return Type::FloatType(4);
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			return Type::VoidType();
		}
	}
	else if (a->GetType()->GetClass() == TYPE_FLOAT)
	{
		if (b->GetType()->GetClass() == TYPE_INT)
		{
			if (a->GetType()->GetWidth() > 0)
				return a->GetType();
			return Type::FloatType(4);
		}
		else if (b->GetType()->GetClass() == TYPE_FLOAT)
		{
			if (a->GetType()->GetWidth() > b->GetType()->GetWidth())
				return a->GetType();
			else if (b->GetType()->GetWidth() > 0)
				return b->GetType();
			return Type::FloatType(4);
		}
		else
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: type mismatch\n", m_location.fileName.c_str(),
				m_location.lineNumber);
			return Type::VoidType();
		}
	}
	else if ((*a->GetType()) == (*b->GetType()))
		return a->GetType();

	state->Error();
	fprintf(stderr, "%s:%d: error: type mismatch\n", m_location.fileName.c_str(),
		m_location.lineNumber);
	return Type::VoidType();
}


Expr* Expr::ConvertToType(ParserState* state, Type* typePtr)
{
	Ref<Type> type = typePtr;

	if (m_type->GetClass() == TYPE_INT)
	{
		if (type->GetClass() == TYPE_INT)
		{
			if (m_type->GetWidth() == 0)
				return Expr::CastExpr(m_location, type, this);
			else if (m_type->GetWidth() != type->GetWidth())
				return Expr::CastExpr(m_location, type, this);
		}
		else
		{
			return Expr::CastExpr(m_location, type, this);
		}
	}
	else if (m_type->GetClass() == TYPE_FLOAT)
	{
		if (type->GetClass() == TYPE_FLOAT)
		{
			if (m_type->GetWidth() == 0)
				return Expr::CastExpr(m_location, type, this);
			else if (m_type->GetWidth() != type->GetWidth())
				return Expr::CastExpr(m_location, type, this);
		}
		else
		{
			return Expr::CastExpr(m_location, type, this);
		}
	}
	else if ((*m_type) != (*type))
	{
		return Expr::CastExpr(m_location, type, this);
	}

	return this;
}


void Expr::ReplaceFunction(Function* from, Function* to)
{
	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		(*i)->ReplaceFunction(from, to);

	if (m_function == from)
		m_function = to;
}


void Expr::ReplaceVariable(Variable* from, Variable* to)
{
	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		(*i)->ReplaceVariable(from, to);

	if (m_variable == from)
		m_variable = to;
}


void Expr::CheckForUndefinedReferences(size_t& errors)
{
	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		(*i)->CheckForUndefinedReferences(errors);

	// All prototypes should have been resolved by the linker
	if (m_function && (!m_function->IsFullyDefined()))
	{
		errors++;
		fprintf(stderr, "%s:%d: error: undefined reference to '%s'\n", m_location.fileName.c_str(), m_location.lineNumber,
			m_function->GetName().c_str());
	}

	// All variables marked 'extern' should have been resolved by the linker
	if (m_variable && m_variable->IsExternal())
	{
		errors++;
		fprintf(stderr, "%s:%d: error: undefined reference to '%s'\n", m_location.fileName.c_str(), m_location.lineNumber,
			m_variable->GetName().c_str());
	}
}


void Expr::GenerateConditionalIL(ParserState* state, Function* func, ILBlock* block, ILBlock* trueBlock, ILBlock* falseBlock)
{
	ILParameter a, b;
	ILBlock* tempBlock;

	switch (m_class)
	{
	case EXPR_LOGICAL_AND:
		tempBlock = func->CreateILBlock();
		m_children[0]->GenerateConditionalIL(state, func, block, tempBlock, falseBlock);
		m_children[1]->GenerateConditionalIL(state, func, tempBlock, trueBlock, falseBlock);
		break;
	case EXPR_LOGICAL_OR:
		tempBlock = func->CreateILBlock();
		m_children[0]->GenerateConditionalIL(state, func, block, trueBlock, tempBlock);
		m_children[1]->GenerateConditionalIL(state, func, tempBlock, trueBlock, falseBlock);
		break;
	case EXPR_LOGICAL_NOT:
		m_children[0]->GenerateConditionalIL(state, func, block, falseBlock, trueBlock);
		break;
	case EXPR_LESS_THAN:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, b, a, falseBlock, trueBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, b, a, falseBlock, trueBlock);
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, a, b, trueBlock, falseBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW, a, b, trueBlock, falseBlock);
		}
		break;
	case EXPR_LESS_EQUAL:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, b, a, falseBlock, trueBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW, b, a, falseBlock, trueBlock);
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, a, b, trueBlock, falseBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, a, b, trueBlock, falseBlock);
		}
		break;
	case EXPR_EQUAL:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (a.IsConstant())
			block->AddInstruction(ILOP_IF_EQUAL, b, a, trueBlock, falseBlock);
		else
			block->AddInstruction(ILOP_IF_EQUAL, a, b, trueBlock, falseBlock);
		break;
	case EXPR_NOT_EQUAL:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (a.IsConstant())
			block->AddInstruction(ILOP_IF_EQUAL, b, a, falseBlock, trueBlock);
		else
			block->AddInstruction(ILOP_IF_EQUAL, a, b, falseBlock, trueBlock);
		break;
	case EXPR_GREATER_THAN:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, b, a, trueBlock, falseBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW, b, a, trueBlock, falseBlock);
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, a, b, falseBlock, trueBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, a, b, falseBlock, trueBlock);
		}
		break;
	case EXPR_GREATER_EQUAL:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, b, a, trueBlock, falseBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, b, a, trueBlock, falseBlock);
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, a, b, falseBlock, trueBlock);
			else
				block->AddInstruction(ILOP_IF_BELOW, a, b, falseBlock, trueBlock);
		}
		break;
	default:
		a = GenerateIL(state, func, block);
		block->AddInstruction(ILOP_IF_TRUE, a, ILParameter(trueBlock), ILParameter(falseBlock));
		break;
	}
}


ILParameter Expr::GenerateArrayAccessIL(ParserState* state, Function* func, ILBlock*& block)
{
	ILParameter result;

	switch (m_class)
	{
	case EXPR_STRING:
		result = ILParameter(m_stringValue);
		break;
	case EXPR_VARIABLE:
		result = ILParameter(m_variable);
		break;
	case EXPR_DOT:
		result = ILParameter(m_children[0]->GenerateIL(state, func, block), m_children[0]->GetType(), m_stringValue);
		result.type = ILParameter::ReduceType(m_type);
		break;
	default:
		state->Error();
		fprintf(stderr, "%s:%d: error: invalid array access\n", m_location.fileName.c_str(),
			m_location.lineNumber);
		break;
	}

	return result;
}


ILParameter Expr::GenerateIL(ParserState* state, Function* func, ILBlock*& block)
{
	ILParameter result, a, b, c, d;
	vector<ILParameter> params;
	ILBlock* trueBlock;
	ILBlock* falseBlock;
	ILBlock* testBlock;
	ILBlock* loopBlock;
	ILBlock* endBlock;

	switch (m_class)
	{
	case EXPR_SEQUENCE:
		for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
			result = (*i)->GenerateIL(state, func, block);
		break;
	case EXPR_INT:
		result = ILParameter(m_type, m_intValue);
		break;
	case EXPR_FLOAT:
		result = ILParameter(m_type, m_floatValue);
		break;
	case EXPR_STRING:
		result = func->CreateTempVariable(Type::PointerType(m_type->GetChildType(), 1));
		block->AddInstruction(ILOP_ADDRESS_OF, result, ILParameter(m_stringValue));
		break;
	case EXPR_TRUE:
		result = ILParameter(true);
		break;
	case EXPR_FALSE:
		result = ILParameter(false);
		break;
	case EXPR_VARIABLE:
		if (m_type->GetClass() == TYPE_ARRAY)
		{
			result = func->CreateTempVariable(Type::PointerType(m_type->GetChildType(), 1));
			block->AddInstruction(ILOP_ADDRESS_OF, result, ILParameter(m_variable));
		}
		else
		{
			result = ILParameter(m_variable);
		}
		break;
	case EXPR_FUNCTION:
		result = ILParameter(m_function);
		break;
	case EXPR_DOT:
		a = ILParameter(m_children[0]->GenerateIL(state, func, block), m_children[0]->GetType(), m_stringValue);
		a.type = ILParameter::ReduceType(m_type);
		if (m_type->GetClass() == TYPE_ARRAY)
		{
			result = func->CreateTempVariable(Type::PointerType(m_type->GetChildType(), 1));
			block->AddInstruction(ILOP_ADDRESS_OF, result, a);
		}
		else
		{
			result = a;
		}
		break;
	case EXPR_ARROW:
		if (m_type->GetClass() == TYPE_ARRAY)
		{
			result = func->CreateTempVariable(Type::PointerType(m_type->GetChildType(), 1));
			block->AddInstruction(ILOP_ADDRESS_OF_MEMBER, result, m_children[0]->GenerateIL(state, func, block),
				ILParameter(m_children[0]->GetType()->GetChildType()->GetStruct(), m_stringValue));
		}
		else
		{
			result = func->CreateTempVariable(m_type);
			block->AddInstruction(ILOP_DEREF_MEMBER, result, m_children[0]->GenerateIL(state, func, block),
				ILParameter(m_children[0]->GetType()->GetChildType()->GetStruct(), m_stringValue));
		}
		break;
	case EXPR_ADDRESS_OF:
		result = func->CreateTempVariable(m_type);
		if (m_children[0]->GetClass() == EXPR_ARRAY_INDEX)
		{
			a = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			b = m_children[0]->m_children[1]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_PTR_ADD, result, a, b, ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		else if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			block->AddInstruction(ILOP_ADDRESS_OF_MEMBER, result, m_children[0]->m_children[0]->GenerateIL(state, func, block),
				ILParameter(m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue));
		}
		else
		{
			block->AddInstruction(ILOP_ADDRESS_OF, result, m_children[0]->GenerateIL(state, func, block));
		}
		break;
	case EXPR_DEREF:
		result = func->CreateTempVariable(m_type);
		block->AddInstruction(ILOP_DEREF, result, m_children[0]->GenerateIL(state, func, block));
		break;
	case EXPR_PRE_INCREMENT:
		result = m_children[0]->GenerateIL(state, func, block);
		if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			block->AddInstruction(ILOP_PTR_ADD, result, result,
				ILParameter(Type::IntType(GetTargetPointerSize(), false), (int64_t)1),
				ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		else
		{
			block->AddInstruction(ILOP_ADD, result, result, ILParameter(result.type, (int64_t)1));
		}
		if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			a = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_MEMBER_ASSIGN, a, ILParameter(
				m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue), result);
		}
		break;
	case EXPR_PRE_DECREMENT:
		result = m_children[0]->GenerateIL(state, func, block);
		if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			block->AddInstruction(ILOP_PTR_SUB, result, result,
				ILParameter(Type::IntType(GetTargetPointerSize(), false), (int64_t)1),
				ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		else
		{
			block->AddInstruction(ILOP_SUB, result, result, ILParameter(result.type, (int64_t)1));
		}
		if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			a = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_MEMBER_ASSIGN, a, ILParameter(
				m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue), result);
		}
		break;
	case EXPR_POST_INCREMENT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_ASSIGN, result, a);
		if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			block->AddInstruction(ILOP_PTR_ADD, a, a,
				ILParameter(Type::IntType(GetTargetPointerSize(), false), (int64_t)1),
				ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		else
		{
			block->AddInstruction(ILOP_ADD, a, a, ILParameter(result.type, (int64_t)1));
		}
		if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			b = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_MEMBER_ASSIGN, b, ILParameter(
				m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue), a);
		}
		break;
	case EXPR_POST_DECREMENT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_ASSIGN, result, a);
		if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			block->AddInstruction(ILOP_PTR_SUB, a, a,
				ILParameter(Type::IntType(GetTargetPointerSize(), false), (int64_t)1),
				ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		else
		{
			block->AddInstruction(ILOP_SUB, a, a, ILParameter(result.type, (int64_t)1));
		}
		if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			b = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_MEMBER_ASSIGN, b, ILParameter(
				m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue), a);
		}
		break;
	case EXPR_ARRAY_INDEX:
		result = func->CreateTempVariable(m_type);
		if ((m_children[0]->GetType()->GetClass() == TYPE_POINTER) ||
			(m_children[0]->GetClass() == EXPR_ARROW))
		{
			a = m_children[0]->GenerateIL(state, func, block);
			b = m_children[1]->GenerateIL(state, func, block);
			if (m_children[0]->GetType()->GetClass() == TYPE_ARRAY)
				c = func->CreateTempVariable(Type::PointerType(m_children[0]->GetType()->GetChildType(), 1));
			else
				c = func->CreateTempVariable(m_children[0]->GetType());
			block->AddInstruction(ILOP_PTR_ADD, c, a, b, ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
			block->AddInstruction(ILOP_DEREF, result, c);
		}
		else
		{
			a = m_children[0]->GenerateArrayAccessIL(state, func, block);
			b = m_children[1]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_ARRAY_INDEX, result, a, b, ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		break;
	case EXPR_PLUS:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			block->AddInstruction(ILOP_PTR_ADD, result, a, b, ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
		}
		else if (m_children[1]->GetType()->GetClass() == TYPE_POINTER)
		{
			block->AddInstruction(ILOP_PTR_ADD, result, b, a, ILParameter(Type::IntType(GetTargetPointerSize(), false),
				(int64_t)m_children[1]->GetType()->GetChildType()->GetWidth()));
		}
		else
		{
			block->AddInstruction(ILOP_ADD, result, a, b);
		}
		break;
	case EXPR_MINUS:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (m_children[0]->GetType()->GetClass() == TYPE_POINTER)
		{
			if (m_children[1]->GetType()->GetClass() == TYPE_POINTER)
			{
				block->AddInstruction(ILOP_PTR_DIFF, result, a, b,
					ILParameter(Type::IntType(GetTargetPointerSize(), false),
					(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
			}
			else
			{
				block->AddInstruction(ILOP_PTR_SUB, result, a, b,
					ILParameter(Type::IntType(GetTargetPointerSize(), false),
					(int64_t)m_children[0]->GetType()->GetChildType()->GetWidth()));
			}
		}
		else
		{
			block->AddInstruction(ILOP_SUB, result, a, b);
		}
		break;
	case EXPR_MULT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (m_children[0]->GetType()->IsSigned())
			block->AddInstruction(ILOP_SMULT, result, a, b);
		else
			block->AddInstruction(ILOP_UMULT, result, a, b);
		break;
	case EXPR_DIV:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if ((GetTargetPointerSize() == 4) && (m_children[0]->GetType()->GetWidth() == 8))
		{
			// 64-bit division on 32-bit output, emit call to divide routine
			string name;
			if (m_children[0]->GetType()->IsSigned())
				name = "__sdiv64";
			else
				name = "__udiv64";
			map< string, Ref<Function> >::const_iterator i = state->GetFunctions().find(name);
			if (i == state->GetFunctions().end())
			{
				fprintf(stderr, "%s:%d: error: undefined function '%s'\n", m_location.fileName.c_str(), m_location.lineNumber,
					name.c_str());
				break;
			}
			block->AddInstruction(ILOP_CALL, result, ILParameter(i->second), a, b);
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_SDIV, result, a, b);
			else
				block->AddInstruction(ILOP_UDIV, result, a, b);
		}
		break;
	case EXPR_MOD:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if ((GetTargetPointerSize() == 4) && (m_children[0]->GetType()->GetWidth() == 8))
		{
			// 64-bit division on 32-bit output, emit call to divide routine
			string name;
			if (m_children[0]->GetType()->IsSigned())
				name = "__smod64";
			else
				name = "__umod64";
			map< string, Ref<Function> >::const_iterator i = state->GetFunctions().find(name);
			if (i == state->GetFunctions().end())
			{
				fprintf(stderr, "%s:%d: error: undefined function '%s'\n", m_location.fileName.c_str(), m_location.lineNumber,
					name.c_str());
				break;
			}
			block->AddInstruction(ILOP_CALL, result, ILParameter(i->second), a, b);
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_SMOD, result, a, b);
			else
				block->AddInstruction(ILOP_UMOD, result, a, b);
		}
		break;
	case EXPR_AND:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_AND, result, a, b);
		break;
	case EXPR_OR:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_OR, result, a, b);
		break;
	case EXPR_XOR:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_XOR, result, a, b);
		break;
	case EXPR_SHIFT_LEFT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_SHL, result, a, b);
		break;
	case EXPR_SHIFT_RIGHT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if (m_children[0]->GetType()->IsSigned())
			block->AddInstruction(ILOP_SAR, result, a, b);
		else
			block->AddInstruction(ILOP_SHR, result, a, b);
		break;
	case EXPR_NEG:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_NEG, result, a);
		break;
	case EXPR_NOT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_NOT, result, a);
		break;
	case EXPR_LOGICAL_AND:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		block->AddInstruction(ILOP_IF_TRUE, a, ILParameter(trueBlock), ILParameter(falseBlock));
		b = m_children[1]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_ASSIGN, result, b);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_LOGICAL_OR:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		block->AddInstruction(ILOP_IF_TRUE, a, ILParameter(trueBlock), ILParameter(falseBlock));
		b = m_children[1]->GenerateIL(state, func, falseBlock);
		falseBlock->AddInstruction(ILOP_ASSIGN, result, b);
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_LOGICAL_NOT:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		block->AddInstruction(ILOP_IF_TRUE, a, ILParameter(trueBlock), ILParameter(falseBlock));
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_LESS_THAN:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
		}
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_LESS_EQUAL:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
		}
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_EQUAL:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		if (a.IsConstant())
			block->AddInstruction(ILOP_IF_EQUAL, b, a, ILParameter(trueBlock), ILParameter(falseBlock));
		else
			block->AddInstruction(ILOP_IF_EQUAL, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_NOT_EQUAL:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		if (a.IsConstant())
			block->AddInstruction(ILOP_IF_EQUAL, b, a, ILParameter(trueBlock), ILParameter(falseBlock));
		else
			block->AddInstruction(ILOP_IF_EQUAL, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_GREATER_EQUAL:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
		}
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_GREATER_THAN:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		if (a.IsConstant())
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW, b, a, ILParameter(falseBlock), ILParameter(trueBlock));
		}
		else
		{
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_EQUAL, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW_EQUAL, a, b, ILParameter(trueBlock), ILParameter(falseBlock));
		}
		trueBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(false));
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		falseBlock->AddInstruction(ILOP_ASSIGN, result, ILParameter(true));
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_ASSIGN:
		if (m_children[0]->GetClass() == EXPR_ARRAY_INDEX)
		{
			if ((m_children[0]->m_children[0]->GetType()->GetClass() == TYPE_POINTER) ||
				(m_children[0]->m_children[0]->GetClass() == EXPR_ARROW))
			{
				if (m_children[0]->m_children[0]->GetType()->GetClass() == TYPE_ARRAY)
					result = func->CreateTempVariable(Type::PointerType(m_children[0]->m_children[0]->GetType()->GetChildType(), 1));
				else
					result = func->CreateTempVariable(m_children[0]->m_children[0]->GetType());
				a = m_children[0]->m_children[0]->GenerateIL(state, func, block);
				b = m_children[0]->m_children[1]->GenerateIL(state, func, block);
				block->AddInstruction(ILOP_PTR_ADD, result, a, b,
					ILParameter(Type::IntType(GetTargetPointerSize(), false),
					(int64_t)m_children[0]->m_children[0]->GetType()->GetChildType()->GetWidth()));
				c = m_children[1]->GenerateIL(state, func, block);
				block->AddInstruction(ILOP_DEREF_ASSIGN, result, c);
				result = c;
			}
			else
			{
				a = m_children[0]->m_children[0]->GenerateArrayAccessIL(state, func, block);
				b = m_children[0]->m_children[1]->GenerateIL(state, func, block);
				c = m_children[1]->GenerateIL(state, func, block);
				block->AddInstruction(ILOP_ARRAY_INDEX_ASSIGN, a, b,
					ILParameter(Type::IntType(GetTargetPointerSize(), false),
					(int64_t)m_children[0]->m_children[0]->GetType()->GetChildType()->GetWidth()), c);
				result = c;
			}
		}
		else if (m_children[0]->GetClass() == EXPR_DEREF)
		{
			a = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			b = m_children[1]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_ASSIGN, a, b);
			result = b;
		}
		else if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			a = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			b = m_children[1]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_MEMBER_ASSIGN, a, ILParameter(
				m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue), b);
			result = b;
		}
		else
		{
			a = m_children[0]->GenerateIL(state, func, block);
			b = m_children[1]->GenerateIL(state, func, block);
			if ((a.cls != ILPARAM_VAR) && (a.cls != ILPARAM_MEMBER))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: expected lvalue\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
			if ((a.cls == ILPARAM_VAR) && (a.variable->IsTempVariable()))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: expected lvalue\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
			block->AddInstruction(ILOP_ASSIGN, a, b);
			result = b;
		}
		break;
	case EXPR_INIT_ASSIGN:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		if ((a.cls != ILPARAM_VAR) && (a.cls != ILPARAM_MEMBER))
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: expected lvalue\n", m_location.fileName.c_str(), m_location.lineNumber);
		}
		block->AddInstruction(ILOP_ASSIGN, a, b);
		result = b;
		break;
	case EXPR_IF:
		trueBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		m_children[0]->GenerateConditionalIL(state, func, block, trueBlock, endBlock);
		m_children[1]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_IF_ELSE:
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		m_children[0]->GenerateConditionalIL(state, func, block, trueBlock, falseBlock);
		m_children[1]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		m_children[2]->GenerateIL(state, func, falseBlock);
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_IF_ELSE_VALUE:
		result = func->CreateTempVariable(m_type);
		trueBlock = func->CreateILBlock();
		falseBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		m_children[0]->GenerateConditionalIL(state, func, block, trueBlock, falseBlock);
		b = m_children[1]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_ASSIGN, result, b);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		b = m_children[2]->GenerateIL(state, func, falseBlock);
		falseBlock->AddInstruction(ILOP_ASSIGN, result, b);
		falseBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		break;
	case EXPR_FOR:
		m_children[0]->GenerateIL(state, func, block);
		testBlock = func->CreateILBlock();
		loopBlock = func->CreateILBlock();
		trueBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		func->PushBreakBlock(endBlock);
		func->PushContinueBlock(loopBlock);
		block->AddInstruction(ILOP_GOTO, ILParameter(testBlock));
		m_children[1]->GenerateConditionalIL(state, func, testBlock, trueBlock, endBlock);
		m_children[3]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(loopBlock));
		m_children[2]->GenerateIL(state, func, loopBlock);
		loopBlock->AddInstruction(ILOP_GOTO, ILParameter(testBlock));
		block = endBlock;
		func->PopBreakBlock();
		func->PopContinueBlock();
		break;
	case EXPR_WHILE:
		loopBlock = func->CreateILBlock();
		trueBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		func->PushBreakBlock(endBlock);
		func->PushContinueBlock(loopBlock);
		block->AddInstruction(ILOP_GOTO, ILParameter(loopBlock));
		m_children[0]->GenerateConditionalIL(state, func, loopBlock, trueBlock, endBlock);
		m_children[1]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(loopBlock));
		block = endBlock;
		func->PopBreakBlock();
		func->PopContinueBlock();
		break;
	case EXPR_DO_WHILE:
		loopBlock = func->CreateILBlock();
		trueBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		func->PushBreakBlock(endBlock);
		func->PushContinueBlock(loopBlock);
		block->AddInstruction(ILOP_GOTO, ILParameter(trueBlock));
		m_children[0]->GenerateConditionalIL(state, func, loopBlock, trueBlock, endBlock);
		m_children[1]->GenerateIL(state, func, trueBlock);
		trueBlock->AddInstruction(ILOP_GOTO, ILParameter(loopBlock));
		block = endBlock;
		func->PopBreakBlock();
		func->PopContinueBlock();
		break;
	case EXPR_CALL:
		if (m_type->GetClass() != TYPE_VOID)
			result = func->CreateTempVariable(m_type);
		params.push_back(result);
		params.push_back(m_children[0]->GenerateIL(state, func, block));
		for (size_t i = 1; i < m_children.size(); i++)
			params.push_back(m_children[i]->GenerateIL(state, func, block));
		block->AddInstruction(ILOP_CALL, params);
		break;
	case EXPR_MIN:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_ASSIGN, result, a);
		for (size_t i = 1; i < m_children.size(); i++)
		{
			a = m_children[i]->GenerateIL(state, func, block);
			trueBlock = func->CreateILBlock();
			falseBlock = func->CreateILBlock();
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, a, result, ILParameter(trueBlock), ILParameter(falseBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW, a, result, ILParameter(trueBlock), ILParameter(falseBlock));
			trueBlock->AddInstruction(ILOP_ASSIGN, result, a);
			trueBlock->AddInstruction(ILOP_GOTO, ILParameter(falseBlock));
			block = falseBlock;
		}
		break;
	case EXPR_MAX:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_ASSIGN, result, a);
		for (size_t i = 1; i < m_children.size(); i++)
		{
			a = m_children[i]->GenerateIL(state, func, block);
			trueBlock = func->CreateILBlock();
			falseBlock = func->CreateILBlock();
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_IF_LESS_THAN, result, a, ILParameter(trueBlock), ILParameter(falseBlock));
			else
				block->AddInstruction(ILOP_IF_BELOW, result, a, ILParameter(trueBlock), ILParameter(falseBlock));
			trueBlock->AddInstruction(ILOP_ASSIGN, result, a);
			trueBlock->AddInstruction(ILOP_GOTO, ILParameter(falseBlock));
			block = falseBlock;
		}
		break;
	case EXPR_ABS:
		if (!m_type->IsSigned())
			result = m_children[0]->GenerateIL(state, func, block);
		else
		{
			result = func->CreateTempVariable(m_type);
			trueBlock = func->CreateILBlock();
			falseBlock = func->CreateILBlock();
			a = m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_ASSIGN, result, a);
			if (m_type->GetClass() == TYPE_FLOAT)
			{
				block->AddInstruction(ILOP_IF_LESS_THAN, result, ILParameter(m_type, 0.0),
					ILParameter(trueBlock), ILParameter(falseBlock));
			}
			else
			{
				block->AddInstruction(ILOP_IF_LESS_THAN, result, ILParameter(m_type, (int64_t)0),
					ILParameter(trueBlock), ILParameter(falseBlock));
			}
			trueBlock->AddInstruction(ILOP_NEG, result, a);
			trueBlock->AddInstruction(ILOP_GOTO, ILParameter(falseBlock));
			block = falseBlock;
		}
		break;
	case EXPR_ALLOCA:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_ALLOCA, result, a);
		func->MarkVariableSizedStackFrame();
		break;
	case EXPR_MEMCPY:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		c = m_children[2]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_MEMCPY, a, b, c);
		result = a;
		break;
	case EXPR_MEMSET:
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		c = m_children[2]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_MEMSET, a, b, c);
		result = a;
		break;
	case EXPR_STRLEN:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_STRLEN, result, a);
		break;
	case EXPR_CAST:
		if (ILParameter::ReduceType(m_type) == ILParameter::ReduceType(m_children[0]->GetType()))
			result = m_children[0]->GenerateIL(state, func, block);
		else if (m_children[0]->GetClass() == EXPR_INT)
		{
			result = m_children[0]->GenerateIL(state, func, block);
			result.type = ILParameter::ReduceType(m_type);
		}
		else
		{
			result = func->CreateTempVariable(m_type);
			a = m_children[0]->GenerateIL(state, func, block);
			if (m_children[0]->GetType()->IsSigned())
				block->AddInstruction(ILOP_SCONVERT, result, a);
			else
				block->AddInstruction(ILOP_UCONVERT, result, a);
		}
		break;
	case EXPR_RETURN:
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_RETURN, a);
		break;
	case EXPR_RETURN_VOID:
		block->AddInstruction(ILOP_RETURN_VOID);
		break;
	case EXPR_LABEL:
		endBlock = func->CreateILBlock();
		block->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		block = endBlock;
		func->SetLabel(m_stringValue, block);
		break;
	case EXPR_GOTO_LABEL:
		endBlock = func->GetLabel(m_stringValue);
		if (!endBlock)
		{
			func->AddLabelFixup(block, block->GetInstructions().size(), m_location, m_stringValue);
			block->AddInstruction(ILOP_GOTO, ILParameter((ILBlock*)NULL));
		}
		else
		{
			block->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		}
		break;
	case EXPR_COMPUTED_GOTO:
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_GOTO, a);
		break;
	case EXPR_BREAK:
		endBlock = func->GetBreakBlock();
		if (!endBlock)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: invalid break statement\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		else
		{
			block->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		}
		break;
	case EXPR_CONTINUE:
		endBlock = func->GetContinueBlock();
		if (!endBlock)
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: invalid continue statement\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		else
		{
			block->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		}
		break;
	case EXPR_SWITCH:
		a = m_children[0]->GenerateIL(state, func, block);
		testBlock = func->CreateILBlock();
		endBlock = func->CreateILBlock();
		func->PushSwitchLabels();
		func->PushBreakBlock(endBlock);
		m_children[1]->GenerateIL(state, func, testBlock);
		testBlock->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		for (map<int64_t, ILBlock*>::const_iterator i = func->GetSwitchLabels().begin();
			i != func->GetSwitchLabels().end(); i++)
		{
			falseBlock = func->CreateILBlock();
			block->AddInstruction(ILOP_IF_EQUAL, a, ILParameter(a.type, i->first), i->second, ILParameter(falseBlock));
			block = falseBlock;
		}
		falseBlock = func->GetDefaultBlock();
		block->AddInstruction(ILOP_GOTO, ILParameter(falseBlock ? falseBlock : endBlock));
		block = endBlock;
		func->PopSwitchLabels();
		func->PopBreakBlock();
		break;
	case EXPR_CASE:
		endBlock = func->CreateILBlock();
		block->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		if (func->GetSwitchLabels().find(m_intValue) != func->GetSwitchLabels().end())
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: duplicate case label in switch statement\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		func->AddSwitchLabel(m_intValue, endBlock);
		block = endBlock;
		break;
	case EXPR_DEFAULT:
		endBlock = func->CreateILBlock();
		block->AddInstruction(ILOP_GOTO, ILParameter(endBlock));
		if (func->GetDefaultBlock())
		{
			state->Error();
			fprintf(stderr, "%s:%d: error: duplicate default label in switch statement\n", m_location.fileName.c_str(),
				m_location.lineNumber);
		}
		func->SetDefaultBlock(endBlock);
		block = endBlock;
		break;
	case EXPR_UNDEFINED:
		result = ILParameter();
		result.cls = ILPARAM_UNDEFINED;
		result.type = ILParameter::ReduceType(m_type);
		break;
	case EXPR_SYSCALL:
		result = func->CreateTempVariable(m_type);
		params.push_back(result);
		params.push_back(m_children[0]->GenerateIL(state, func, block));
		for (size_t i = 1; i < m_children.size(); i++)
			params.push_back(m_children[i]->GenerateIL(state, func, block));
		block->AddInstruction(ILOP_SYSCALL, params);
		break;
	case EXPR_SYSCALL2:
		a = func->CreateTempVariable(m_type);
		b = func->CreateTempVariable(m_type);
		result = a;
		params.push_back(a);
		params.push_back(b);
		params.push_back(m_children[1]->GenerateIL(state, func, block));
		for (size_t i = 2; i < m_children.size(); i++)
			params.push_back(m_children[i]->GenerateIL(state, func, block));
		block->AddInstruction(ILOP_SYSCALL2, params);

		if (m_children[0]->GetClass() == EXPR_ARRAY_INDEX)
		{
			if ((m_children[0]->m_children[0]->GetType()->GetClass() == TYPE_POINTER) ||
				(m_children[0]->m_children[0]->GetClass() == EXPR_ARROW))
			{
				if (m_children[0]->m_children[0]->GetType()->GetClass() == TYPE_ARRAY)
					a = func->CreateTempVariable(Type::PointerType(m_children[0]->m_children[0]->GetType()->GetChildType(), 1));
				else
					a = func->CreateTempVariable(m_children[0]->m_children[0]->GetType());
				c = m_children[0]->m_children[0]->GenerateIL(state, func, block);
				d = m_children[0]->m_children[1]->GenerateIL(state, func, block);
				block->AddInstruction(ILOP_PTR_ADD, a, c, d,
					ILParameter(Type::IntType(GetTargetPointerSize(), false),
					(int64_t)m_children[0]->m_children[0]->GetType()->GetChildType()->GetWidth()));
				block->AddInstruction(ILOP_DEREF_ASSIGN, a, b);
			}
			else
			{
				c = m_children[0]->m_children[0]->GenerateArrayAccessIL(state, func, block);
				d = m_children[0]->m_children[1]->GenerateIL(state, func, block);
				block->AddInstruction(ILOP_ARRAY_INDEX_ASSIGN, c, d,
					ILParameter(Type::IntType(GetTargetPointerSize(), false),
					(int64_t)m_children[0]->m_children[0]->GetType()->GetChildType()->GetWidth()), b);
			}
		}
		else if (m_children[0]->GetClass() == EXPR_DEREF)
		{
			c = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_ASSIGN, c, b);
		}
		else if (m_children[0]->GetClass() == EXPR_ARROW)
		{
			c = m_children[0]->m_children[0]->GenerateIL(state, func, block);
			block->AddInstruction(ILOP_DEREF_MEMBER_ASSIGN, c, ILParameter(
				m_children[0]->m_children[0]->GetType()->GetChildType()->GetStruct(),
				m_children[0]->m_stringValue), b);
		}
		else
		{
			c = m_children[0]->GenerateIL(state, func, block);
			if ((c.cls != ILPARAM_VAR) && (c.cls != ILPARAM_MEMBER))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: expected lvalue\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
			if ((c.cls == ILPARAM_VAR) && (c.variable->IsTempVariable()))
			{
				state->Error();
				fprintf(stderr, "%s:%d: error: expected lvalue\n", m_location.fileName.c_str(), m_location.lineNumber);
			}
			block->AddInstruction(ILOP_ASSIGN, c, b);
		}
		break;
	case EXPR_RDTSC:
		result = func->CreateTempVariable(m_type);
		block->AddInstruction(ILOP_RDTSC, result);
		break;
	case EXPR_RDTSC_LOW:
		result = func->CreateTempVariable(m_type);
		block->AddInstruction(ILOP_RDTSC_LOW, result);
		break;
	case EXPR_RDTSC_HIGH:
		result = func->CreateTempVariable(m_type);
		block->AddInstruction(ILOP_RDTSC_HIGH, result);
		break;
	case EXPR_NEXT_ARG:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_NEXT_ARG, result, a, b);
		break;
	case EXPR_PREV_ARG:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		b = m_children[1]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_PREV_ARG, result, a, b);
		break;
	case EXPR_BYTESWAP:
		result = func->CreateTempVariable(m_type);
		a = m_children[0]->GenerateIL(state, func, block);
		block->AddInstruction(ILOP_BYTESWAP, result, a);
		break;
	default:
		state->Error();
		fprintf(stderr, "%s:%d: error: invalid expression in IL generation\n", m_location.fileName.c_str(),
			m_location.lineNumber);
		break;
	}

	return result;
}


Expr* Expr::BoolExpr(const Location& loc, bool value)
{
	return new Expr(loc, value ? EXPR_TRUE : EXPR_FALSE);
}


Expr* Expr::IntExpr(const Location& loc, int64_t value)
{
	Expr* expr = new Expr(loc, EXPR_INT);
	expr->m_intValue = value;
	return expr;
}


Expr* Expr::FloatExpr(const Location& loc, double value)
{
	Expr* expr = new Expr(loc, EXPR_FLOAT);
	expr->m_floatValue = value;
	return expr;
}


Expr* Expr::StringExpr(const Location& loc, const string& value)
{
	Expr* expr = new Expr(loc, EXPR_STRING);
	expr->m_stringValue = value;
	return expr;
}


Expr* Expr::VariableExpr(const Location& loc, Variable* var)
{
	Expr* expr = new Expr(loc, EXPR_VARIABLE);
	expr->m_variable = var;
	return expr;
}


Expr* Expr::FunctionExpr(const Location& loc, Function* func)
{
	Expr* expr = new Expr(loc, EXPR_FUNCTION);
	expr->m_function = func;
	return expr;
}


Expr* Expr::DotExpr(const Location& loc, Expr* left, const string& right)
{
	Expr* expr = new Expr(loc, EXPR_DOT);
	expr->m_children.push_back(left);
	expr->m_stringValue = right;
	return expr;
}


Expr* Expr::ArrowExpr(const Location& loc, Expr* left, const string& right)
{
	Expr* expr = new Expr(loc, EXPR_ARROW);
	expr->m_children.push_back(left);
	expr->m_stringValue = right;
	return expr;
}


Expr* Expr::UnaryExpr(const Location& loc, ExprClass cls, Expr* child)
{
	Expr* expr = new Expr(loc, cls);
	expr->m_children.push_back(child);
	return expr;
}


Expr* Expr::BinaryExpr(const Location& loc, ExprClass cls, Expr* left, Expr* right)
{
	Expr* expr = new Expr(loc, cls);
	expr->m_children.push_back(left);
	expr->m_children.push_back(right);
	return expr;
}


Expr* Expr::IfExpr(const Location& loc, Expr* cond, Expr* ifTrue)
{
	Expr* expr = new Expr(loc, EXPR_IF);
	expr->m_children.push_back(cond);
	expr->m_children.push_back(ifTrue);
	return expr;
}


Expr* Expr::IfElseExpr(const Location& loc, Expr* cond, Expr* ifTrue, Expr* ifFalse)
{
	Expr* expr = new Expr(loc, EXPR_IF_ELSE);
	expr->m_children.push_back(cond);
	expr->m_children.push_back(ifTrue);
	expr->m_children.push_back(ifFalse);
	return expr;
}


Expr* Expr::IfElseValueExpr(const Location& loc, Expr* cond, Expr* ifTrue, Expr* ifFalse)
{
	Expr* expr = new Expr(loc, EXPR_IF_ELSE_VALUE);
	expr->m_children.push_back(cond);
	expr->m_children.push_back(ifTrue);
	expr->m_children.push_back(ifFalse);
	return expr;
}


Expr* Expr::ForExpr(const Location& loc, Expr* init, Expr* cond, Expr* update, Expr* body)
{
	Expr* expr = new Expr(loc, EXPR_FOR);
	expr->m_children.push_back(init);
	expr->m_children.push_back(cond);
	expr->m_children.push_back(update);
	expr->m_children.push_back(body);
	return expr;
}


Expr* Expr::WhileExpr(const Location& loc, Expr* cond, Expr* body)
{
	Expr* expr = new Expr(loc, EXPR_WHILE);
	expr->m_children.push_back(cond);
	expr->m_children.push_back(body);
	return expr;
}


Expr* Expr::DoWhileExpr(const Location& loc, Expr* cond, Expr* body)
{
	Expr* expr = new Expr(loc, EXPR_DO_WHILE);
	expr->m_children.push_back(cond);
	expr->m_children.push_back(body);
	return expr;
}


Expr* Expr::CallExpr(const Location& loc, Expr* func, const vector< Ref<Expr> >& params)
{
	Expr* expr = new Expr(loc, EXPR_CALL);
	expr->m_children.push_back(func);
	expr->m_children.insert(expr->m_children.end(), params.begin(), params.end());
	return expr;
}


Expr* Expr::BuiltinCallExpr(const Location& loc, ExprClass cls, const vector< Ref<Expr> >& params)
{
	Expr* expr = new Expr(loc, cls);
	expr->m_children = params;
	return expr;
}


Expr* Expr::CastExpr(const Location& loc, Type* type, Expr* value)
{
	Expr* expr = new Expr(loc, EXPR_CAST);
	expr->m_type = type;
	expr->m_children.push_back(value);
	return expr;
}


Expr* Expr::LabelExpr(const Location& loc, const string& value)
{
	Expr* expr = new Expr(loc, EXPR_LABEL);
	expr->m_stringValue = value;
	return expr;
}


Expr* Expr::GotoLabelExpr(const Location& loc, const string& value)
{
	Expr* expr = new Expr(loc, EXPR_GOTO_LABEL);
	expr->m_stringValue = value;
	return expr;
}


void Expr::Serialize(OutputBlock* output)
{
	output->WriteInteger(m_class);
	output->WriteInteger((m_type != NULL) ? 1 : 0);
	if (m_type)
		m_type->Serialize(output);

	output->WriteInteger(m_children.size());
	for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		(*i)->Serialize(output);

	switch (m_class)
	{
	case EXPR_INT:
	case EXPR_CASE:
		output->WriteInteger(m_intValue);
		break;
	case EXPR_FLOAT:
		output->Write(&m_floatValue, sizeof(m_floatValue));
		break;
	case EXPR_STRING:
	case EXPR_DOT:
	case EXPR_ARROW:
	case EXPR_LABEL:
	case EXPR_GOTO_LABEL:
		output->WriteString(m_stringValue);
		break;
	case EXPR_VARIABLE:
		m_variable->Serialize(output);
		break;
	case EXPR_FUNCTION:
		m_function->Serialize(output);
		break;
	default:
		break;
	}
}


bool Expr::DeserializeInternal(InputBlock* input)
{
	uint32_t cls;
	if (!input->ReadUInt32(cls))
		return false;
	m_class = (ExprClass)cls;

	bool hasType;
	if (!input->ReadBool(hasType))
		return false;
	if (hasType)
	{
		m_type = Type::Deserialize(input);
		if (!m_type)
			return false;
	}
	else
	{
		m_type = NULL;
	}

	size_t childCount;
	if (!input->ReadNativeInteger(childCount))
		return false;
	for (size_t i = 0; i < childCount; i++)
	{
		Expr* child = Expr::Deserialize(input);
		if (!child)
			return false;
		m_children.push_back(child);
	}

	switch (m_class)
	{
	case EXPR_INT:
	case EXPR_CASE:
		if (!input->ReadInt64(m_intValue))
			return false;
		break;
	case EXPR_FLOAT:
		if (!input->Read(&m_floatValue, sizeof(m_floatValue)))
			return false;
		break;
	case EXPR_STRING:
	case EXPR_DOT:
	case EXPR_ARROW:
	case EXPR_LABEL:
	case EXPR_GOTO_LABEL:
		if (!input->ReadString(m_stringValue))
			return false;
		break;
	case EXPR_VARIABLE:
		m_variable = Variable::Deserialize(input);
		if (!m_variable)
			return false;
		break;
	case EXPR_FUNCTION:
		m_function = Function::Deserialize(input);
		if (!m_function)
			return false;
		break;
	default:
		break;
	}

	return true;
}


Expr* Expr::Deserialize(InputBlock* input)
{
	Expr* expr = new Expr(EXPR_SEQUENCE);
	if (expr->DeserializeInternal(input))
		return expr;
	delete expr;
	return NULL;
}


#ifndef WIN32
static void PrintIndent(size_t indent)
{
	for (size_t i = 0; i < indent; i++)
		fprintf(stderr, "\t");
}


void Expr::Print(size_t indent)
{
	switch (m_class)
	{
	case EXPR_SEQUENCE:
		if (m_children.size() == 0)
		{
			fprintf(stderr, "{}");
			break;
		}

		fprintf(stderr, "{\n");
		PrintIndent(indent + 1);

		for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		{
			(*i)->Print(indent + 1);
			fprintf(stderr, "\n");
			if ((i + 1) != m_children.end())
				PrintIndent(indent + 1);
			else
				PrintIndent(indent);
		}

		fprintf(stderr, "}");
		break;
	case EXPR_INITIALIZER:
		fprintf(stderr, "{");
		for (vector< Ref<Expr> >::iterator i = m_children.begin(); i != m_children.end(); i++)
		{
			if (i != m_children.begin())
				fprintf(stderr, ", ");
			(*i)->Print(indent);
		}
		fprintf(stderr, "}");
		break;
	case EXPR_INT:  fprintf(stderr, "%lld {%d}", (long long)m_intValue, (int)m_type->GetWidth()); break;
	case EXPR_FLOAT:  fprintf(stderr, "%f", m_floatValue); break;
	case EXPR_STRING:  fprintf(stderr, "\"%s\"", m_stringValue.c_str()); break;
	case EXPR_TRUE:  fprintf(stderr, "true"); break;
	case EXPR_FALSE:  fprintf(stderr, "false"); break;
	case EXPR_VARIABLE:  fprintf(stderr, "var<%s>", m_variable->GetName().c_str()); break;
	case EXPR_FUNCTION:  fprintf(stderr, "func<%s>", m_function->GetName().c_str()); break;
	case EXPR_DOT:  m_children[0]->Print(indent); fprintf(stderr, ".%s", m_stringValue.c_str()); break;
	case EXPR_ARROW:  m_children[0]->Print(indent); fprintf(stderr, "->%s", m_stringValue.c_str()); break;
	case EXPR_ADDRESS_OF:  fprintf(stderr, "&"); m_children[0]->Print(indent); break;
	case EXPR_DEREF:  fprintf(stderr, "*"); m_children[0]->Print(indent); break;
	case EXPR_PRE_INCREMENT:  fprintf(stderr, "++"); m_children[0]->Print(indent); break;
	case EXPR_PRE_DECREMENT:  fprintf(stderr, "--"); m_children[0]->Print(indent); break;
	case EXPR_POST_INCREMENT:  m_children[0]->Print(indent); fprintf(stderr, "++"); break;
	case EXPR_POST_DECREMENT:  m_children[0]->Print(indent); fprintf(stderr, "--"); break;
	case EXPR_ARRAY_INDEX:  m_children[0]->Print(indent); fprintf(stderr, "["); m_children[1]->Print(indent); fprintf(stderr, "]"); break;
	case EXPR_PLUS:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") + (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_MINUS:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") - (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_MULT:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") * (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_DIV:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") / (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_MOD:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") %% (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_AND:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") & (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_OR:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") | (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_XOR:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") ^ (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_SHIFT_LEFT:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") << (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_SHIFT_RIGHT:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") >> (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_NEG:  fprintf(stderr, "-("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_NOT:  fprintf(stderr, "~("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_LOGICAL_AND:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") && (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_LOGICAL_OR:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") || (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_LOGICAL_NOT:  fprintf(stderr, "!("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_LESS_THAN:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") < (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_LESS_EQUAL:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") <= (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_EQUAL:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") == (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_NOT_EQUAL:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") != (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_GREATER_EQUAL:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") >= (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_GREATER_THAN:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") > (");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_ASSIGN:  m_children[0]->Print(indent); fprintf(stderr, " = "); m_children[1]->Print(indent); break;
	case EXPR_INIT_ASSIGN:  m_children[0]->Print(indent); fprintf(stderr, " = [init] "); m_children[1]->Print(indent); break;
	case EXPR_IF:
		fprintf(stderr, "if (");
		m_children[0]->Print(indent);
		fprintf(stderr, ")\n");
		PrintIndent(indent + 1);
		m_children[1]->Print(indent + 1);
		break;
	case EXPR_IF_ELSE:
		fprintf(stderr, "if (");
		m_children[0]->Print(indent);
		fprintf(stderr, ")\n");
		PrintIndent(indent + 1);
		m_children[1]->Print(indent + 1);
		fprintf(stderr, "\n");
		PrintIndent(indent);
		fprintf(stderr, "else\n");
		PrintIndent(indent + 1);
		m_children[2]->Print(indent + 1);
		break;
	case EXPR_IF_ELSE_VALUE:
		fprintf(stderr, "(");
		m_children[0]->Print(indent);
		fprintf(stderr, ") ? (");
		m_children[1]->Print(indent);
		fprintf(stderr, ") : (");
		m_children[2]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_FOR:
		fprintf(stderr, "for (");
		m_children[0]->Print(indent);
		fprintf(stderr, "; ");
		m_children[1]->Print(indent);
		fprintf(stderr, "; ");
		m_children[2]->Print(indent);
		fprintf(stderr, ")\n");
		PrintIndent(indent + 1);
		m_children[3]->Print(indent + 1);
		break;
	case EXPR_WHILE:
		fprintf(stderr, "while (");
		m_children[0]->Print(indent);
		fprintf(stderr, ")\n");
		PrintIndent(indent + 1);
		m_children[1]->Print(indent + 1);
		break;
	case EXPR_DO_WHILE:
		fprintf(stderr, "do\n");
		PrintIndent(indent + 1);
		m_children[1]->Print(indent + 1);
		fprintf(stderr, "\n");
		PrintIndent(indent);
		fprintf(stderr, "while (");
		m_children[0]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_CALL:
		m_children[0]->Print(indent);
		fprintf(stderr, "(");
		for (size_t i = 1; i < m_children.size(); i++)
		{
			if (i > 1)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_MIN:
		fprintf(stderr, "min(");
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_MAX:
		fprintf(stderr, "max(");
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_ABS:  fprintf(stderr, "abs("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_ALLOCA:  fprintf(stderr, "alloca("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_MEMCPY:
		fprintf(stderr, "memcpy(");
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_MEMSET:
		fprintf(stderr, "memset(");
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_STRLEN:  fprintf(stderr, "strlen("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_CAST:  fprintf(stderr, "cast<"); m_type->Print(); fprintf(stderr, ">("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	case EXPR_RETURN:  fprintf(stderr, "return "); m_children[0]->Print(indent); break;
	case EXPR_RETURN_VOID:  fprintf(stderr, "return void"); break;
	case EXPR_LABEL:  fprintf(stderr, "%s:", m_stringValue.c_str()); break;
	case EXPR_GOTO_LABEL:  fprintf(stderr, "goto %s", m_stringValue.c_str()); break;
	case EXPR_COMPUTED_GOTO:  fprintf(stderr, "goto *"); m_children[0]->Print(indent); break;
	case EXPR_BREAK:  fprintf(stderr, "break"); break;
	case EXPR_CONTINUE:  fprintf(stderr, "continue"); break;
	case EXPR_SWITCH:
		fprintf(stderr, "switch (");
		m_children[0]->Print(indent);
		fprintf(stderr, ")\n");
		PrintIndent(indent + 1);
		m_children[1]->Print(indent + 1);
		break;
	case EXPR_CASE:  fprintf(stderr, "case %lld:", (long long)m_intValue); break;
	case EXPR_DEFAULT:  fprintf(stderr, "default:"); break;
	case EXPR_UNDEFINED:  fprintf(stderr, "__undefined"); break;
	case EXPR_SYSCALL:
		fprintf(stderr, "__syscall(");
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_SYSCALL2:
		fprintf(stderr, "__syscall2(");
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (i > 0)
				fprintf(stderr, ", ");
			m_children[i]->Print(indent);
		}
		fprintf(stderr, ")");
		break;
	case EXPR_RDTSC:  fprintf(stderr, "__rdtsc()"); break;
	case EXPR_RDTSC_LOW:  fprintf(stderr, "__rdtsc_low()"); break;
	case EXPR_RDTSC_HIGH:  fprintf(stderr, "__rdtsc_high()"); break;
	case EXPR_NEXT_ARG:
		fprintf(stderr, "__next_arg(");
		m_children[0]->Print(indent);
		fprintf(stderr, ", ");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_PREV_ARG:
		fprintf(stderr, "__prev_arg(");
		m_children[0]->Print(indent);
		fprintf(stderr, ", ");
		m_children[1]->Print(indent);
		fprintf(stderr, ")");
		break;
	case EXPR_BYTESWAP:  fprintf(stderr, "__byteswap("); m_children[0]->Print(indent); fprintf(stderr, ")"); break;
	default:
		fprintf(stderr, "<invalid_expr>");
		break;
	}
}
#endif

