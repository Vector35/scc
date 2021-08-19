#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "Output.h"
#include "RefCountObject.h"
#include <string>


enum TokenClass
{
	TOKEN_BASIC,
	TOKEN_ID,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_COMMA,
	TOKEN_PASTE
};

class Token : public RefCountObject
{
	TokenClass m_type;
	std::string m_string;

 public:
	Token(TokenClass type);
	Token(TokenClass type, const std::string& str);
	virtual ~Token();

	TokenClass GetType() const { return m_type; }
	const std::string& GetString() const { return m_string; }

	void Serialize(OutputBlock* output);
	static Token* Deserialize(InputBlock* input);
};


#endif
