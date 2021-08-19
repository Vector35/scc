#ifdef WIN32
	#define YY_NO_UNISTD_H
#endif
#include "PreprocessState.h"
#include "PreprocessLexer.h"
#include "PreprocessParser.h"
#include <stdio.h>

using namespace std;


extern int Preprocess_parse(PreprocessState* state);
extern int Preprocess_get_lineno(void* yyscanner);


PreprocessState::PreprocessState(const string& name, void* scanner, const Settings& settings) :
    m_fileName(name), m_scanner(scanner), m_settings(settings)
{
	m_errors = 0;
	m_expansionInProgress = false;
	m_ifFailCount = 0;
	m_startingIfStackSize = 0;
	m_locationRequest = true;
	m_paste = false;
}


PreprocessState::PreprocessState(PreprocessState& parent, const string& name, void* scanner) :
    m_fileName(name), m_scanner(scanner), m_settings(parent.m_settings)
{
	m_errors = 0;
	m_expansionInProgress = false;
	m_ifFailCount = 0;
	m_startingIfStackSize = 0;
	m_locationRequest = true;
	m_paste = false;

	m_macros = parent.m_macros;
}


PreprocessState::~PreprocessState() {}


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


void PreprocessState::Append(const string& str)
{
	if (m_locationRequest)
	{
		if (str == "\n")
			return;
		AppendLocation();
	}

	m_output += str;
}


void PreprocessState::Append(Token* token)
{
	// Ignore anything inside a failed #ifdef
	if (m_ifFailCount > 0)
		return;

	// Process macro parameters
	if (m_expansionInProgress)
	{
		if (token->GetType() == TOKEN_LPAREN)
		{
			m_expansion.parens++;
			if (m_expansion.parens == 1)
				return;
			m_expansion.curParam.push_back(token);
			return;
		}
		else if (token->GetType() == TOKEN_RPAREN)
		{
			if (m_expansion.parens == 0)
				FinishMacroExpansion();
			else
			{
				if (m_expansion.parens == 1)
					FinishMacroExpansion();
				else
				{
					m_expansion.parens--;
					m_expansion.curParam.push_back(token);
				}
				return;
			}
		}
		else if (token->GetType() == TOKEN_COMMA)
		{
			if (m_expansion.parens == 0)
				FinishMacroExpansion();
			else if (m_expansion.parens == 1)
			{
				m_expansion.params.push_back(m_expansion.curParam);
				m_expansion.curParam.clear();
				return;
			}
			else
			{
				m_expansion.curParam.push_back(token);
				return;
			}
		}
		else
		{
			if (m_expansion.parens == 0)
				FinishMacroExpansion();
			else
			{
				m_expansion.curParam.push_back(token);
				return;
			}
		}
	}

	if (m_locationRequest)
		AppendLocation();

	// Output the token text
	switch (token->GetType())
	{
	case TOKEN_LPAREN:
		m_output += "(";
		m_paste = false;
		break;
	case TOKEN_RPAREN:
		m_output += ")";
		m_paste = false;
		break;
	case TOKEN_COMMA:
		m_output += ",";
		m_paste = false;
		break;
	case TOKEN_PASTE:
		m_paste = true;
		break;
	default:
		if (!m_paste)
			m_output += " ";
		m_output += token->GetString();
		m_paste = false;
		break;
	}
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
	m_locationRequest = false;
}


void PreprocessState::IncludeSource(const string& source)
{
	// Ignore anything inside a failed #ifdef
	if (m_ifFailCount > 0)
		return;

	string origFileName = m_fileName;
	void* origScanner = m_scanner;
	size_t origStartingIfStackSize = m_startingIfStackSize;
	m_fileName = "";
	m_startingIfStackSize = m_ifStack.size();

	yyscan_t scanner;
	Preprocess_lex_init(&scanner);
	m_scanner = scanner;

	YY_BUFFER_STATE buf = Preprocess__scan_string(source.c_str(), scanner);
	Preprocess__switch_to_buffer(buf, scanner);
	Preprocess_set_lineno(1, scanner);

	AppendLocation();

	Preprocess_parse(this);
	Preprocess_lex_destroy(scanner);

	m_fileName = origFileName;
	m_scanner = origScanner;
	m_startingIfStackSize = origStartingIfStackSize;
}


