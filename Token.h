#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <string>
#include "RefCountObject.h"


enum TokenType
{
	TOKEN_BASIC,
	TOKEN_ID,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_COMMA
};

class Token: public RefCountObject
{
	TokenType m_type;
	std::string m_string;

public:
	Token(TokenType type);
	Token(TokenType type, const std::string& str);
	virtual ~Token();

	TokenType GetType() const { return m_type; }
	const std::string& GetString() const { return m_string; }
};


#endif

