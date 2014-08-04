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

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "ParserState.h"
%}

%union
{
	int64_t intval;
	double floatval;
	char* str;
	CodeBlock* code;
	CodeToken* token;
	TreeNode* node;
	OperandDefinition* opdef;
	std::map< std::string, Ref<CodeBlock> >* operands;
	OperandType optype;
	OperandAccessType opaccess;
	InstructionToken* instrtoken;
	std::vector<InstructionToken>* instrtokenlist;
	Encoding* encoding;
	EncodingField* field;
	std::vector< Ref<CodeBlock> >* reglist;
}

%{
using namespace std;

extern int Codegen_lex(void* yyscanner);
extern void Codegen_set_extra(void* user_defined, void* yyscanner);

static int Codegen_lex_wrapper(void* yyscanner)
{
	return Codegen_lex(yyscanner);
}

#define Codegen_lex Codegen_lex_final

static int Codegen_lex_final(YYSTYPE* lval, ParserState* state)
{
	state->SetLValue(lval);
	Codegen_set_extra(state, state->GetScanner());
	return Codegen_lex_wrapper(state->GetScanner());
}

void Codegen_error(ParserState* state, const char* msg)
{
	state->Error();
	fprintf(stderr, "%s:%d: error: %s\n", state->GetFileName().c_str(), state->GetLineNumber(), msg);
}

#define YYLOC state->GetFileName(), state->GetLineNumber()
%}

%pure-parser
%parse-param {ParserState* state}
%lex-param {ParserState* state}

%token END 0
%token _ERROR

%token COLON
%token LPAREN RPAREN LBRACKET RBRACKET
%token COMMA
%token ARROW SEMICOLON LBRACE RBRACE
%token ARCH_TOK REGISTERCLASS LARGEREGISTERCLASS TEMPREGISTERCLASS
%token IMMEDIATECLASS
%token LOW_TOK HIGH_TOK BASE_TOK OFFSET_TOK TEMP_TOK BLOCK_TOK
%token MEMORY_TOK BRANCH_TOK READFLAGS_TOK WRITEFLAGS_TOK COPY_TOK STACK_TOK
%token SIGNED8 UNSIGNED8 SIGNED16 UNSIGNED16 SIGNED32 UNSIGNED32 SIGNED64 UNSIGNED64 SIGNED128 UNSIGNED128 FLOAT32 FLOAT64
%token FUNCTION VAR_TOK DEFAULT_TOK INCLUDE_TOK INSTR_TOK ENCODING_TOK EQUAL_TOK UPDATE_TOK
%token FIXED_TOK CALLERSAVED_TOK CALLEESAVED_TOK SPECIAL_TOK
%token ASSIGN_TOK LOAD_TOK STORE_TOK REF_TOK ADD_TOK SUB_TOK SMUL_TOK UMUL_TOK SDIV_TOK UDIV_TOK SMOD_TOK UMOD_TOK
%token AND_TOK OR_TOK XOR_TOK SHL_TOK SHR_TOK SAR_TOK NEG_TOK NOT_TOK IFTRUE_TOK IFSLT_TOK IFULT_TOK
%token IFSLE_TOK IFULE_TOK IFE_TOK GOTO_TOK CALL_TOK CALLVOID_TOK SYSCALL_TOK SYSCALLVOID_TOK
%token SCONVERT_TOK UCONVERT_TOK RETURN_TOK RETURNVOID_TOK ALLOCA_TOK
%token MEMCPY_TOK MEMSET_TOK STRLEN_TOK RDTSC_TOK RDTSC_LOW_TOK RDTSC_HIGH_TOK
%token VARARG_TOK BYTESWAP_TOK BREAKPOINT_TOK POW_TOK FLOOR_TOK CEIL_TOK SQRT_TOK SIN_TOK COS_TOK TAN_TOK
%token ASIN_TOK ACOS_TOK ATAN_TOK PUSH_TOK
%token <intval> INT_VAL
%token <intval> INT64_VAL
%token <floatval> FLOAT_VAL
%token <floatval> DOUBLE_VAL
%token <str> STRING_VAL CHAR_VAL
%token <str> ID ARG_ID ARG_ID_COLON INSTR_ID INSTR_VAL_ID OPERATOR

%type <code> code code_stmt_list code_stmt token_list function_type_list function_arg_list
%type <code> operand_token_list inner_operand_token_list
%type <code> instr_code instr_code_stmt_list instr_code_stmt
%type <token> token function_type_token function_arg_token keyword_token
%type <token> operand_token inner_operand_token
%type <node> match tree temp_list
%type <intval> match_type match_type_list instr_flag instr_flag_list
%type <str> match_name operand_name instr_name
%type <opdef> operand_def
%type <operands> operand_def_list
%type <optype> operand_type
%type <opaccess> operand_access
%type <instrtoken> instr_token
%type <instrtokenlist> instr_token_list
%type <encoding> encoding_field_list
%type <field> encoding_field
%type <reglist> reg_list

%destructor { free($$); } STRING_VAL CHAR_VAL
%destructor { free($$); } ID ARG_ID ARG_ID_COLON INSTR_ID INSTR_VAL_ID
%destructor { $$->Release(); } code code_stmt_list code_stmt token_list function_type_list function_arg_list
%destructor { $$->Release(); } operand_token_list inner_operand_token_list
%destructor { $$->Release(); } instr_code instr_code_stmt_list instr_code_stmt
%destructor { delete $$; } token function_type_token function_arg_token keyword_token
%destructor { delete $$; } operand_token inner_operand_token
%destructor { $$->Release(); } match tree temp_list
%destructor { free($$); } match_name operand_name instr_name
%destructor { delete $$; } operand_def
%destructor { delete $$; } operand_def_list
%destructor { delete $$; } instr_token
%destructor { delete $$; } instr_token_list
%destructor { $$->Release(); } encoding_field_list
%destructor { delete $$; } encoding_field
%destructor { delete $$; } reg_list

%%

input: toplevel_stmt_list END
	 |
	 ;

toplevel_stmt_list: toplevel_stmt_list toplevel_stmt
				  | toplevel_stmt
				  ;

