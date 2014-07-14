%{
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

#ifdef WIN32
#define YY_NO_UNISTD_H
#endif
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
#define TOKSTR_NODUP(t, v) { YYLVAL->str = (v); return (t); }
%}

%option noyywrap

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

\n		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->RequestLocation(); TOK(NEWLINE); }
\r		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->RequestLocation(); TOK(NEWLINE); }
\r\n		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->RequestLocation(); TOK(NEWLINE); }
\n\r		{ yylineno++; PARSERSTATE->Append("\n"); PARSERSTATE->RequestLocation(); TOK(NEWLINE); }

\\\n		yylineno++;
\\\r		yylineno++;
\\\r\n		yylineno++;
\\\n\r		yylineno++;

#[\ \t]*include[\ \t]*<[^<>\r\n]*>	{
						char* name = strchr(yytext, '<') + 1;
						PARSERSTATE->IncludeFile(std::string(name, strlen(name) - 1));
					}
#[\ \t]*include[\ \t]*\"[^\"\r\n]*\"	{
						char* name = strchr(yytext, '\"') + 1;
						PARSERSTATE->IncludeFile(std::string(name, strlen(name) - 1));
					}

#[ \t]*define[\ \t]*[[:alpha:]_][[:alnum:]_]*	{
							char* name = strstr(yytext, "define") + strlen("define");
							while ((name[0] == ' ') || (name[0] == '\t'))
								name++;
							TOKSTR(DEFINE_NO_PARAMS, name)
						}

#[ \t]*define[\ \t]*[[:alpha:]_][[:alnum:]_]*\(	{
							char* name = strstr(yytext, "define") + strlen("define");
							while ((name[0] == ' ') || (name[0] == '\t'))
								name++;
							char* trimmed = strdup(name);
							trimmed[strlen(trimmed) - 1] = 0;
							TOKSTR_NODUP(DEFINE_PARAMS, trimmed)
						}

#[ \t]*undef	TOK(UNDEF_TOK)
#[ \t]*ifdef	TOK(IFDEF_TOK)
#[ \t]*ifndef	TOK(IFNDEF_TOK)
#[ \t]*else	TOK(ELSE_TOK)
#[ \t]*endif	TOK(ENDIF_TOK)

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
"<"		TOKSTR(TOKEN, yytext)
"<="		TOKSTR(TOKEN, yytext)
"=="		TOKSTR(TOKEN, yytext)
"!="		TOKSTR(TOKEN, yytext)
">="		TOKSTR(TOKEN, yytext)
">"		TOKSTR(TOKEN, yytext)
"~"		TOKSTR(TOKEN, yytext)
"..."		TOKSTR(TOKEN, yytext)
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
[0-9]+LL						TOKSTR(TOKEN, yytext)
0x[0-9a-fA-F]+					TOKSTR(TOKEN, yytext)
0x[0-9a-fA-F]+LL					TOKSTR(TOKEN, yytext)
0b[0-1]+					TOKSTR(TOKEN, yytext)
0b[0-1]+LL					TOKSTR(TOKEN, yytext)
[0-9]+\.[0-9]+(([eE][-+]?[0-9]+)?)f		TOKSTR(TOKEN, yytext)
\.[0-9]+(([eE][-+]?[0-9]+)?)f			TOKSTR(TOKEN, yytext)
[0-9]+\.(([eE][-+]?[0-9]+)?)f			TOKSTR(TOKEN, yytext)
[0-9]+([eE][-+]?[0-9]+)f				TOKSTR(TOKEN, yytext)
[0-9]+\.[0-9]+(([eE][-+]?[0-9]+)?)		TOKSTR(TOKEN, yytext)
\.[0-9]+(([eE][-+]?[0-9]+)?)			TOKSTR(TOKEN, yytext)
[0-9]+\.(([eE][-+]?[0-9]+)?)			TOKSTR(TOKEN, yytext)
[0-9]+([eE][-+]?[0-9]+)				TOKSTR(TOKEN, yytext)

\"([^\"\r\n]*(\\\")?)*\"			TOKSTR(TOKEN, yytext)
\'([^\'\r\n]*(\\\')?)*\'			TOKSTR(TOKEN, yytext)
[[:alpha:]_][[:alnum:]_]*	{
					if (PARSERSTATE->IsDefined(yytext))
						TOKSTR(MACRO, yytext)
					else
						TOKSTR(ID, yytext);
				}

.		TOK(_ERROR)
<<EOF>>		TOK(END)
%%

