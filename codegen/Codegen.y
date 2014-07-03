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
%token FUNCTION
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
%token <str> ID ARG_ID INSTR_ID OPERATOR

%type <registerType> register_type
%type <code> code code_stmt_list code_stmt token_list function_type_list function_arg_list
%type <token> token function_type_token function_arg_token
%type <node> match tree temp_list
%type <intval> match_type match_type_list

%destructor { free($$); } STRING_VAL CHAR_VAL
%destructor { free($$); } ID ARG_ID INSTR_ID
%destructor { $$->Release(); } code code_stmt_list code_stmt token_list function_type_list function_arg_list
%destructor { delete $$; } token function_type_token function_arg_token
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
			 | match_stmt
			 | ARCH_TOK ID  { state->SetArchName($2); free($2); }
			 ;	

class_stmt: REGISTERCLASS register_type ID ID  { state->DefineRegisterClass($2, $3, $4); free($3); free($4); }
		  | LARGEREGISTERCLASS register_type ID ID ID
		  	{
				state->DefineLargeRegisterClass($2, $3, $4, $5);
				free($3);
				free($4);
				free($5);
			}
		  | TEMPREGISTERCLASS ID  { state->DefineTempRegisterClass($2); free($2); }
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
		 | SEMICOLON  { $$ = new CodeBlock; $$->AddRef(); $$->AddTextToken("{"); }
		 | LBRACE code_stmt_list RBRACE
		 	{
				$$ = new CodeBlock;
				$$->AddRef();
				$$->AddTextToken("{");
				$$->AddTokens($2->GetTokens());
				$$->AddTextToken("}");
				$2->Release();
		 	}
		 | INSTR_ID token_list SEMICOLON
		 	{
				$$ = new CodeBlock;
				$$->AddRef();
				$$->AddInstrStartToken($1);
				$$->AddTokens($2->GetTokens());
				$$->AddInstrEndToken();
				$$->AddTextToken(";");
				free($1);
				$2->Release();
			}
		 | INSTR_ID SEMICOLON
		 	{
				$$ = new CodeBlock;
				$$->AddRef();
				$$->AddInstrStartToken($1);
				$$->AddInstrEndToken();
				$$->AddTextToken(";");
				free($1);
			}
		 ;

token_list: token_list token  { $$ = $1; $$->AddToken(*$2); delete $2; }
		  | token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
		  ;

