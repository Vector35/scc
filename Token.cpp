#include "Token.h"

using namespace std;


Token::Token(TokenType type): m_type(type)
{
}


Token::Token(TokenType type, const string& str): m_type(type), m_string(str)
{
}


Token::~Token()
{
}