toplevel_stmt: class_stmt
			 | function_stmt
			 | var_stmt
			 | match_stmt
			 | encoding_stmt
			 | instr_stmt
			 | ARCH_TOK ID INT_VAL  { state->SetArchName($2); state->SetArchBits((uint32_t)$3); free($2); }
			 | INCLUDE_TOK STRING_VAL  { state->AddInclude($2); free($2); }
			 | CALLERSAVED_TOK LBRACE reg_list RBRACE  { state->AddCallerSavedRegs(*$3); delete $3; }
			 | CALLEESAVED_TOK LBRACE reg_list RBRACE  { state->AddCalleeSavedRegs(*$3); delete $3; }
			 | SPECIAL_TOK ID code  { state->DefineSpecialReg($2, $3); free($2); $3->Release(); }
			 ;	

reg_list: reg_list COMMA operand_token_list  { $$ = $1; $$->push_back($3); $3->Release(); }
		| operand_token_list  { $$ = new vector< Ref<CodeBlock> >(); $$->push_back($1); $1->Release(); }
		;

class_stmt: REGISTERCLASS ID LPAREN match_type_list RPAREN  { state->DefineRegisterClass($4, $2, NULL, false); free($2); }
		  | REGISTERCLASS ID LPAREN match_type_list RPAREN COLON ID
		  	{
				state->DefineRegisterClass($4, $2, NULL, false);
				state->DefineRegisterSubclass($2, $7);
				free($2);
				free($7);
			}
		  | REGISTERCLASS ID LPAREN match_type_list RPAREN FIXED_TOK code
		  	{
				state->DefineRegisterClass($4, $2, $7, false);
				free($2);
				$7->Release();
			}
		  | REGISTERCLASS ID LPAREN match_type_list RPAREN FIXED_TOK code COLON ID
		  	{
				state->DefineRegisterClass($4, $2, $7, false);
				state->DefineRegisterSubclass($2, $9);
				free($2);
				$7->Release();
				free($9);
			}
		  | REGISTERCLASS DEFAULT_TOK ID LPAREN match_type_list RPAREN
		  	{
				state->DefineRegisterClass($5, $3, NULL, true);
				free($3);
			}
		  | REGISTERCLASS DEFAULT_TOK ID LPAREN match_type_list RPAREN COLON ID
		  	{
				state->DefineRegisterClass($5, $3, NULL, true);
				state->DefineRegisterSubclass($3, $8);
				free($3);
				free($8);
			}
		  | REGISTERCLASS DEFAULT_TOK ID LPAREN match_type_list RPAREN FIXED_TOK code
		  	{
				state->DefineRegisterClass($5, $3, $8, true);
				free($3);
				$8->Release();
			}
		  | REGISTERCLASS DEFAULT_TOK ID LPAREN match_type_list RPAREN FIXED_TOK code COLON ID
		  	{
				state->DefineRegisterClass($5, $3, $8, true);
				state->DefineRegisterSubclass($3, $10);
				free($3);
				$8->Release();
				free($10);
			}
		  | LARGEREGISTERCLASS ID LPAREN match_type_list RPAREN ID ID
		  	{
				state->DefineLargeRegisterClass($4, $2, $6, $7);
				free($2);
				free($6);
				free($7);
			}
		  | TEMPREGISTERCLASS ID LPAREN match_type_list RPAREN ID  { state->DefineTempRegisterClass($4, $2, $6); free($2); free($6); }
		  | IMMEDIATECLASS ID code  { state->DefineImmediateClass($2, $3); free($2); $3->Release(); }
		  ;

code: LBRACE code_stmt_list RBRACE  { $$ = $2; }
	| LBRACE RBRACE  { $$ = new CodeBlock; $$->AddRef(); }
	;

code_stmt_list: code_stmt_list code_stmt  { $$ = $1; $$->AddTokens($2->GetTokens()); $2->Release(); }
			  | code_stmt  { $$ = $1; }
			  ;

code_stmt: token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
		 | SEMICOLON  { $$ = new CodeBlock; $$->AddRef(); $$->AddTextToken(YYLOC, ";"); }
		 | LBRACE code_stmt_list RBRACE
		 	{
				$$ = new CodeBlock;
				$$->AddRef();
				$$->AddTextToken(YYLOC, "{");
				$$->AddTokens($2->GetTokens());
				$$->AddTextToken(YYLOC, "}");
				$2->Release();
		 	}
		 | INSTR_ID token_list SEMICOLON
		 	{
				$$ = new CodeBlock;
				$$->AddRef();
				$$->AddInstrStartToken(YYLOC, $1);
				$$->AddTokens($2->GetTokens());
				$$->AddInstrEndToken(YYLOC);
				$$->AddTextToken(YYLOC, ";");
				free($1);
				$2->Release();
			}
		 | INSTR_ID SEMICOLON
		 	{
				$$ = new CodeBlock;
				$$->AddRef();
				$$->AddInstrStartToken(YYLOC, $1);
				$$->AddInstrEndToken(YYLOC);
				$$->AddTextToken(YYLOC, ";");
				free($1);
			}
		 ;

token_list: token_list token  { $$ = $1; $$->AddToken(*$2); delete $2; }
		  | token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
		  ;

token: ID  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
	 | INT_VAL
	 	{
			char str[32];
			sprintf(str, "%" PRId64, $1);
			$$ = CodeToken::CreateTextToken(YYLOC, str);
		}
	 | INT64_VAL
	 	{
			char str[32];
			sprintf(str, "%" PRId64 "LL", $1);
			$$ = CodeToken::CreateTextToken(YYLOC, str);
		}
	 | STRING_VAL
	 	{
			$$ = CodeToken::CreateTextToken(YYLOC, std::string("\"") + std::string($1) + std::string("\""));
			free($1);
		}
	 | CHAR_VAL
	 	{
			$$ = CodeToken::CreateTextToken(YYLOC, std::string("\'") + std::string($1) + std::string("\'"));
			free($1);
		}
	 | ARG_ID  { $$ = CodeToken::CreateVarToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON LOW_TOK  { $$ = CodeToken::CreateVarLowToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON HIGH_TOK  { $$ = CodeToken::CreateVarHighToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON INT_VAL  { $$ = CodeToken::CreateVarIntToken(YYLOC, $1, (int)$2); free($1); }
	 | ARG_ID_COLON BASE_TOK  { $$ = CodeToken::CreateVarBaseToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON OFFSET_TOK  { $$ = CodeToken::CreateVarOffsetToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON TEMP_TOK  { $$ = CodeToken::CreateVarTempToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON FUNCTION  { $$ = CodeToken::CreateVarFunctionToken(YYLOC, $1); free($1); }
	 | ARG_ID_COLON BLOCK_TOK  { $$ = CodeToken::CreateVarBlockToken(YYLOC, $1); free($1); }
	 | OPERATOR  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
	 | COLON  { $$ = CodeToken::CreateTextToken(YYLOC, ":"); }
	 | COMMA  { $$ = CodeToken::CreateTextToken(YYLOC, ","); }
	 | LPAREN  { $$ = CodeToken::CreateTextToken(YYLOC, "("); }
	 | RPAREN  { $$ = CodeToken::CreateTextToken(YYLOC, ")"); }
	 | LBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "["); }
	 | RBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "]"); }
	 | EQUAL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "="); }
	 | keyword_token  { $$ = $1; }
	 ;

