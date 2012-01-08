%{
#include "PreprocessState.h"
#include "PreprocessParser.h"

#ifdef WIN32
#define strdup _strdup
#pragma warning(disable:4018)
#define isatty(x) 0
#endif

#define PARSERSTATE ((PreprocessState*)yyget_extra(yyscanner))
#define YYLVAL ((YYSTYPE*)PARSERSTATE->GetLValue())

#define TOK(t) { return (t); }
#define TOKSTR(t, v) { YYLVAL->str = strdup(v); return (t); }
%}

%option noyywrap
%option case-insensitive

%%

[ \t\f]*

"/*"		{
			int c = yyinput(yyscanner);
			while (c != 0)
			{
				if (c == '\n')
					yylineno++;
				if (c == '*')
				{
					if ((c = yyinput(yyscanner)) == '/')
						break;
					continue;
				}
				c = yyinput(yyscanner);
			}
		}
"//".*$

\n		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->AppendLocation(); TOK(NEWLINE); }
\r		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->AppendLocation(); TOK(NEWLINE); }
\r\n		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->AppendLocation(); TOK(NEWLINE); }
\n\r		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->AppendLocation(); TOK(NEWLINE); }

\\\n		yylineno++;
\\\r		yylineno++;
\\\r\n		yylineno++;
\\\n\r		yylineno++;

#\ *include	TOK(INCLUDE_TOK)
#\ *define	TOK(DEFINE_TOK)
#\ *ifdef	TOK(IFDEF_TOK)
#\ *ifndef	TOK(IFNDEF_TOK)
#\ *endif	TOK(ENDIF_TOK)

"-"		TOKSTR(TOKEN, yytext)
"+"		TOKSTR(TOKEN, yytext)
"*"		TOKSTR(TOKEN, yytext)
"/"		TOKSTR(TOKEN, yytext)
"%"		TOKSTR(TOKEN, yytext)
"&"		TOKSTR(TOKEN, yytext)
"|"		TOKSTR(TOKEN, yytext)
"^"		TOKSTR(TOKEN, yytext)
"-="		TOKSTR(TOKEN, yytext)
"+="		TOKSTR(TOKEN, yytext)
"*="		TOKSTR(TOKEN, yytext)
"/="		TOKSTR(TOKEN, yytext)
"%="		TOKSTR(TOKEN, yytext)
"&="		TOKSTR(TOKEN, yytext)
"|="		TOKSTR(TOKEN, yytext)
"^="		TOKSTR(TOKEN, yytext)
"<<="		TOKSTR(TOKEN, yytext)
">>="		TOKSTR(TOKEN, yytext)
"++"		TOKSTR(TOKEN, yytext)
"--"		TOKSTR(TOKEN, yytext)
"{"		TOKSTR(TOKEN, yytext)
"}"		TOKSTR(TOKEN, yytext)
"("		TOK(LPAREN)
")"		TOK(RPAREN)
"["		TOKSTR(TOKEN, yytext)
"]"		TOKSTR(TOKEN, yytext)
"="		TOKSTR(TOKEN, yytext)
"<"		TOK(LESS_TOK)
"<="		TOKSTR(TOKEN, yytext)
"=="		TOKSTR(TOKEN, yytext)
"!="		TOKSTR(TOKEN, yytext)
">="		TOKSTR(TOKEN, yytext)
">"		TOK(GREATER_TOK)
"~"		TOKSTR(TOKEN, yytext)
"."		TOKSTR(TOKEN, yytext)
"->"		TOKSTR(TOKEN, yytext)
":"		TOKSTR(TOKEN, yytext)
"?"		TOKSTR(TOKEN, yytext)
";"		TOKSTR(TOKEN, yytext)
","		TOK(COMMA)
"||"		TOKSTR(TOKEN, yytext)
"&&"		TOKSTR(TOKEN, yytext)
"!"		TOKSTR(TOKEN, yytext)
"<<"		TOKSTR(TOKEN, yytext)
">>"		TOKSTR(TOKEN, yytext)

"##"		TOK(PASTE)

[0-9]+						TOKSTR(TOKEN, yytext)
0x[0-9a-fA-F]+					TOKSTR(TOKEN, yytext)
0b[0-1]+					TOKSTR(TOKEN, yytext)
[0-9]+\.[0-9]+(([eE][-+]?[0-9]+)?)		TOKSTR(TOKEN, yytext)
\.[0-9]+(([eE][-+]?[0-9]+)?)			TOKSTR(TOKEN, yytext)
[0-9]+\.(([eE][-+]?[0-9]+)?)			TOKSTR(TOKEN, yytext)
[0-9]+([eE][-+]?[0-9]+)				TOKSTR(TOKEN, yytext)

\"([^\"\r\n]*(\\\")?)*\"			TOKSTR(TOKEN, yytext)
\'([^\"\r\n]*(\\\")?)*\'			TOKSTR(TOKEN, yytext)
[[:alpha:]_][[:alnum:]_]*			TOKSTR(ID, yytext)

.		TOK(_ERROR)
<<EOF>>		TOK(END)
%%