token: ID  { $$ = CodeToken::CreateTextToken($1); free($1); }
	 | INT_VAL
	 	{
			char str[32];
			sprintf(str, "%" PRIx64 "u", $1);
			$$ = CodeToken::CreateTextToken(str);
		}
	 | INT64_VAL
	 	{
			char str[32];
			sprintf(str, "%" PRIx64 "uLL", $1);
			$$ = CodeToken::CreateTextToken(str);
		}
	 | STRING_VAL
	 	{
			$$ = CodeToken::CreateTextToken(std::string("\"") + std::string($1) + std::string("\""));
			free($1);
		}
	 | CHAR_VAL
	 	{
			$$ = CodeToken::CreateTextToken(std::string("\'") + std::string($1) + std::string("\'"));
			free($1);
		}
	 | ARG_ID  { $$ = CodeToken::CreateVarToken($1); free($1); }
	 | ARG_ID COLON LOW_TOK  { $$ = CodeToken::CreateVarLowToken($1); free($1); }
	 | ARG_ID COLON HIGH_TOK  { $$ = CodeToken::CreateVarHighToken($1); free($1); }
	 | ARG_ID COLON INT_VAL  { $$ = CodeToken::CreateVarOffsetToken($1, (int)$3); free($1); }
	 | OPERATOR  { $$ = CodeToken::CreateTextToken($1); free($1); }
	 | COMMA  { $$ = CodeToken::CreateTextToken(","); }
	 | LPAREN  { $$ = CodeToken::CreateTextToken("("); }
	 | RPAREN  { $$ = CodeToken::CreateTextToken(")"); }
	 | LBRACKET  { $$ = CodeToken::CreateTextToken("["); }
	 | RBRACKET  { $$ = CodeToken::CreateTextToken("]"); }
	 | SIGNED8  { $$ = CodeToken::CreateTextToken("S8"); }
	 | UNSIGNED8  { $$ = CodeToken::CreateTextToken("U8"); }
	 | SIGNED16  { $$ = CodeToken::CreateTextToken("S16"); }
	 | UNSIGNED16  { $$ = CodeToken::CreateTextToken("U16"); }
	 | SIGNED32  { $$ = CodeToken::CreateTextToken("S32"); }
	 | UNSIGNED32  { $$ = CodeToken::CreateTextToken("U32"); }
	 | SIGNED64  { $$ = CodeToken::CreateTextToken("S64"); }
	 | UNSIGNED64  { $$ = CodeToken::CreateTextToken("U64"); }
	 | SIGNED128  { $$ = CodeToken::CreateTextToken("S128"); }
	 | UNSIGNED128  { $$ = CodeToken::CreateTextToken("U128"); }
	 | FLOAT32  { $$ = CodeToken::CreateTextToken("F32"); }
	 | FLOAT64  { $$ = CodeToken::CreateTextToken("F64"); }
	 | ASSIGN_TOK  { $$ = CodeToken::CreateTextToken("assign"); }
	 | LOAD_TOK  { $$ = CodeToken::CreateTextToken("load"); }
	 | STORE_TOK  { $$ = CodeToken::CreateTextToken("store"); }
	 | REF_TOK  { $$ = CodeToken::CreateTextToken("ref"); }
	 | ADD_TOK  { $$ = CodeToken::CreateTextToken("add"); }
	 | SUB_TOK  { $$ = CodeToken::CreateTextToken("sub"); }
	 | SMUL_TOK  { $$ = CodeToken::CreateTextToken("smul"); }
	 | UMUL_TOK  { $$ = CodeToken::CreateTextToken("umul"); }
	 | SDIV_TOK  { $$ = CodeToken::CreateTextToken("sdiv"); }
	 | UDIV_TOK  { $$ = CodeToken::CreateTextToken("udiv"); }
	 | SMOD_TOK  { $$ = CodeToken::CreateTextToken("smod"); }
	 | UMOD_TOK  { $$ = CodeToken::CreateTextToken("umod"); }
	 | AND_TOK  { $$ = CodeToken::CreateTextToken("and"); }
	 | OR_TOK  { $$ = CodeToken::CreateTextToken("or"); }
	 | XOR_TOK  { $$ = CodeToken::CreateTextToken("xor"); }
	 | SHL_TOK  { $$ = CodeToken::CreateTextToken("shl"); }
	 | SHR_TOK  { $$ = CodeToken::CreateTextToken("shr"); }
	 | SAR_TOK  { $$ = CodeToken::CreateTextToken("sar"); }
	 | NEG_TOK  { $$ = CodeToken::CreateTextToken("neg"); }
	 | NOT_TOK  { $$ = CodeToken::CreateTextToken("not"); }
	 | IFTRUE_TOK  { $$ = CodeToken::CreateTextToken("iftrue"); }
	 | IFSLT_TOK  { $$ = CodeToken::CreateTextToken("ifslt"); }
	 | IFULT_TOK  { $$ = CodeToken::CreateTextToken("ifult"); }
	 | IFSLE_TOK  { $$ = CodeToken::CreateTextToken("ifsle"); }
	 | IFULE_TOK  { $$ = CodeToken::CreateTextToken("ifule"); }
	 | IFE_TOK  { $$ = CodeToken::CreateTextToken("ife"); }
	 | GOTO_TOK  { $$ = CodeToken::CreateTextToken("goto"); }
	 | SCONVERT_TOK  { $$ = CodeToken::CreateTextToken("sconvert"); }
	 | UCONVERT_TOK  { $$ = CodeToken::CreateTextToken("uconvert"); }
	 | RETURN_TOK  { $$ = CodeToken::CreateTextToken("return"); }
	 | RETURNVOID_TOK  { $$ = CodeToken::CreateTextToken("returnvoid"); }
	 | ALLOCA_TOK  { $$ = CodeToken::CreateTextToken("alloca"); }
	 | MEMCPY_TOK  { $$ = CodeToken::CreateTextToken("memcpy"); }
	 | MEMSET_TOK  { $$ = CodeToken::CreateTextToken("memset"); }
	 | STRLEN_TOK  { $$ = CodeToken::CreateTextToken("strlen"); }
	 | RDTSC_TOK  { $$ = CodeToken::CreateTextToken("rdtsc"); }
	 | RDTSC_LOW_TOK  { $$ = CodeToken::CreateTextToken("rdtsc_low"); }
	 | RDTSC_HIGH_TOK  { $$ = CodeToken::CreateTextToken("rdtsc_high"); }
	 | VARARG_TOK  { $$ = CodeToken::CreateTextToken("vararg"); }
	 | BYTESWAP_TOK  { $$ = CodeToken::CreateTextToken("byteswap"); }
	 | BREAKPOINT_TOK  { $$ = CodeToken::CreateTextToken("breakpoint"); }
	 | POW_TOK  { $$ = CodeToken::CreateTextToken("pow"); }
	 | FLOOR_TOK  { $$ = CodeToken::CreateTextToken("floor"); }
	 | CEIL_TOK  { $$ = CodeToken::CreateTextToken("ceil"); }
	 | SQRT_TOK  { $$ = CodeToken::CreateTextToken("sqrt"); }
	 | SIN_TOK  { $$ = CodeToken::CreateTextToken("sin"); }
	 | COS_TOK  { $$ = CodeToken::CreateTextToken("cos"); }
	 | TAN_TOK  { $$ = CodeToken::CreateTextToken("tan"); }
	 | ASIN_TOK  { $$ = CodeToken::CreateTextToken("asin"); }
	 | ACOS_TOK  { $$ = CodeToken::CreateTextToken("acos"); }
	 | ATAN_TOK  { $$ = CodeToken::CreateTextToken("atan"); }
	 ;