keyword_token: SIGNED8  { $$ = CodeToken::CreateTextToken(YYLOC, "S8"); }
			 | UNSIGNED8  { $$ = CodeToken::CreateTextToken(YYLOC, "U8"); }
			 | SIGNED16  { $$ = CodeToken::CreateTextToken(YYLOC, "S16"); }
			 | UNSIGNED16  { $$ = CodeToken::CreateTextToken(YYLOC, "U16"); }
			 | SIGNED32  { $$ = CodeToken::CreateTextToken(YYLOC, "S32"); }
			 | UNSIGNED32  { $$ = CodeToken::CreateTextToken(YYLOC, "U32"); }
			 | SIGNED64  { $$ = CodeToken::CreateTextToken(YYLOC, "S64"); }
			 | UNSIGNED64  { $$ = CodeToken::CreateTextToken(YYLOC, "U64"); }
			 | SIGNED128  { $$ = CodeToken::CreateTextToken(YYLOC, "S128"); }
			 | UNSIGNED128  { $$ = CodeToken::CreateTextToken(YYLOC, "U128"); }
			 | FLOAT32  { $$ = CodeToken::CreateTextToken(YYLOC, "F32"); }
			 | FLOAT64  { $$ = CodeToken::CreateTextToken(YYLOC, "F64"); }
			 | FUNCTION  { $$ = CodeToken::CreateTextToken(YYLOC, "function"); }
			 | VAR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "var"); }
			 | DEFAULT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "default"); }
			 | INCLUDE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "include"); }
			 | INSTR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "instr"); }
			 | ENCODING_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "encoding"); }
			 | ASSIGN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "assign"); }
			 | LOAD_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "load"); }
			 | STORE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "store"); }
			 | REF_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ref"); }
			 | ADD_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "add"); }
			 | SUB_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "sub"); }
			 | SMUL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "smul"); }
			 | UMUL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "umul"); }
			 | SDIV_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "sdiv"); }
			 | UDIV_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "udiv"); }
			 | SMOD_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "smod"); }
			 | UMOD_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "umod"); }
			 | AND_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "and"); }
			 | OR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "or"); }
			 | XOR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "xor"); }
			 | SHL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "shl"); }
			 | SHR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "shr"); }
			 | SAR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "sar"); }
			 | NEG_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "neg"); }
			 | NOT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "not"); }
			 | IFTRUE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "iftrue"); }
			 | IFSLT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ifslt"); }
			 | IFULT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ifult"); }
			 | IFSLE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ifsle"); }
			 | IFULE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ifule"); }
			 | IFE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ife"); }
			 | GOTO_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "goto"); }
			 | CALL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "call"); }
			 | CALLVOID_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "callvoid"); }
			 | SYSCALL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "syscall"); }
			 | SYSCALLVOID_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "syscallvoid"); }
			 | SCONVERT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "sconvert"); }
			 | UCONVERT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "uconvert"); }
			 | RETURN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "return"); }
			 | RETURNVOID_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "returnvoid"); }
			 | ALLOCA_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "alloca"); }
			 | MEMCPY_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "memcpy"); }
			 | MEMSET_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "memset"); }
			 | STRLEN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "strlen"); }
			 | RDTSC_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "rdtsc"); }
			 | RDTSC_LOW_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "rdtsc_low"); }
			 | RDTSC_HIGH_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "rdtsc_high"); }
			 | VARARG_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "vararg"); }
			 | BYTESWAP_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "byteswap"); }
			 | BREAKPOINT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "breakpoint"); }
			 | POW_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "pow"); }
			 | FLOOR_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "floor"); }
			 | CEIL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "ceil"); }
			 | SQRT_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "sqrt"); }
			 | SIN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "sin"); }
			 | COS_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "cos"); }
			 | TAN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "tan"); }
			 | ASIN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "asin"); }
			 | ACOS_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "acos"); }
			 | ATAN_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "atan"); }
			 | PUSH_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "push"); }
			 | LOW_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "low"); }
			 | HIGH_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "high"); }
			 | BASE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "base"); }
			 | OFFSET_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "offset"); }
			 | TEMP_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "temp"); }
			 | BLOCK_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "block"); }
			 | MEMORY_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "memory"); }
			 | BRANCH_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "branch"); }
			 | READFLAGS_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "readflags"); }
			 | WRITEFLAGS_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "writeflags"); }
			 | COPY_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "copy"); }
			 | STACK_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "stack"); }
			 | UPDATE_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "update"); }
			 | FIXED_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "fixed"); }
			 | CALLERSAVED_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "callersaved"); }
			 | CALLEESAVED_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "calleesaved"); }
			 | SPECIAL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "special"); }
			 ;

