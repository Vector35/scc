%{
#include <stdio.h>
#include "PreprocessState.h"
%}

%union
{
	char* str;
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
%token LESS_TOK GREATER_TOK
%token NEWLINE PASTE

%token INCLUDE_TOK
%token DEFINE_TOK IFDEF_TOK IFNDEF_TOK ENDIF_TOK

%token <str> ID TOKEN

%destructor { free($$); } ID TOKEN

%%

input:	toplevel_stmt_list END
|
;

toplevel_stmt_list:	toplevel_stmt_list toplevel_stmt
		|	toplevel_stmt
		;

toplevel_stmt:	literal_token_list NEWLINE
	|	NEWLINE
	|	INCLUDE_TOK TOKEN NEWLINE
	|	IFDEF_TOK ID NEWLINE
	|	IFNDEF_TOK ID NEWLINE
	|	ENDIF_TOK NEWLINE
	|	DEFINE_TOK ID token_list NEWLINE
	|	DEFINE_TOK ID NEWLINE
	|	DEFINE_TOK LPAREN RPAREN token_list NEWLINE
	|	DEFINE_TOK LPAREN RPAREN NEWLINE
	|	DEFINE_TOK LPAREN param_list RPAREN token_list NEWLINE
	|	DEFINE_TOK LPAREN param_list RPAREN NEWLINE
	;

token_list:	token_list token
	|	token
	;

token:	TOKEN
|	ID
|	LPAREN
|	RPAREN
|	LESS_TOK
|	GREATER_TOK
|	COMMA
;

literal_token_list:	literal_token_list literal_token
		|	literal_token
		;

literal_token:	TOKEN  { state->Append($1); state->Append(" "); free($1); }
	|	ID  { state->Append($1); state->Append(" "); free($1); }
	|	LPAREN  { state->Append("("); }
	|	RPAREN  { state->Append(")"); }
	|	LESS_TOK  { state->Append("< "); }
	|	GREATER_TOK  { state->Append("> "); }
	|	COMMA  { state->Append(","); }
	;

param_list:	param_list COMMA ID
	|	ID
	;

%%