void PreprocessState::IncludeFile(const string& name)
{
	// Ignore anything inside a failed #ifdef
	if (m_ifFailCount > 0)
		return;

	FILE* fp = fopen(name.c_str(), "rb");
	if (!fp)
	{
		// Look for include file in alternative directories
		for (vector<string>::const_iterator i = m_settings.includeDirs.begin();
		     i != m_settings.includeDirs.end(); i++)
		{
			string path = *i;
			if ((path.size() > 0) && (path[path.size() - 1] != '/') && (path[path.size() - 1] != '\\'))
			{
#ifdef WIN32
				path += "\\";
#else
				path += "/";
#endif
			}
			path += name;

			fp = fopen(path.c_str(), "rb");
			if (fp)
				break;
		}

		if (!fp)
		{
			if (m_scanner)
			{
				fprintf(stderr, "%s:%d: error: include file '%s' not found\n", GetFileName().c_str(),
				    GetLineNumber(), name.c_str());
			}
			else
			{
				fprintf(stderr, "%s: error: include file '%s' not found\n", GetFileName().c_str(),
				    name.c_str());
			}
			m_errors++;
			return;
		}
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* data = new char[size + 2];
	if (fread(data, 1, size, fp) != (size_t)size)
	{
		fprintf(stderr, "%s: error: include file '%s' could not be read\n", GetFileName().c_str(),
		    name.c_str());
		m_errors++;
		delete[] data;
		fclose(fp);
		return;
	}
	data[size++] = '\n';
	data[size] = 0;
	fclose(fp);

	string origFileName = m_fileName;
	void* origScanner = m_scanner;
	size_t origStartingIfStackSize = m_startingIfStackSize;
	m_fileName = name;
	m_startingIfStackSize = m_ifStack.size();

	yyscan_t scanner;
	Preprocess_lex_init(&scanner);
	m_scanner = scanner;

	YY_BUFFER_STATE buf = Preprocess__scan_string(data, scanner);
	Preprocess__switch_to_buffer(buf, scanner);
	Preprocess_set_lineno(1, scanner);

	AppendLocation();

	Preprocess_parse(this);
	Preprocess_lex_destroy(scanner);
	delete[] data;

	m_fileName = origFileName;
	m_scanner = origScanner;
	m_startingIfStackSize = origStartingIfStackSize;
}


void PreprocessState::Define(const string& name, const vector<string>& params,
    const vector<Ref<Token>>& tokens, bool hasParams)
{
	// Ignore anything inside a failed #ifdef
	if (m_ifFailCount > 0)
		return;

	if (m_macros.find(name) != m_macros.end())
	{
		Macro oldDef = m_macros[name];
		fprintf(stderr, "%s:%d: error: '%s' redefined\n", GetFileName().c_str(), GetLineNumber(),
		    name.c_str());
		fprintf(stderr, "%s:%d: error: previous declaration of '%s'\n",
		    oldDef.location.fileName.c_str(), oldDef.location.lineNumber, name.c_str());
		m_errors++;
	}

	Macro macro;
	macro.name = name;
	macro.params = params;
	macro.hasParams = hasParams;
	macro.tokens = tokens;
	macro.location.fileName = GetFileName();
	macro.location.lineNumber = GetLineNumber();
	m_macros[name] = macro;
}


void PreprocessState::Undefine(const string& name)
{
	// Ignore anything inside a failed #ifdef
	if (m_ifFailCount > 0)
		return;

	if (m_macros.find(name) == m_macros.end())
	{
		fprintf(stderr, "%s:%d: error: '%s' is not defined\n", GetFileName().c_str(), GetLineNumber(),
		    name.c_str());
		m_errors++;
	}
	else
	{
		m_macros.erase(name);
	}
}


bool PreprocessState::IsDefined(const string& name)
{
	return m_macros.find(name) != m_macros.end();
}


void PreprocessState::BeginMacroExpansion(const string& name)
{
	// Ignore anything inside a failed #ifdef
	if (m_ifFailCount > 0)
		return;

	if (m_expansionInProgress)
	{
		if (m_expansion.parens > 0)
			m_expansionStack.push(m_expansion);
		else
			FinishMacroExpansion();
	}

	m_expansionInProgress = true;
	m_expansion.macro = m_macros[name];
	m_expansion.parens = 0;
	m_expansion.params.clear();
	m_expansion.curParam.clear();

	if (!m_expansion.macro.hasParams)
		FinishMacroExpansion();
}


void PreprocessState::FinishMacroExpansion()
{
	// Return to outer scope
	MacroExpansion expansion = m_expansion;
	if (m_expansionStack.size() == 0)
		m_expansionInProgress = false;
	else
	{
		m_expansion = m_expansionStack.top();
		m_expansionStack.pop();
	}

	// If there is another parameter queued, add it to the list
	if (expansion.curParam.size() != 0)
		expansion.params.push_back(expansion.curParam);

	// Check argument count against macro
	if (expansion.params.size() != expansion.macro.params.size())
	{
		fprintf(stderr, "%s:%d: error: expected %d argument%s to '%s'\n", GetFileName().c_str(),
		    GetLineNumber(), (int)expansion.macro.params.size(),
		    (expansion.macro.params.size() == 1) ? "" : "s", expansion.macro.name.c_str());
		m_errors++;
		return;
	}

	// Organize parameters by name
	map<string, vector<Ref<Token>>> paramsByName;
	for (size_t i = 0; i < expansion.params.size(); i++)
		paramsByName[expansion.macro.params[i]] = expansion.params[i];

	// Paste macro tokens into output stream
	for (vector<Ref<Token>>::iterator i = expansion.macro.tokens.begin();
	     i != expansion.macro.tokens.end(); i++)
	{
		if ((*i)->GetType() != TOKEN_ID)
			Append(*i);
		else
		{
			map<string, vector<Ref<Token>>>::iterator j = paramsByName.find((*i)->GetString());
			if (j == paramsByName.end())
			{
				if (((*i)->GetType() == TOKEN_ID) && IsDefined((*i)->GetString()))
					BeginMacroExpansion((*i)->GetString());
				else
					Append(*i);
			}
			else
			{
				for (vector<Ref<Token>>::iterator k = j->second.begin(); k != j->second.end(); k++)
				{
					if (((*k)->GetType() == TOKEN_ID) && IsDefined((*k)->GetString()))
						BeginMacroExpansion((*k)->GetString());
					else
						Append(*k);
				}
			}
		}
	}
}


void PreprocessState::Finalize()
{
	if (m_expansionInProgress)
	{
		if ((m_expansion.parens > 0) || (m_expansionStack.size() > 0))
		{
			fprintf(stderr, "%s:%d: error: unterminated macro expansion\n", GetFileName().c_str(),
			    GetLineNumber());
			m_errors++;
		}
		else
		{
			FinishMacroExpansion();
		}
	}

	if (m_ifStack.size() != m_startingIfStackSize)
	{
		fprintf(stderr, "%s:%d: error: expected #endif\n", GetFileName().c_str(), GetLineNumber());
		m_errors++;
	}
}


void PreprocessState::BeginIf(bool result)
{
	m_ifStack.push(result);
	if (!result)
		m_ifFailCount++;
}


void PreprocessState::Else()
{
	if (m_ifStack.size() == 0)
	{
		fprintf(stderr, "%s:%d: error: #else outside of conditional\n", GetFileName().c_str(),
		    GetLineNumber());
		m_errors++;
		return;
	}

	bool old = m_ifStack.top();
	EndIf();
	BeginIf(!old);
}


void PreprocessState::EndIf()
{
	if (m_ifStack.size() == 0)
	{
		fprintf(stderr, "%s:%d: error: #endif outside of conditional\n", GetFileName().c_str(),
		    GetLineNumber());
		m_errors++;
		return;
	}

	if (!m_ifStack.top())
		m_ifFailCount--;
	m_ifStack.pop();
}


void PreprocessState::Serialize(OutputBlock* output)
{
	output->WriteInteger(m_macros.size());

	for (map<string, Macro>::iterator i = m_macros.begin(); i != m_macros.end(); i++)
	{
		output->WriteString(i->second.name);

		output->WriteInteger(i->second.hasParams ? 1 : 0);
		output->WriteInteger(i->second.params.size());
		for (vector<string>::iterator j = i->second.params.begin(); j != i->second.params.end(); j++)
			output->WriteString(*j);

		output->WriteInteger(i->second.tokens.size());
		for (vector<Ref<Token>>::iterator j = i->second.tokens.begin(); j != i->second.tokens.end();
		     j++)
			(*j)->Serialize(output);

		output->WriteString(i->second.location.fileName);
		output->WriteInteger(i->second.location.lineNumber);
	}
}


bool PreprocessState::Deserialize(InputBlock* input)
{
	size_t macroCount;
	if (!input->ReadNativeInteger(macroCount))
		return false;

	m_macros.clear();
	for (size_t i = 0; i < macroCount; i++)
	{
		Macro macro;
		if (!input->ReadString(macro.name))
			return false;

		size_t paramCount;
		if (!input->ReadBool(macro.hasParams))
			return false;
		if (!input->ReadNativeInteger(paramCount))
			return false;
		for (size_t j = 0; j < paramCount; j++)
		{
			string param;
			if (!input->ReadString(param))
				return false;
			macro.params.push_back(param);
		}

		size_t tokenCount;
		if (!input->ReadNativeInteger(tokenCount))
			return false;
		for (size_t j = 0; j < tokenCount; j++)
		{
			Token* token = Token::Deserialize(input);
			if (!token)
				return false;
			macro.tokens.push_back(token);
		}

		if (!input->ReadString(macro.location.fileName))
			return false;
		if (!input->ReadInt32(macro.location.lineNumber))
			return false;

		m_macros[macro.name] = macro;
	}

	return true;
}


bool PreprocessState::PreprocessSource(const Settings& settings, const string& source,
    const string& fileName, string& output, PreprocessState* parent)
{
	yyscan_t scanner;
	Preprocess_lex_init(&scanner);
	PreprocessState* parser;
	if (parent)
		parser = new PreprocessState(*parent, fileName.c_str(), scanner);
	else
		parser = new PreprocessState(fileName.c_str(), scanner, settings);

	YY_BUFFER_STATE buf = Preprocess__scan_string(source.c_str(), scanner);
	Preprocess__switch_to_buffer(buf, scanner);
	Preprocess_set_lineno(1, scanner);

	parser->AppendLocation();

	bool ok = true;
	if (Preprocess_parse(parser) != 0)
		ok = false;
	if (parser->HasErrors())
		ok = false;

	Preprocess_lex_destroy(scanner);
	output = parser->GetOutput();
	delete parser;
	return ok;
}