function_stmt: FUNCTION function_type_list LPAREN function_arg_list RPAREN code
			 	{
					CodeBlock* func = $2;
					func->AddTextToken(YYLOC, "(");
					func->AddTokens($4->GetTokens());
					func->AddTextToken(YYLOC, ")");
					func->AddTextToken(YYLOC, "{");
					func->AddTokens($6->GetTokens());
					func->AddTextToken(YYLOC, "}");
					state->DefineFunction(func);
					$2->Release();
					$4->Release();
					$6->Release();
				}
			 | FUNCTION function_type_list LPAREN RPAREN code
			 	{
					CodeBlock* func = $2;
					func->AddTextToken(YYLOC, "(");
					func->AddTextToken(YYLOC, ")");
					func->AddTextToken(YYLOC, "{");
					func->AddTokens($5->GetTokens());
					func->AddTextToken(YYLOC, "}");
					state->DefineFunction(func);
					$2->Release();
					$5->Release();
				}
			| INSTR_TOK FUNCTION function_type_list LPAREN function_arg_list RPAREN instr_code
			 	{
					CodeBlock* func = $3;
					func->AddTextToken(YYLOC, "(");
					func->AddTokens($5->GetTokens());
					func->AddTextToken(YYLOC, ")");
					func->AddTextToken(YYLOC, "{");
					func->AddTokens($7->GetTokens());
					func->AddTextToken(YYLOC, "}");
					state->DefineInstrFunction(func);
					$3->Release();
					$5->Release();
					$7->Release();
				}
			 | INSTR_TOK FUNCTION function_type_list LPAREN RPAREN instr_code
			 	{
					CodeBlock* func = $3;
					func->AddTextToken(YYLOC, "(");
					func->AddTextToken(YYLOC, ")");
					func->AddTextToken(YYLOC, "{");
					func->AddTokens($6->GetTokens());
					func->AddTextToken(YYLOC, "}");
					state->DefineInstrFunction(func);
					$3->Release();
					$6->Release();
				}
			| ARCH_TOK FUNCTION function_type_list LPAREN function_arg_list RPAREN code
			 	{
					CodeBlock* func = $3;
					func->AddTextToken(YYLOC, "(");
					func->AddTokens($5->GetTokens());
					func->AddTextToken(YYLOC, ")");
					func->AddTextToken(YYLOC, "{");
					func->AddTokens($7->GetTokens());
					func->AddTextToken(YYLOC, "}");
					state->DefineArchFunction(func);
					$3->Release();
					$5->Release();
					$7->Release();
				}
			 | ARCH_TOK FUNCTION function_type_list LPAREN RPAREN code
			 	{
					CodeBlock* func = $3;
					func->AddTextToken(YYLOC, "(");
					func->AddTextToken(YYLOC, ")");
					func->AddTextToken(YYLOC, "{");
					func->AddTokens($6->GetTokens());
					func->AddTextToken(YYLOC, "}");
					state->DefineArchFunction(func);
					$3->Release();
					$6->Release();
				}
			 ;

var_stmt: VAR_TOK token_list SEMICOLON  { $2->AddTextToken(YYLOC, ";"); state->DefineVariable($2); $2->Release(); }
		;

function_type_list: function_arg_list function_type_token  { $$ = $1; $$->AddToken(*$2); delete $2; }
				 | function_type_token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
				 ;

function_type_token: ID  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
				   | OPERATOR  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
				   | LBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "["); }
				   | RBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "]"); }
				   | keyword_token  { $$ = $1; }
				   ;

function_arg_list: function_arg_list function_arg_token  { $$ = $1; $$->AddToken(*$2); delete $2; }
				 | function_arg_token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
				 ;

function_arg_token: ID  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
				  | OPERATOR  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
				  | COMMA  { $$ = CodeToken::CreateTextToken(YYLOC, ","); }
				  | LBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "["); }
				  | RBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "]"); }
				  | EQUAL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "="); }
				  | keyword_token  { $$ = $1; }
				  ;

match_stmt: tree code  { state->DefineMatch(YYLOC, $1, NULL, NULL, $2); $1->Release(); $2->Release(); }
		  | tree COMMA temp_list code { state->DefineMatch(YYLOC, $1, NULL, $3, $4); $1->Release(); $3->Release(); $4->Release(); }
		  | tree ARROW match code  { state->DefineMatch(YYLOC, $1, $3, NULL, $4); $1->Release(); $3->Release(); $4->Release(); }
		  | tree ARROW match COMMA temp_list code
		  	{
				state->DefineMatch(YYLOC, $1, $3, $5, $6);
				$1->Release();
				$3->Release();
				$5->Release();
				$6->Release();
			}
		  ;

temp_list: temp_list COMMA match  { $$ = $1; $$->AddChildNode($3); $3->Release(); }
		 | match  { $$ = new TreeNode(NODE_REG); $$->AddRef(); $$->AddChildNode($1); $1->Release(); }
		 ;

match: match_name COLON ID
	 	{
			uint32_t sizeFlags = REG_MATCH_ALL;
			RegisterClassDef* cls = state->GetRegisterClass($3);
			if (cls)
				sizeFlags &= cls->GetSizeFlags();
			$$ = TreeNode::CreateRegNode($1, $3, sizeFlags);
			$$->AddRef();
			free($1);
			free($3);
		}
	 | match_name COLON ID LPAREN match_type_list RPAREN
	 	{
			uint32_t sizeFlags = $5;
			RegisterClassDef* cls = state->GetRegisterClass($3);
			if (cls)
				sizeFlags &= cls->GetSizeFlags();
			$$ = TreeNode::CreateRegNode($1, $3, sizeFlags);
			$$->AddRef();
			free($1);
			free($3);
		}
	 ;

match_name: ID  { $$ = $1; }
		  | LOW_TOK  { $$ = strdup("low"); }
		  | HIGH_TOK  { $$ = strdup("high"); }
		  | BASE_TOK  { $$ = strdup("base"); }
		  | OFFSET_TOK  { $$ = strdup("offset"); }
		  | TEMP_TOK  { $$ = strdup("temp"); }
		  | BLOCK_TOK  { $$ = strdup("block"); }
		  | MEMORY_TOK  { $$ = strdup("memory"); }
		  | BRANCH_TOK  { $$ = strdup("branch"); }
		  | READFLAGS_TOK  { $$ = strdup("readflags"); }
		  | WRITEFLAGS_TOK  { $$ = strdup("writeflags"); }
		  | COPY_TOK  { $$ = strdup("copy"); }
		  | STACK_TOK  { $$ = strdup("stack"); }
		  ;

match_type_list: match_type_list COMMA match_type  { $$ = $1 | $3; }
			   | match_type  { $$ = $1; }
			   ;

