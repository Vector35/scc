#include <stdio.h>
#include "PreprocessState.h"
#include "PreprocessParser.h"
#include "PreprocessLexer.h"

using namespace std;


extern int Preprocess_parse(PreprocessState* state);
extern int Preprocess_get_lineno(void* yyscanner);


PreprocessState::PreprocessState(const string& name, void* scanner): m_fileName(name), m_scanner(scanner)
{
	m_errors = 0;
}


PreprocessState::~PreprocessState()
{
}


int PreprocessState::GetLineNumber()
{
	return Preprocess_get_lineno(GetScanner());
}


Location PreprocessState::GetLocation()
{
	Location loc;
	loc.fileName = GetFileName();
	loc.lineNumber = GetLineNumber();
	return loc;
}


void PreprocessState::AppendLocation()
{
	string result = "#line ";

	char lineStr[32];
	sprintf(lineStr, "%d", GetLineNumber());
	result += lineStr;

	result += " \"";
	result += GetFileName();
	result += "\"\n";

	m_output += result;
}


bool PreprocessState::PreprocessSource(const string& source, const string& fileName, string& output)
{
	yyscan_t scanner;
	Preprocess_lex_init(&scanner);
	PreprocessState parser(fileName.c_str(), scanner);

	YY_BUFFER_STATE buf = Preprocess__scan_string(source.c_str(), scanner);
	Preprocess__switch_to_buffer(buf, scanner);
	Preprocess_set_lineno(1, scanner);

	parser.AppendLocation();

	bool ok = true;
	if (Preprocess_parse(&parser) != 0)
		ok = false;
	if (parser.HasErrors())
		ok = false;

	Preprocess_lex_destroy(scanner);
	output = parser.GetOutput();
	return ok;
}

