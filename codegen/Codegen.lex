%{
// Copyright (c) 2011-2014 Rusty Wagner
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
#include "ParserState.h"
#include "CodegenParser.h"

#ifdef WIN32
#define strdup _strdup
#define strtoull _strtoi64
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
	YYLVAL->str = str; \
	return (t); \
}
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

"-"		TOKSTR(OPERATOR, yytext)
"+"		TOKSTR(OPERATOR, yytext)
"*"		TOKSTR(OPERATOR, yytext)
"/"		TOKSTR(OPERATOR, yytext)
"%"		TOKSTR(OPERATOR, yytext)
"&"		TOKSTR(OPERATOR, yytext)
"|"		TOKSTR(OPERATOR, yytext)
"^"		TOKSTR(OPERATOR, yytext)
"-="		TOKSTR(OPERATOR, yytext)
"+="		TOKSTR(OPERATOR, yytext)
"*="		TOKSTR(OPERATOR, yytext)
"/="		TOKSTR(OPERATOR, yytext)
"%="		TOKSTR(OPERATOR, yytext)
"&="		TOKSTR(OPERATOR, yytext)
"|="		TOKSTR(OPERATOR, yytext)
"^="		TOKSTR(OPERATOR, yytext)
"<<="		TOKSTR(OPERATOR, yytext)
">>="		TOKSTR(OPERATOR, yytext)
"++"		TOKSTR(OPERATOR, yytext)
"--"		TOKSTR(OPERATOR, yytext)
"<"		TOKSTR(OPERATOR, yytext)
"<="		TOKSTR(OPERATOR, yytext)
"=="		TOKSTR(OPERATOR, yytext)
"!="		TOKSTR(OPERATOR, yytext)
">="		TOKSTR(OPERATOR, yytext)
">"		TOKSTR(OPERATOR, yytext)
"~"		TOKSTR(OPERATOR, yytext)
"..."		TOKSTR(OPERATOR, yytext)
"."		TOKSTR(OPERATOR, yytext)
"->"		TOKSTR(OPERATOR, yytext)
"?"		TOKSTR(OPERATOR, yytext)
"||"		TOKSTR(OPERATOR, yytext)
"&&"		TOKSTR(OPERATOR, yytext)
"!"		TOKSTR(OPERATOR, yytext)
"<<"		TOKSTR(OPERATOR, yytext)
">>"		TOKSTR(OPERATOR, yytext)
"::"		TOKSTR(OPERATOR, yytext)

","		TOK(COMMA)
":"		TOK(COLON)
";"		TOK(SEMICOLON)
"("		TOK(LPAREN)
")"		TOK(RPAREN)
"{"		TOK(LBRACE)
"}"		TOK(RBRACE)
"["		TOK(LBRACKET)
"]"		TOK(RBRACKET)
"=>"		TOK(ARROW)
"="		TOK(EQUAL_TOK)

arch				TOK(ARCH_TOK)
registerclass		TOK(REGISTERCLASS)
largeregisterclass	TOK(LARGEREGISTERCLASS)
tempregisterclass	TOK(TEMPREGISTERCLASS)
immediateclass		TOK(IMMEDIATECLASS)
function			TOK(FUNCTION)
var					TOK(VAR_TOK)
default				TOK(DEFAULT_TOK)
include				TOK(INCLUDE_TOK)
instr				TOK(INSTR_TOK)
encoding			TOK(ENCODING_TOK)
update				TOK(UPDATE_TOK)
fixed				TOK(FIXED_TOK)
callersaved			TOK(CALLERSAVED_TOK)
calleesaved			TOK(CALLEESAVED_TOK)
special				TOK(SPECIAL_TOK)

low		TOK(LOW_TOK)
high	TOK(HIGH_TOK)
base	TOK(BASE_TOK)
offset	TOK(OFFSET_TOK)
temp	TOK(TEMP_TOK)
block	TOK(BLOCK_TOK)

memory	TOK(MEMORY_TOK)
branch	TOK(BRANCH_TOK)
readflags TOK(READFLAGS_TOK)
writeflags TOK(WRITEFLAGS_TOK)
copy	TOK(COPY_TOK)
stack	TOK(STACK_TOK)

U8		TOK(UNSIGNED8)
S8		TOK(SIGNED8)
U16		TOK(UNSIGNED16)
S16		TOK(SIGNED16)
U32		TOK(UNSIGNED32)
S32		TOK(SIGNED32)
U64		TOK(UNSIGNED64)
S64		TOK(SIGNED64)
U128	TOK(UNSIGNED128)
S128	TOK(SIGNED128)
F32		TOK(FLOAT32)
F64		TOK(FLOAT64)