match_type: INT_VAL
		  	{
				if ($1 == 8)
					$$ = REG_MATCH_S8 | REG_MATCH_U8;
				else if ($1 == 16)
					$$ = REG_MATCH_S16 | REG_MATCH_U16;
				else if ($1 == 32)
					$$ = REG_MATCH_S32 | REG_MATCH_U32;
				else if ($1 == 64)
					$$ = REG_MATCH_S64 | REG_MATCH_U64;
				else if ($1 == 128)
					$$ = REG_MATCH_S128 | REG_MATCH_U128;
				else
					Codegen_error(state, "invalid register size");
			}
		  | SIGNED8  { $$ = REG_MATCH_S8; }
		  | UNSIGNED8  { $$ = REG_MATCH_U8; }
		  | SIGNED16  { $$ = REG_MATCH_S16; }
		  | UNSIGNED16  { $$ = REG_MATCH_U16; }
		  | SIGNED32  { $$ = REG_MATCH_S32; }
		  | UNSIGNED32  { $$ = REG_MATCH_U32; }
		  | SIGNED64  { $$ = REG_MATCH_S64; }
		  | UNSIGNED64  { $$ = REG_MATCH_U64; }
		  | SIGNED128  { $$ = REG_MATCH_S128; }
		  | UNSIGNED128  { $$ = REG_MATCH_U128; }
		  | FLOAT32  { $$ = REG_MATCH_F32; }
		  | FLOAT64  { $$ = REG_MATCH_F64; }
		  ;

tree: match  { $$ = $1; }
	| ASSIGN_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_ASSIGN, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| LOAD_TOK tree  { $$ = TreeNode::CreateLoadNode($2, REG_MATCH_ALL); $$->AddRef(); $2->Release(); }
	| LOAD_TOK LPAREN match_type_list RPAREN tree  { $$ = TreeNode::CreateLoadNode($5, $3); $$->AddRef(); $5->Release(); }
	| STORE_TOK tree tree
		{
			$$ = TreeNode::CreateStoreNode($2, $3, REG_MATCH_ALL);
			$$->AddRef();
			$2->Release();
			$3->Release();
		}
	| STORE_TOK LPAREN match_type_list RPAREN tree tree
		{
			$$ = TreeNode::CreateStoreNode($5, $6, $3);
			$$->AddRef();
			$5->Release();
			$6->Release();
		}
	| REF_TOK tree  { $$ = TreeNode::CreateNode(NODE_REF, $2); $$->AddRef(); $2->Release(); }
	| ADD_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_ADD, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SUB_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SUB, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SMUL_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SMUL, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| UMUL_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_UMUL, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SDIV_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SDIV, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| UDIV_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_UDIV, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SMOD_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SMOD, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| UMOD_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_UMOD, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| AND_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_AND, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| OR_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_OR, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| XOR_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_XOR, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SHL_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SHL, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SHR_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SHR, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| SAR_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_SAR, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| NEG_TOK tree  { $$ = TreeNode::CreateNode(NODE_NEG, $2); $$->AddRef(); $2->Release(); }
	| NOT_TOK tree  { $$ = TreeNode::CreateNode(NODE_NOT, $2); $$->AddRef(); $2->Release(); }
	| IFTRUE_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_IFTRUE, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| IFSLT_TOK tree tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_IFSLT, $2, $3, $4, $5);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
			$5->Release();
		}
	| IFULT_TOK tree tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_IFULT, $2, $3, $4, $5);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
			$5->Release();
		}
	| IFSLE_TOK tree tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_IFSLE, $2, $3, $4, $5);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
			$5->Release();
		}
	| IFULE_TOK tree tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_IFULE, $2, $3, $4, $5);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
			$5->Release();
		}
	| IFE_TOK tree tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_IFE, $2, $3, $4, $5);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
			$5->Release();
		}
	| GOTO_TOK tree  { $$ = TreeNode::CreateNode(NODE_GOTO, $2); $$->AddRef(); $2->Release(); }
	| CALL_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_CALL, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| CALLVOID_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_CALLVOID, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| SYSCALL_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_SYSCALL, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| SYSCALLVOID_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_SYSCALLVOID, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| SCONVERT_TOK tree  { $$ = TreeNode::CreateNode(NODE_SCONVERT, $2); $$->AddRef(); $2->Release(); }
	| UCONVERT_TOK tree  { $$ = TreeNode::CreateNode(NODE_UCONVERT, $2); $$->AddRef(); $2->Release(); }
	| RETURN_TOK tree  { $$ = TreeNode::CreateNode(NODE_RETURN, $2); $$->AddRef(); $2->Release(); }
	| RETURNVOID_TOK  { $$ = TreeNode::CreateNode(NODE_RETURNVOID); $$->AddRef(); }
	| ALLOCA_TOK tree  { $$ = TreeNode::CreateNode(NODE_ALLOCA, $2); $$->AddRef(); $2->Release(); }
	| MEMCPY_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_MEMCPY, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| MEMSET_TOK tree tree tree
		{
			$$ = TreeNode::CreateNode(NODE_MEMSET, $2, $3, $4);
			$$->AddRef();
			$2->Release();
			$3->Release();
			$4->Release();
		}
	| STRLEN_TOK tree  { $$ = TreeNode::CreateNode(NODE_STRLEN, $2); $$->AddRef(); $2->Release(); }
	| RDTSC_TOK tree  { $$ = TreeNode::CreateNode(NODE_RDTSC, $2); $$->AddRef(); $2->Release(); }
	| RDTSC_LOW_TOK tree  { $$ = TreeNode::CreateNode(NODE_RDTSC_LOW, $2); $$->AddRef(); $2->Release(); }
	| RDTSC_HIGH_TOK tree  { $$ = TreeNode::CreateNode(NODE_RDTSC_HIGH, $2); $$->AddRef(); $2->Release(); }
	| VARARG_TOK  { $$ = TreeNode::CreateNode(NODE_VARARG); $$->AddRef(); }
	| BYTESWAP_TOK tree  { $$ = TreeNode::CreateNode(NODE_BYTESWAP, $2); $$->AddRef(); $2->Release(); }
	| BREAKPOINT_TOK  { $$ = TreeNode::CreateNode(NODE_BREAKPOINT); $$->AddRef(); }
	| POW_TOK tree tree  { $$ = TreeNode::CreateNode(NODE_POW, $2, $3); $$->AddRef(); $2->Release(); $3->Release(); }
	| FLOOR_TOK tree  { $$ = TreeNode::CreateNode(NODE_FLOOR, $2); $$->AddRef(); $2->Release(); }
	| CEIL_TOK tree  { $$ = TreeNode::CreateNode(NODE_CEIL, $2); $$->AddRef(); $2->Release(); }
	| SQRT_TOK tree  { $$ = TreeNode::CreateNode(NODE_SQRT, $2); $$->AddRef(); $2->Release(); }
	| SIN_TOK tree  { $$ = TreeNode::CreateNode(NODE_SIN, $2); $$->AddRef(); $2->Release(); }
	| COS_TOK tree  { $$ = TreeNode::CreateNode(NODE_COS, $2); $$->AddRef(); $2->Release(); }
	| TAN_TOK tree  { $$ = TreeNode::CreateNode(NODE_TAN, $2); $$->AddRef(); $2->Release(); }
	| ASIN_TOK tree  { $$ = TreeNode::CreateNode(NODE_ASIN, $2); $$->AddRef(); $2->Release(); }
	| ACOS_TOK tree  { $$ = TreeNode::CreateNode(NODE_ACOS, $2); $$->AddRef(); $2->Release(); }
	| ATAN_TOK tree  { $$ = TreeNode::CreateNode(NODE_ATAN, $2); $$->AddRef(); $2->Release(); }
	| PUSH_TOK tree  { $$ = TreeNode::CreateNode(NODE_PUSH, $2); $$->AddRef(); $2->Release(); }
	;

