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
	RegisterType registerType;
	CodeBlock* code;
	CodeToken* token;
	TreeNode* node;
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
%token INT8_TOK INT16_TOK INT32_TOK INT64_TOK FLOAT_TOK LOW_TOK HIGH_TOK
%token SIGNED8 UNSIGNED8 SIGNED16 UNSIGNED16 SIGNED32 UNSIGNED32 SIGNED64 UNSIGNED64 SIGNED128 UNSIGNED128 FLOAT32 FLOAT64
%token FUNCTION VAR_TOK DEFAULT_TOK INCLUDE_TOK
%token ASSIGN_TOK LOAD_TOK STORE_TOK REF_TOK ADD_TOK SUB_TOK SMUL_TOK UMUL_TOK SDIV_TOK UDIV_TOK SMOD_TOK UMOD_TOK
%token AND_TOK OR_TOK XOR_TOK SHL_TOK SHR_TOK SAR_TOK NEG_TOK NOT_TOK IFTRUE_TOK IFSLT_TOK IFULT_TOK
%token IFSLE_TOK IFULE_TOK IFE_TOK GOTO_TOK SCONVERT_TOK UCONVERT_TOK RETURN_TOK RETURNVOID_TOK ALLOCA_TOK
%token MEMCPY_TOK MEMSET_TOK STRLEN_TOK RDTSC_TOK RDTSC_LOW_TOK RDTSC_HIGH_TOK
%token VARARG_TOK BYTESWAP_TOK BREAKPOINT_TOK POW_TOK FLOOR_TOK CEIL_TOK SQRT_TOK SIN_TOK COS_TOK TAN_TOK
%token ASIN_TOK ACOS_TOK ATAN_TOK
%token <intval> INT_VAL
%token <intval> INT64_VAL
%token <floatval> FLOAT_VAL
%token <floatval> DOUBLE_VAL
%token <str> STRING_VAL CHAR_VAL
%token <str> ID ARG_ID ARG_ID_COLON INSTR_ID OPERATOR

%type <registerType> register_type
%type <code> code code_stmt_list code_stmt token_list function_type_list function_arg_list
%type <token> token function_type_token function_arg_token keyword_token
%type <node> match tree temp_list
%type <intval> match_type match_type_list

%destructor { free($$); } STRING_VAL CHAR_VAL
%destructor { free($$); } ID ARG_ID ARG_ID_COLON INSTR_ID
%destructor { $$->Release(); } code code_stmt_list code_stmt token_list function_type_list function_arg_list
%destructor { delete $$; } token function_type_token function_arg_token keyword_token
%destructor { $$->Release(); } match tree temp_list

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
			 | ARCH_TOK ID  { state->SetArchName($2); free($2); }
			 | INCLUDE_TOK STRING_VAL  { state->AddInclude($2); free($2); }
			 ;	

class_stmt: REGISTERCLASS register_type ID ID  { state->DefineRegisterClass($2, $3, $4, false); free($3); free($4); }
		  | REGISTERCLASS DEFAULT_TOK register_type ID ID  { state->DefineRegisterClass($3, $4, $5, true); free($4); free($5); }
		  | LARGEREGISTERCLASS register_type ID ID ID
		  	{
				state->DefineLargeRegisterClass($2, $3, $4, $5);
				free($3);
				free($4);
				free($5);
			}
		  | TEMPREGISTERCLASS ID ID  { state->DefineTempRegisterClass($2, $3); free($2); free($3); }
		  | IMMEDIATECLASS ID code  { state->DefineImmediateClass($2, $3); free($2); $3->Release(); }
		  ;

register_type: INT8_TOK  { $$ = REGTYPE_INT8; }
			 | INT16_TOK  { $$ = REGTYPE_INT16; }
			 | INT32_TOK  { $$ = REGTYPE_INT32; }
			 | INT64_TOK  { $$ = REGTYPE_INT64; }
			 | FLOAT_TOK  { $$ = REGTYPE_FLOAT; }
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
	 | ARG_ID_COLON INT_VAL  { $$ = CodeToken::CreateVarOffsetToken(YYLOC, $1, (int)$2); free($1); }
	 | OPERATOR  { $$ = CodeToken::CreateTextToken(YYLOC, $1); free($1); }
	 | COLON  { $$ = CodeToken::CreateTextToken(YYLOC, ":"); }
	 | COMMA  { $$ = CodeToken::CreateTextToken(YYLOC, ","); }
	 | LPAREN  { $$ = CodeToken::CreateTextToken(YYLOC, "("); }
	 | RPAREN  { $$ = CodeToken::CreateTextToken(YYLOC, ")"); }
	 | LBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "["); }
	 | RBRACKET  { $$ = CodeToken::CreateTextToken(YYLOC, "]"); }
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

temp_list: temp_list COMMA match  { $$ = $1; $$->AddChildNodes($3->GetChildNodes()); $3->Release(); }
		 | match  { $$ = new TreeNode(NODE_REG); $$->AddRef(); $$->AddChildNode($1); $1->Release(); }
		 ;

match: ID COLON ID  { $$ = TreeNode::CreateRegNode($1, $3, REG_MATCH_ALL); $$->AddRef(); free($1); free($3); }
	 | ID COLON ID LPAREN match_type_list RPAREN  { $$ = TreeNode::CreateRegNode($1, $3, $5); $$->AddRef(); free($1); free($3); }
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
	;

%%