assign	TOK(ASSIGN_TOK)
load	TOK(LOAD_TOK)
store	TOK(STORE_TOK)
ref		TOK(REF_TOK)
add		TOK(ADD_TOK)
sub		TOK(SUB_TOK)
smul	TOK(SMUL_TOK)
umul	TOK(UMUL_TOK)
sdiv	TOK(SDIV_TOK)
udiv	TOK(UDIV_TOK)
smod	TOK(SMOD_TOK)
umod	TOK(UMOD_TOK)
and		TOK(AND_TOK)
or		TOK(OR_TOK)
xor		TOK(XOR_TOK)
shl		TOK(SHL_TOK)
shr		TOK(SHR_TOK)
sar		TOK(SAR_TOK)
neg		TOK(NEG_TOK)
not		TOK(NOT_TOK)
iftrue	TOK(IFTRUE_TOK)
ifslt	TOK(IFSLT_TOK)
ifult	TOK(IFULT_TOK)
ifsle	TOK(IFSLE_TOK)
ifule	TOK(IFULE_TOK)
ife		TOK(IFE_TOK)
goto	TOK(GOTO_TOK)
call	TOK(CALL_TOK)
callvoid	TOK(CALLVOID_TOK)
syscall	TOK(SYSCALL_TOK)
syscallvoid	TOK(SYSCALLVOID_TOK)
sconvert	TOK(SCONVERT_TOK)
uconvert	TOK(UCONVERT_TOK)
return	TOK(RETURN_TOK)
returnvoid	TOK(RETURNVOID_TOK)
alloca	TOK(ALLOCA_TOK)
memcpy	TOK(MEMCPY_TOK)
memset	TOK(MEMSET_TOK)
strlen	TOK(STRLEN_TOK)
rdtsc	TOK(RDTSC_TOK)
rdtsc_low	TOK(RDTSC_LOW_TOK)
rdtsc_high	TOK(RDTSC_HIGH_TOK)
peb		TOK(PEB_TOK)
teb		TOK(TEB_TOK)
vararg	TOK(VARARG_TOK)
byteswap	TOK(BYTESWAP_TOK)
breakpoint	TOK(BREAKPOINT_TOK)
pow		TOK(POW_TOK)
floor	TOK(FLOOR_TOK)
ceil	TOK(CEIL_TOK)
sqrt	TOK(SQRT_TOK)
sin		TOK(SIN_TOK)
cos		TOK(COS_TOK)
tan		TOK(TAN_TOK)
asin	TOK(ASIN_TOK)
acos	TOK(ACOS_TOK)
atan	TOK(ATAN_TOK)
push	TOK(PUSH_TOK)

0[0-7]+						TOKINT(INT_VAL, strtoull(yytext, NULL, 8))
0[0-7]+LL						TOKINT(INT64_VAL, strtoull(yytext, NULL, 8))
[0-9]+						TOKINT(INT_VAL, strtoull(yytext, NULL, 10))
[0-9]+LL						TOKINT(INT64_VAL, strtoull(yytext, NULL, 10))
0x[0-9a-fA-F]+					TOKINT(INT_VAL, strtoull(yytext + 2, NULL, 16))
0x[0-9a-fA-F]+LL					TOKINT(INT64_VAL, strtoull(yytext + 2, NULL, 16))
0b[0-1]+					TOKINT(INT_VAL, strtoull(yytext + 2, NULL, 2))
0b[0-1]+LL					TOKINT(INT64_VAL, strtoull(yytext + 2, NULL, 2))
[0-9]+\.[0-9]+(([eE][-+]?[0-9]+)?)f		TOKFLOAT(FLOAT_VAL, atof(yytext))
\.[0-9]+(([eE][-+]?[0-9]+)?)f			TOKFLOAT(FLOAT_VAL, atof(yytext))
[0-9]+\.(([eE][-+]?[0-9]+)?)f			TOKFLOAT(FLOAT_VAL, atof(yytext))
[0-9]+([eE][-+]?[0-9]+)f				TOKFLOAT(FLOAT_VAL, atof(yytext))
[0-9]+\.[0-9]+(([eE][-+]?[0-9]+)?)		TOKFLOAT(DOUBLE_VAL, atof(yytext))
\.[0-9]+(([eE][-+]?[0-9]+)?)			TOKFLOAT(DOUBLE_VAL, atof(yytext))
[0-9]+\.(([eE][-+]?[0-9]+)?)			TOKFLOAT(DOUBLE_VAL, atof(yytext))
[0-9]+([eE][-+]?[0-9]+)				TOKFLOAT(DOUBLE_VAL, atof(yytext))

\"([^\"\r\n]*(\\\")?)*\"			TOKQUOTEDSTR(STRING_VAL, yytext)
\'([^\'\r\n]*(\\\')?)*\'			TOKQUOTEDSTR(CHAR_VAL, yytext)
[[:alpha:]_][[:alnum:]_]*			TOKSTR(ID, yytext)
\%[[:alpha:]_][[:alnum:]_]*\:		TOKQUOTEDSTR(ARG_ID_COLON, yytext)
\%[[:alpha:]_][[:alnum:]_]*			TOKSTR(ARG_ID, &yytext[1])
\@[[:alpha:]_][[:alnum:]_]*			TOKSTR(INSTR_ID, &yytext[1])
\@\@[[:alpha:]_][[:alnum:]_]*		TOKSTR(INSTR_VAL_ID, &yytext[2])

.		TOK(_ERROR)
<<EOF>>		TOK(END)
%%