encoding_stmt: ENCODING_TOK ID LBRACE encoding_field_list RBRACE  { state->DefineEncoding($2, $4); free($2); $4->Release(); }
			 ;

encoding_field_list: encoding_field_list COMMA encoding_field
				   		{
							$$ = $1;
							if ($3->type != FIELD_FIXED_VALUE)
							{
								if ($$->IsFieldDefined($3->name))
									Codegen_error(state, "duplicate field names");
							}
							$$->AddField(*$3);
							delete $3;
						}
				   | encoding_field  { $$ = new Encoding(); $$->AddRef(); $$->AddField(*$1); delete $1; }
				   ;

encoding_field: ID COLON INT_VAL  { $$ = EncodingField::CreateNormalField($1, $3); free($1); }
			  | ID EQUAL_TOK INT_VAL COLON INT_VAL  { $$ = EncodingField::CreateDefaultValueField($1, $5, $3); free($1); }
			  | INT_VAL COLON INT_VAL  { $$ = EncodingField::CreateFixedValueField($3, $1); }
			  ;

instr_stmt: INSTR_TOK instr_name instr_token_list instr_code
		  	{
				Instruction* instr = new Instruction($2, 0, *$3, $4, NULL);
				state->DefineInstruction(instr);
				free($2);
				delete $3;
				$4->Release();
			}
		  | INSTR_TOK LPAREN instr_flag_list RPAREN instr_name instr_token_list instr_code
		  	{
				Instruction* instr = new Instruction($5, $3, *$6, $7, NULL);
				state->DefineInstruction(instr);
				free($5);
				delete $6;
				$7->Release();
			}
		  | INSTR_TOK instr_name instr_token_list instr_code UPDATE_TOK code
		  	{
				Instruction* instr = new Instruction($2, 0, *$3, $4, $6);
				state->DefineInstruction(instr);
				free($2);
				delete $3;
				$4->Release();
				$6->Release();
			}
		  | INSTR_TOK LPAREN instr_flag_list RPAREN instr_name instr_token_list instr_code UPDATE_TOK code
		  	{
				Instruction* instr = new Instruction($5, $3, *$6, $7, $9);
				state->DefineInstruction(instr);
				free($5);
				delete $6;
				$7->Release();
				$9->Release();
			}
		  ;

instr_token_list: instr_token_list instr_token  { $$ = $1; $$->push_back(*$2); delete $2; }
				| instr_token  { $$ = new vector<InstructionToken>(); $$->push_back(*$1); delete $1; }
				;

instr_token: operand_name  { $$ = InstructionToken::CreateTextToken(YYLOC, $1); free($1); }
		   | operand_name COLON operand_type
		   		{
					if (($3 == OPERAND_REG) || ($3 == OPERAND_REG_LIST))
						Codegen_error(state, "register operand requires access type");
					$$ = InstructionToken::CreateOperandToken(YYLOC, $1, $3, ACCESS_DEFAULT);
					free($1);
				}
		   | operand_name COLON operand_type LPAREN operand_access RPAREN
		   		{
					if (($3 != OPERAND_REG) && ($3 != OPERAND_REG_LIST))
						Codegen_error(state, "access type only valid on register operands");
					$$ = InstructionToken::CreateOperandToken(YYLOC, $1, $3, $5);
					free($1);
				}
		   | LBRACKET  { $$ = InstructionToken::CreateTextToken(YYLOC, "["); }
		   | RBRACKET  { $$ = InstructionToken::CreateTextToken(YYLOC, "]"); }
		   | COMMA  { $$ = InstructionToken::CreateTextToken(YYLOC, ","); }
		   | EQUAL_TOK  { $$ = InstructionToken::CreateTextToken(YYLOC, "="); }
		   | OPERATOR  { $$ = InstructionToken::CreateTextToken(YYLOC, $1); free($1); }
		   ;

operand_type: ID
				{
					if (!strcmp($1, "REG"))
						$$ = OPERAND_REG;
					else if (!strcmp($1, "REGLIST"))
						$$ = OPERAND_REG_LIST;
					else if (!strcmp($1, "IMM"))
						$$ = OPERAND_IMMED;
					else if (!strcmp($1, "STACKVAR"))
						$$ = OPERAND_STACK_VAR;
					else if (!strcmp($1, "GLOBALVAR"))
						$$ = OPERAND_GLOBAL_VAR;
					else if (!strcmp($1, "FUNCTION"))
						$$ = OPERAND_FUNCTION;
					else if (!strcmp($1, "TEMP"))
						$$ = OPERAND_TEMP;
					else
						Codegen_error(state, "invalid operand type");
				}
			;

