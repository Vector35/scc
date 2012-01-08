%{
#include <stdio.h>
#include "PreprocessState.h"
%}

%union
{
	char* str;
	Token* token;
	std::vector< Ref<Token> >* tokens;
	std::vector<std::string>* params;
}

%{
using namespace std;

extern int Preprocess_lex(void* yyscanner);
extern void Preprocess_set_extra(void* user_defined, void* yyscanner);
extern int Preprocess_get_lineno(void* yyscanner);

static int Preprocess_lex_wrapper(void* yyscanner)
{
	return Preprocess_lex(yyscanner);
}

#define Preprocess_lex Preprocess_lex_final

static int Preprocess_lex_final(YYSTYPE* lval, PreprocessState* state)
{
	state->SetLValue(lval);
	Preprocess_set_extra(state, state->GetScanner());
	return Preprocess_lex_wrapper(state->GetScanner());
}

void Preprocess_error(PreprocessState* state, const char* msg)
{
	state->Error();
	fprintf(stderr, "%s:%d: error: %s\n", state->GetFileName().c_str(), Preprocess_get_lineno(state->GetScanner()), msg);
}
%}

%pure-parser
%parse-param {PreprocessState* state}
%lex-param {PreprocessState* state}

%token END 0
%token _ERROR

%token LPAREN RPAREN COMMA
%token NEWLINE PASTE

%token DEFINE_TOK UNDEF_TOK IFDEF_TOK IFNDEF_TOK ELSE_TOK ENDIF_TOK

%token <str> ID TOKEN MACRO

%destructor { free($$); } ID TOKEN MACRO id_or_macro
%destructor { delete $$; } token_list
%destructor { delete $$; } param_list
%destructor { $$->Release(); } token non_lparen

%type <tokens> token_list
%type <token> token non_lparen
%type <params> param_list
%type <str> id_or_macro

%%

input:	toplevel_stmt_list END  { state->Finalize(); }
|
;

toplevel_stmt_list:	toplevel_stmt_list toplevel_stmt
		|	toplevel_stmt
		;

toplevel_stmt:	output_token_list NEWLINE
	|	NEWLINE
	|	IFDEF_TOK id_or_macro NEWLINE  { state->BeginIf(state->IsDefined($2)); free($2); }
	|	IFNDEF_TOK id_or_macro NEWLINE  { state->BeginIf(!state->IsDefined($2)); free($2); }
	|	ELSE_TOK NEWLINE  { state->Else(); }
	|	ENDIF_TOK NEWLINE  { state->EndIf(); }
	|	DEFINE_TOK id_or_macro non_lparen NEWLINE
		{
			std::vector< Ref<Token> > tokens;
			tokens.push_back($3);
			state->Define($2, std::vector<std::string>(), tokens);
			free($2);
			$3->Release();
		}
	|	DEFINE_TOK id_or_macro non_lparen token_list NEWLINE
		{
			std::vector< Ref<Token> > tokens;
			tokens.push_back($3);
			tokens.insert(tokens.end(), $4->begin(), $4->end());
			state->Define($2, std::vector<std::string>(), tokens);
			free($2);
			delete $3;
		}
	|	DEFINE_TOK id_or_macro NEWLINE
		{
			state->Define($2, std::vector<std::string>(), std::vector< Ref<Token> >());
			free($2);
		}
	|	DEFINE_TOK id_or_macro LPAREN RPAREN token_list NEWLINE
		{
			state->Define($2, std::vector<std::string>(), *$5);
			free($2);
			delete $5;
		}
	|	DEFINE_TOK id_or_macro LPAREN RPAREN NEWLINE
		{
			state->Define($2, std::vector<std::string>(), std::vector< Ref<Token> >());
			free($2);
		}
	|	DEFINE_TOK id_or_macro LPAREN param_list RPAREN token_list NEWLINE
		{
			state->Define($2, *$4, *$6);
			free($2);
			delete $4;
			delete $6;
		}
	|	DEFINE_TOK id_or_macro LPAREN param_list RPAREN NEWLINE
		{
			state->Define($2, *$4, std::vector< Ref<Token> >());
			free($2);
			delete $4;
		}
	|	UNDEF_TOK id_or_macro NEWLINE  { state->Undefine($2); free($2); }
	;

id_or_macro:	ID  { $$ = $1; }
	|	MACRO  { $$ = $1; }
	;

token_list:	token_list token  { $$ = $1; $$->push_back($2); $2->Release(); }
	|	token  { $$ = new std::vector< Ref<Token> >(); $$->push_back($1); $1->Release(); }
	;

token:	non_lparen  { $$ = $1; }
|	LPAREN  { $$ = new Token(TOKEN_LPAREN); $$->AddRef(); }
;

non_lparen:	TOKEN  { $$ = new Token(TOKEN_BASIC, $1); $$->AddRef(); free($1); }
	|	ID  { $$ = new Token(TOKEN_ID, $1); $$->AddRef(); free($1); }
	|	MACRO  { $$ = new Token(TOKEN_ID, $1); $$->AddRef(); free($1); }
	|	RPAREN  { $$ = new Token(TOKEN_RPAREN); $$->AddRef(); }
	|	COMMA  { $$ = new Token(TOKEN_COMMA); $$->AddRef(); }
	;

param_list:	param_list COMMA ID  { $$ = $1; $$->push_back($3); free($3); }
	|	ID  { $$ = new std::vector<std::string>(); $$->push_back($1); free($1); }
	;

output_token_list:	output_token_list output_token
		|	output_token
		;

output_token:	TOKEN  { Ref<Token> token = new Token(TOKEN_BASIC, $1); state->Append(token); free($1); }
	|	ID  { Ref<Token> token = new Token(TOKEN_ID, $1); state->Append(token); free($1); }
	|	LPAREN  { Ref<Token> token = new Token(TOKEN_LPAREN); state->Append(token); }
	|	RPAREN  { Ref<Token> token = new Token(TOKEN_RPAREN); state->Append(token); }
	|	COMMA  { Ref<Token> token = new Token(TOKEN_COMMA); state->Append(token); }
	|	MACRO  { state->BeginMacroExpansion($1); free($1); }
	;

%%

