%{
#include "ParserState.h"
#include "CodeParser.h"

#ifdef WIN32
#define strdup _strdup
#pragma warning(disable:4018)
#define isatty(x) 0
#endif

#define PARSERSTATE ((ParserState*)yyget_extra(yyscanner))
#define YYLVAL ((YYSTYPE*)PARSERSTATE->GetLValue())

#define TOK(t) { return (t); }
#define TOKINT(t, v) { YYLVAL->intval = (v); return (t); }
#define TOKFLOAT(t, v) { YYLVAL->floatval = (v); return (t); }
#define TOKSTR(t, v) { YYLVAL->str = strdup(v); return (t); }

#define TOKQUOTEDSTR(t, v) \
{ \
	char* str = strdup(v + 1); \
	str[strlen(str) - 1] = 0; \
	YYLVAL->str = strdup(ParserState::ProcessEscapedString(str).c_str()); \
	return (t); \
}
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

#\ *line[^\r\n]*	{
				char* line = strchr(yytext, ' ');
				if (line)
				{
					char* name;
					long lineNumber = strtol(line, &name, 10);
					name = strchr(name, '\"');
					if (name)
					{
						name++;
						char* end = strrchr(name, '\"');
						size_t len;
						if (end)
							len = end - name;
						else
							len = strlen(name);
						PARSERSTATE->SetLocation(std::string(name, len), lineNumber);
					}
				}
			}

\n		yylineno++;
\r		yylineno++;
\r\n		yylineno++;
\n\r		yylineno++;

"-"		TOK(MINUS)
"+"		TOK(PLUS)
"*"		TOK(STAR)
"/"		TOK(DIV)
"%"		TOK(MOD)
"&"		TOK(AND)
"|"		TOK(OR)
"^"		TOK(XOR)
"-="		TOK(MINUS_EQ)
"+="		TOK(PLUS_EQ)
"*="		TOK(MULT_EQ)
"/="		TOK(DIV_EQ)
"%="		TOK(MOD_EQ)
"&="		TOK(AND_EQ)
"|="		TOK(OR_EQ)
"^="		TOK(XOR_EQ)
"<<="		TOK(SHIFT_LEFT_EQ)
">>="		TOK(SHIFT_RIGHT_EQ)
"++"		TOK(INCREMENT)
"--"		TOK(DECREMENT)
"{"		TOK(LBRACE)
"}"		TOK(RBRACE)
"("		TOK(LPAREN)
")"		TOK(RPAREN)
"["		TOK(LBRACKET)
"]"		TOK(RBRACKET)
"="		TOK(ASSIGN)
"<"		TOK(LESS_THAN)
"<="		TOK(LESS_EQUAL)
"=="		TOK(EQUAL)
"!="		TOK(NOT_EQUAL)
">="		TOK(GREATER_EQUAL)
">"		TOK(GREATER_THAN)
"~"		TOK(NOT)
"..."		TOK(ELLIPSIS)
"."		TOK(DOT)
"->"		TOK(ARROW)
":"		TOK(COLON)
"?"		TOK(QUESTION)
";"		TOK(SEMICOLON)
","		TOK(COMMA)
"||"		TOK(LOGICAL_OR)
"&&"		TOK(LOGICAL_AND)
"!"		TOK(LOGICAL_NOT)
"<<"		TOK(SHIFT_LEFT)
">>"		TOK(SHIFT_RIGHT)

const		TOK(CONST_TOK)
bool		TOK(BOOL_TOK)
true		TOK(TRUE_VAL)
false		TOK(FALSE_VAL)
char		TOK(CHAR_TOK)
int		TOK(INT_TOK)
short		TOK(SHORT_TOK)
long		TOK(LONG_TOK)
float		TOK(FLOAT_TOK)
double		TOK(DOUBLE_TOK)
signed		TOK(SIGNED_TOK)
unsigned	TOK(UNSIGNED_TOK)
int8_t		TOK(INT8_TOK)
int16_t		TOK(INT16_TOK)
int32_t		TOK(INT32_TOK)
int64_t		TOK(INT64_TOK)
uint8_t		TOK(UINT8_TOK)
uint16_t	TOK(UINT16_TOK)
uint32_t	TOK(UINT32_TOK)
uint64_t	TOK(UINT64_TOK)
size_t		TOK(SIZET)
ssize_t		TOK(SSIZET)
ptrdiff_t	TOK(PTRDIFFT)
struct		TOK(STRUCT)
union		TOK(UNION)
enum		TOK(ENUM)
return		TOK(RETURN)
if		TOK(IF)
else		TOK(ELSE)
for		TOK(FOR)
while		TOK(WHILE)
do		TOK(DO)
break		TOK(BREAK)
continue	TOK(CONTINUE)
goto		TOK(GOTO)
switch		TOK(SWITCH)
case		TOK(CASE)
default		TOK(DEFAULT)
void		TOK(VOID_TOK)
min		TOK(MIN)
max		TOK(MAX)
abs		TOK(ABS)
alloca		TOK(ALLOCA)
memcpy		TOK(MEMCPY)
memset		TOK(MEMSET)
sizeof		TOK(SIZEOF)
typedef		TOK(TYPEDEF)
static		TOK(STATIC_TOK)
extern		TOK(EXTERN_TOK)
__undefined     TOK(UNDEFINED)

__cdecl		TOK(CDECL_TOK)
__stdcall	TOK(STDCALL_TOK)
__fastcall	TOK(FASTCALL_TOK)

__syscall	TOK(SYSCALL_TOK)

__rdtsc		TOK(RDTSC_TOK)
__rdtsc_low	TOK(RDTSC_LOW)
__rdtsc_high	TOK(RDTSC_HIGH)

__next_arg	TOK(NEXT_ARG)
__prev_arg	TOK(PREV_ARG)

__byteswap	TOK(BYTESWAP)

[0-9]+						TOKINT(INT_VAL, strtoull(yytext, NULL, 10))
0x[0-9a-fA-F]+					TOKINT(INT_VAL, strtoull(yytext + 2, NULL, 16))
0b[0-1]+					TOKINT(INT_VAL, strtoull(yytext + 2, NULL, 2))
[0-9]+\.[0-9]+(([eE][-+]?[0-9]+)?)		TOKFLOAT(FLOAT_VAL, (float)atof(yytext))
\.[0-9]+(([eE][-+]?[0-9]+)?)			TOKFLOAT(FLOAT_VAL, (float)atof(yytext))
[0-9]+\.(([eE][-+]?[0-9]+)?)			TOKFLOAT(FLOAT_VAL, (float)atof(yytext))
[0-9]+([eE][-+]?[0-9]+)				TOKFLOAT(FLOAT_VAL, (float)atof(yytext))

\"([^\"\r\n]*(\\\")?)*\"			TOKQUOTEDSTR(STRING_VAL, yytext)
\'([^\'\r\n]*(\\\')?)*\'			TOKQUOTEDSTR(CHAR_VAL, yytext)
[[:alpha:]_][[:alnum:]_]*			{
							if (PARSERSTATE->IsTypeDefined(yytext))
							{
								TOKSTR(TYPE_ID, yytext);
							}
							else
							{
								TOKSTR(ID, yytext);
							}
						}

.		TOK(_ERROR)
<<EOF>>		TOK(END)
%%