operand_access: ID
			  	{
					if (!strcmp($1, "r"))
						$$ = ACCESS_READ;
					else if (!strcmp($1, "w"))
						$$ = ACCESS_WRITE;
					else if (!strcmp($1, "rw"))
						$$ = ACCESS_READ_WRITE;
					else if (!strcmp($1, "wr"))
						$$ = ACCESS_READ_WRITE;
					else
						Codegen_error(state, "invalid operand access type");
				}
			  ;

operand_name: instr_name  { $$ = $1; }
			| FUNCTION  { $$ = strdup("function"); }
			;

instr_name: match_name  { $$ = $1; }
		  | VAR_TOK  { $$ = strdup("var"); }
		  | SIGNED8  { $$ = strdup("S8"); }
		  | UNSIGNED8  { $$ = strdup("U8"); }
		  | SIGNED16  { $$ = strdup("S16"); }
		  | UNSIGNED16  { $$ = strdup("U16"); }
		  | SIGNED32  { $$ = strdup("S32"); }
		  | UNSIGNED32  { $$ = strdup("U32"); }
		  | SIGNED64  { $$ = strdup("S64"); }
		  | UNSIGNED64  { $$ = strdup("U64"); }
		  | SIGNED128  { $$ = strdup("S128"); }
		  | UNSIGNED128  { $$ = strdup("U128"); }
		  | FLOAT32  { $$ = strdup("F32"); }
		  | FLOAT64  { $$ = strdup("F64"); }
		  | DEFAULT_TOK  { $$ = strdup("default"); }
		  | INCLUDE_TOK  { $$ = strdup("include"); }
		  | INSTR_TOK  { $$ = strdup("instr"); }
		  | ENCODING_TOK  { $$ = strdup("encoding"); }
		  | ASSIGN_TOK  { $$ = strdup("assign"); }
		  | LOAD_TOK  { $$ = strdup("load"); }
		  | STORE_TOK  { $$ = strdup("store"); }
		  | REF_TOK  { $$ = strdup("ref"); }
		  | ADD_TOK  { $$ = strdup("add"); }
		  | SUB_TOK  { $$ = strdup("sub"); }
		  | SMUL_TOK  { $$ = strdup("smul"); }
		  | UMUL_TOK  { $$ = strdup("umul"); }
		  | SDIV_TOK  { $$ = strdup("sdiv"); }
		  | UDIV_TOK  { $$ = strdup("udiv"); }
		  | SMOD_TOK  { $$ = strdup("smod"); }
		  | UMOD_TOK  { $$ = strdup("umod"); }
		  | AND_TOK  { $$ = strdup("and"); }
		  | OR_TOK  { $$ = strdup("or"); }
		  | XOR_TOK  { $$ = strdup("xor"); }
		  | SHL_TOK  { $$ = strdup("shl"); }
		  | SHR_TOK  { $$ = strdup("shr"); }
		  | SAR_TOK  { $$ = strdup("sar"); }
		  | NEG_TOK  { $$ = strdup("neg"); }
		  | NOT_TOK  { $$ = strdup("not"); }
		  | IFTRUE_TOK  { $$ = strdup("iftrue"); }
		  | IFSLT_TOK  { $$ = strdup("ifslt"); }
		  | IFULT_TOK  { $$ = strdup("ifult"); }
		  | IFSLE_TOK  { $$ = strdup("ifsle"); }
		  | IFULE_TOK  { $$ = strdup("ifule"); }
		  | IFE_TOK  { $$ = strdup("ife"); }
		  | GOTO_TOK  { $$ = strdup("goto"); }
		  | CALL_TOK  { $$ = strdup("call"); }
		  | CALLVOID_TOK  { $$ = strdup("callvoid"); }
		  | SYSCALL_TOK  { $$ = strdup("syscall"); }
		  | SYSCALLVOID_TOK  { $$ = strdup("syscallvoid"); }
		  | SCONVERT_TOK  { $$ = strdup("sconvert"); }
		  | UCONVERT_TOK  { $$ = strdup("uconvert"); }
		  | RETURN_TOK  { $$ = strdup("return"); }
		  | RETURNVOID_TOK  { $$ = strdup("returnvoid"); }
		  | ALLOCA_TOK  { $$ = strdup("alloca"); }
		  | MEMCPY_TOK  { $$ = strdup("memcpy"); }
		  | MEMSET_TOK  { $$ = strdup("memset"); }
		  | STRLEN_TOK  { $$ = strdup("strlen"); }
		  | RDTSC_TOK  { $$ = strdup("rdtsc"); }
		  | RDTSC_LOW_TOK  { $$ = strdup("rdtsc_low"); }
		  | RDTSC_HIGH_TOK  { $$ = strdup("rdtsc_high"); }
		  | VARARG_TOK  { $$ = strdup("vararg"); }
		  | BYTESWAP_TOK  { $$ = strdup("byteswap"); }
		  | BREAKPOINT_TOK  { $$ = strdup("breakpoint"); }
		  | POW_TOK  { $$ = strdup("pow"); }
		  | FLOOR_TOK  { $$ = strdup("floor"); }
		  | CEIL_TOK  { $$ = strdup("ceil"); }
		  | SQRT_TOK  { $$ = strdup("sqrt"); }
		  | SIN_TOK  { $$ = strdup("sin"); }
		  | COS_TOK  { $$ = strdup("cos"); }
		  | TAN_TOK  { $$ = strdup("tan"); }
		  | ASIN_TOK  { $$ = strdup("asin"); }
		  | ACOS_TOK  { $$ = strdup("acos"); }
		  | ATAN_TOK  { $$ = strdup("atan"); }
		  | PUSH_TOK  { $$ = strdup("push"); }
		  ;

instr_flag_list: instr_flag_list COMMA instr_flag  { $$ = $1 | $3; }
			   | instr_flag  { $$ = $1; }
			   ;

instr_flag: MEMORY_TOK  { $$ = SYMFLAG_MEMORY_BARRIER; }
		  | BRANCH_TOK  { $$ = SYMFLAG_CONTROL_FLOW; }
		  | CALL_TOK  { $$ = SYMFLAG_CALL; }
		  | READFLAGS_TOK  { $$ = SYMFLAG_USES_FLAGS; }
		  | WRITEFLAGS_TOK  { $$ = SYMFLAG_WRITES_FLAGS; }
		  | COPY_TOK  { $$ = SYMFLAG_COPY; }
		  | STACK_TOK  { $$ = SYMFLAG_STACK; }
		  ;