function_stmt: FUNCTION function_type_list LPAREN function_arg_list RPAREN code
			 	{
					CodeBlock* func = $2;
					func->AddTextToken("(");
					func->AddTokens($4->GetTokens());
					func->AddTextToken(")");
					func->AddTokens($6->GetTokens());
					state->DefineFunction(func);
					$2->Release();
					$4->Release();
					$6->Release();
				}
			 | FUNCTION function_type_list LPAREN RPAREN code
			 	{
					CodeBlock* func = $2;
					func->AddTextToken("(");
					func->AddTextToken(")");
					func->AddTokens($5->GetTokens());
					state->DefineFunction(func);
					$2->Release();
					$5->Release();
				}
			 ;

function_type_list: function_arg_list function_type_token  { $$ = $1; $$->AddToken(*$2); delete $2; }
				 | function_type_token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
				 ;

function_type_token: ID  { $$ = CodeToken::CreateTextToken($1); free($1); }
				   | OPERATOR  { $$ = CodeToken::CreateTextToken($1); free($1); }
				   | LBRACKET  { $$ = CodeToken::CreateTextToken("["); }
				   | RBRACKET  { $$ = CodeToken::CreateTextToken("]"); }
				   ;

function_arg_list: function_arg_list function_arg_token  { $$ = $1; $$->AddToken(*$2); delete $2; }
				 | function_arg_token  { $$ = new CodeBlock; $$->AddRef(); $$->AddToken(*$1); delete $1; }
				 ;

function_arg_token: ID  { $$ = CodeToken::CreateTextToken($1); free($1); }
				  | OPERATOR  { $$ = CodeToken::CreateTextToken($1); free($1); }
				  | COMMA  { $$ = CodeToken::CreateTextToken(","); }
				  | LBRACKET  { $$ = CodeToken::CreateTextToken("["); }
				  | RBRACKET  { $$ = CodeToken::CreateTextToken("]"); }
				  ;

match_stmt: tree code  { state->DefineMatch($1, NULL, NULL, $2); $1->Release(); $2->Release(); }
		  | tree COMMA temp_list code { state->DefineMatch($1, NULL, $3, $4); $1->Release(); $3->Release(); $4->Release(); }
		  | tree ARROW match code  { state->DefineMatch($1, $3, NULL, $4); $1->Release(); $3->Release(); $4->Release(); }
		  | tree ARROW match COMMA temp_list code
		  	{
				state->DefineMatch($1, $3, $5, $6);
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

