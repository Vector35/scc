#include "Token.h"
#include "Struct.h"

using namespace std;


Token::Token(TokenClass type) : m_type(type) {}


Token::Token(TokenClass type, const string& str) : m_type(type), m_string(str) {}


Token::~Token() {}


void Token::Serialize(OutputBlock* output)
{
	output->WriteInteger(m_type);
	output->WriteString(m_string);
}


Token* Token::Deserialize(InputBlock* input)
{
	uint32_t type;
	string str;

	if (!input->ReadUInt32(type))
		return NULL;
	if (!input->ReadString(str))
		return NULL;

	return new Token((TokenClass)type, str);
}