instr_code: LBRACE instr_code_stmt_list RBRACE  { $$ = $2; }
		  | LBRACE RBRACE  { $$ = new CodeBlock; $$->AddRef(); }
		  ;

instr_code_stmt_list: instr_code_stmt_list instr_code_stmt  { $$ = $1; $$->AddTokens($2->GetTokens()); $2->Release(); }
					| instr_code_stmt  { $$ = $1; }
					;

instr_code_stmt: token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
			   | SEMICOLON  { $$ = new CodeBlock; $$->AddRef(); $$->AddTextToken(YYLOC, ";"); }
			   | LBRACE instr_code_stmt_list RBRACE
			   	{
					$$ = new CodeBlock;
					$$->AddRef();
					$$->AddTextToken(YYLOC, "{");
					$$->AddTokens($2->GetTokens());
					$$->AddTextToken(YYLOC, "}");
					$2->Release();
		 		}
			   | INSTR_ID operand_def_list SEMICOLON
		 		{
					$$ = new CodeBlock;
					$$->AddRef();
					$$->AddInstrEncodingToken(YYLOC, $1, *$2);
					$$->AddTextToken(YYLOC, ";");
					free($1);
					delete $2;
				}
			   | INSTR_ID SEMICOLON
			   	{
					$$ = new CodeBlock;
					$$->AddRef();
					$$->AddInstrEncodingToken(YYLOC, $1, map< string, Ref<CodeBlock> >());
					$$->AddTextToken(YYLOC, ";");
					free($1);
				}
			   | INSTR_VAL_ID operand_def_list SEMICOLON
		 		{
					$$ = new CodeBlock;
					$$->AddRef();
					$$->AddInstrEncodingValueToken(YYLOC, $1, *$2);
					$$->AddTextToken(YYLOC, ";");
					free($1);
					delete $2;
				}
			   | INSTR_VAL_ID SEMICOLON
		 		{
					$$ = new CodeBlock;
					$$->AddRef();
					$$->AddInstrEncodingValueToken(YYLOC, $1, map< string, Ref<CodeBlock> >());
					$$->AddTextToken(YYLOC, ";");
					free($1);
				}
			   ;

operand_def_list: operand_def_list COMMA operand_def  { $$ = $1; (*$$)[$3->name] = $3->block; delete $3; }
				| operand_def  { $$ = new map< string, Ref<CodeBlock> >(); (*$$)[$1->name] = $1->block; delete $1; }
				;

operand_def: operand_name EQUAL_TOK operand_token_list  { $$ = new OperandDefinition; $$->name = $1; $$->block = $3; free($1); }
		   ;

operand_token_list: operand_token_list operand_token  { $$ = $1; $$->AddToken(*$2); delete $2; }
				  | operand_token_list LPAREN inner_operand_token_list RPAREN
					{
						$$ = $1;
						$$->AddTextToken(YYLOC, "(");
						$$->AddTokens($3->GetTokens());
						$$->AddTextToken(YYLOC, ")");
						delete $3;
					}
				  | operand_token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
				  | LPAREN inner_operand_token_list RPAREN
					{
						$$ = new CodeBlock;
						$$->AddRef();
						$$->AddTextToken(YYLOC, "(");
						$$->AddTokens($2->GetTokens());
						$$->AddTextToken(YYLOC, ")");
						delete $2;
					}
				  ;

operand_token: ID  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
			 | INT_VAL
			 	{
					char str[32];
					sprintf(str, "%" PRId64, $1);
					$$ = CodeToken::CreateTextToken(YYLOC, str);
				}
			 | INT64_VAL
			 	{
					char str[32];
					sprintf(str, "%" PRId64 "LL", $1);
					$$ = CodeToken::CreateTextToken(YYLOC, str);
				}
			 | STRING_VAL
			 	{
					$$ = CodeToken::CreateTextToken(YYLOC, std::string("\"") + std::string($1) + std::string("\""));
					free($1);
				}
			 | CHAR_VAL
			 	{
					$$ = CodeToken::CreateTextToken(YYLOC, std::string("\'") + std::string($1) + std::string("\'"));
					free($1);
				}
			 | ARG_ID  { $$ = CodeToken::CreateVarToken(YYLOC, $1); free($1); }
			 | ARG_ID_COLON BASE_TOK  { $$ = CodeToken::CreateVarBaseToken(YYLOC, $1); free($1); }
			 | ARG_ID_COLON OFFSET_TOK  { $$ = CodeToken::CreateVarOffsetToken(YYLOC, $1); free($1); }
			 | ARG_ID_COLON TEMP_TOK  { $$ = CodeToken::CreateVarTempToken(YYLOC, $1); free($1); }
			 | ARG_ID_COLON FUNCTION  { $$ = CodeToken::CreateVarFunctionToken(YYLOC, $1); free($1); }
			 | ARG_ID_COLON BLOCK_TOK  { $$ = CodeToken::CreateVarBlockToken(YYLOC, $1); free($1); }
			 | OPERATOR  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
			 | COLON  { $$ = CodeToken::CreateTextToken(YYLOC, ":"); }
			 | LBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "["); }
			 | RBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "]"); }
			 | EQUAL_TOK  { $$ = CodeToken::CreateTextToken(YYLOC, "="); }
			 | keyword_token  { $$ = $1; }
			 ;

inner_operand_token_list: inner_operand_token_list inner_operand_token  { $$ = $1; $$->AddToken(*$2); delete $2; }
			 			| inner_operand_token_list LPAREN inner_operand_token_list RPAREN
							{
								$$ = $1;
								$$->AddTextToken(YYLOC, "(");
								$$->AddTokens($3->GetTokens());
								$$->AddTextToken(YYLOC, ")");
								delete $3;
							}
						| inner_operand_token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
			 			| LPAREN inner_operand_token_list RPAREN
							{
								$$ = new CodeBlock;
								$$->AddRef();
								$$->AddTextToken(YYLOC, "(");
								$$->AddTokens($2->GetTokens());
								$$->AddTextToken(YYLOC, ")");
								delete $2;
							}
						;

inner_operand_token: operand_token  { $$ = $1; }
				   | COMMA  { $$ = CodeToken::CreateTextToken(YYLOC, ","); }
				   ;

%%

