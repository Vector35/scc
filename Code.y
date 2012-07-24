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

#include <stdio.h>
#include <string.h>
#include "ParserState.h"
%}

%union
{
	int64_t intval;
	double floatval;
	char* str;
	bool boolean;
	Type* type;
	std::vector< std::pair< Ref<Type>, std::string > >* params;
	std::vector< Ref<Expr> >* args;
	std::vector<std::string>* ids;
	EnumMember* enumMember;
	Expr* expr;
	VarInitInfo* varInit;
	std::vector<VarInitInfo>* varInitList;
	FunctionInfo* function;
}

%{
using namespace std;

extern int Code_lex(void* yyscanner);
extern void Code_set_extra(void* user_defined, void* yyscanner);

static int Code_lex_wrapper(void* yyscanner)
{
	return Code_lex(yyscanner);
}

#define Code_lex Code_lex_final

static int Code_lex_final(YYSTYPE* lval, ParserState* state)
{
	state->SetLValue(lval);
	Code_set_extra(state, state->GetScanner());
	return Code_lex_wrapper(state->GetScanner());
}

void Code_error(ParserState* state, const char* msg)
{
	state->Error();
	fprintf(stderr, "%s:%d: error: %s\n", state->GetFileName().c_str(), state->GetLineNumber(), msg);
}

#define YYLOC state->GetFileName(), state->GetLineNumber()
#define YYSTMT(t) new Statement(YYLOC, t)
#define YYFUNC(n) new Function(YYLOC, n)
%}

%pure-parser
%parse-param {ParserState* state}
%lex-param {ParserState* state}

%token END 0
%token _ERROR

%token MINUS PLUS STAR DIV MOD
%token AND OR XOR
%token MINUS_EQ PLUS_EQ MULT_EQ DIV_EQ MOD_EQ
%token AND_EQ OR_EQ XOR_EQ
%token SHIFT_LEFT_EQ
%token SHIFT_RIGHT_EQ
%token INCREMENT DECREMENT
%token LBRACE RBRACE
%token LPAREN RPAREN
%token LBRACKET RBRACKET
%token ASSIGN
%token LESS_THAN LESS_EQUAL EQUAL NOT_EQUAL GREATER_EQUAL GREATER_THAN
%token NOT
%token DOT ARROW
%token QUESTION COLON
%token SEMICOLON COMMA
%token LOGICAL_OR LOGICAL_AND LOGICAL_NOT
%token SHIFT_LEFT SHIFT_RIGHT
%token ELLIPSIS
%token <intval> INT_VAL
%token <floatval> FLOAT_VAL
%token <str> STRING_VAL CHAR_VAL
%token <str> ID TYPE_ID

%token CONST_TOK
%token BOOL_TOK TRUE_VAL FALSE_VAL
%token CHAR_TOK INT_TOK SHORT_TOK LONG_TOK
%token FLOAT_TOK DOUBLE_TOK
%token SIGNED_TOK UNSIGNED_TOK
%token INT8_TOK INT16_TOK INT32_TOK INT64_TOK
%token UINT8_TOK UINT16_TOK UINT32_TOK UINT64_TOK
%token SIZET SSIZET PTRDIFFT
%token STRUCT UNION ENUM
%token RETURN
%token IF ELSE
%token FOR WHILE DO
%token BREAK CONTINUE
%token GOTO
%token SWITCH CASE DEFAULT
%token VOID_TOK
%token MIN MAX ABS
%token ALLOCA MEMCPY MEMSET STRLEN
%token SIZEOF
%token TYPEDEF
%token STATIC_TOK EXTERN_TOK
%token UNDEFINED

%token CDECL_TOK STDCALL_TOK FASTCALL_TOK SUBARCH_TOK NORETURN_TOK PACKED_TOK
%token SYSCALL_TOK SYSCALL2_TOK
%token RDTSC_TOK RDTSC_LOW RDTSC_HIGH
%token NEXT_ARG PREV_ARG
%token BYTESWAP BREAKPOINT

%destructor { free($$); } STRING_VAL CHAR_VAL
%destructor { free($$); } ID TYPE_ID
%destructor { $$->Release(); } var_type return_type primitive_type
%destructor { $$->Release(); } struct_member_list struct_member_list_packed union_member_list union_member_list_packed struct_member
%destructor { $$->Release(); } enum_member_list struct_declaration union_declaration enum_declaration
%destructor { delete $$; } enum_member
%destructor { delete $$; } param_list param_list_nonempty param
%destructor { delete $$; } arg_list arg_list_nonempty
%destructor { delete $$; } id_list
%destructor { $$->Release(); } expression expression_with_comma optional_expression for_initializer
%destructor { $$->Release(); } stmt_list stmt_list_nonempty stmt
%destructor { $$->Release(); } var_declaration initializer initializer_list initializer_list_nonempty
%destructor { delete $$; } var_init_list var_init
%destructor { delete $$; } func_type func_attr_list func_attr_list_nonempty func_attr

%type <type> var_type return_type primitive_type
%type <type> struct_member_list struct_member_list_packed union_member_list union_member_list_packed struct_member
%type <type> enum_member_list struct_declaration union_declaration enum_declaration
%type <enumMember> enum_member
%type <boolean> sign_type
%type <intval> ptr_decorator calling_convention
%type <params> param_list param_list_nonempty param
%type <args> arg_list arg_list_nonempty
%type <ids> id_list
%type <expr> expression expression_with_comma optional_expression for_initializer
%type <expr> stmt_list stmt_list_nonempty stmt
%type <expr> var_declaration initializer initializer_list initializer_list_nonempty
%type <varInit> var_init
%type <varInitList> var_init_list
%type <function> func_type func_attr_list func_attr_list_nonempty func_attr

%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE
%left COMMA
%right ASSIGN PLUS_EQ MINUS_EQ MULT_EQ DIV_EQ MOD_EQ AND_EQ OR_EQ XOR_EQ SHIFT_LEFT_EQ SHIFT_RIGHT_EQ
%right QUESTION COLON
%left LOGICAL_OR
%left LOGICAL_AND
%left OR
%left XOR
%left AND
%left EQUAL NOT_EQUAL
%left LESS_THAN LESS_EQUAL GREATER_EQUAL GREATER_THAN
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left STAR DIV MOD
%right INCREMENT DECREMENT UNARY_SIGN NOT LOGICAL_NOT CAST DEREF ADDRESS_OF SIZEOF
%left POST_INCREMENT POST_DECREMENT LBRACKET LPAREN DOT ARROW

%%

input:	toplevel_stmt_list END
|
;

toplevel_stmt_list:	toplevel_stmt_list toplevel_stmt
		|	toplevel_stmt
		;

toplevel_stmt:	var_declaration SEMICOLON  { state->AddInitExpression($1); $1->Release(); }
	|	func_prototype
	|	func_declaration
	|	EXTERN_TOK var_type var_init_list
		{
			for (vector<VarInitInfo>::iterator i = $3->begin(); i != $3->end(); i++)
			{
				if (i->initialized)
					Code_error(state, "attempting to initialize variable marked 'extern'");
				Expr* expr = state->DeclareVariable($2, *i, false, true);
				delete expr;
			}
			$2->Release();
			delete $3;
		}
	|	EXTERN_TOK return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN
		{
			Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$9), $5 - 1);
			VarInitInfo info;
			info.name = $6;
			info.initialized = false;
			info.array = false;
			info.location = state->GetLocation();
			Expr* expr = state->DeclareVariable(type, info, false, true);
			delete expr;
			$2->Release();
			free($6);
			delete $9;
		}
	|	EXTERN_TOK return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN
		{
			Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$12), $5 - 1);
			VarInitInfo info;
			info.name = $6;
			info.initialized = false;
			info.array = true;
			info.elements = $8->ComputeIntegerValue(state);
			info.location = state->GetLocation();
			Expr* expr = state->DeclareVariable(type, info, false, true);
			delete expr;
			$2->Release();
			free($6);
			$8->Release();
			delete $12;
		}
	|	TYPEDEF var_type ID SEMICOLON  { state->DefineType($3, $2); $2->Release(); free($3); }
	|	TYPEDEF struct_declaration ID SEMICOLON  { state->DefineType($3, $2); $2->Release(); free($3); }
	|	TYPEDEF union_declaration ID SEMICOLON  { state->DefineType($3, $2); $2->Release(); free($3); }
	|	TYPEDEF enum_declaration ID SEMICOLON  { state->DefineType($3, $2); $2->Release(); free($3); }
	|	TYPEDEF var_type ID LBRACKET expression RBRACKET SEMICOLON
		{
			Type* type = Type::ArrayType($2, (size_t)$5->ComputeIntegerValue(state));
			state->DefineType($3, type);
			$2->Release();
			free($3);
			$5->Release();
		}
	|	TYPEDEF struct_declaration ID LBRACKET expression RBRACKET SEMICOLON
		{
			Type* type = Type::ArrayType($2, (size_t)$5->ComputeIntegerValue(state));
			state->DefineType($3, type);
			$2->Release();
			free($3);
			$5->Release();
		}
	|	TYPEDEF union_declaration ID LBRACKET expression RBRACKET SEMICOLON
		{
			Type* type = Type::ArrayType($2, (size_t)$5->ComputeIntegerValue(state));
			state->DefineType($3, type);
			$2->Release();
			free($3);
			$5->Release();
		}
	|	TYPEDEF enum_declaration ID LBRACKET expression RBRACKET SEMICOLON
		{
			Type* type = Type::ArrayType($2, (size_t)$5->ComputeIntegerValue(state));
			state->DefineType($3, type);
			$2->Release();
			free($3);
			$5->Release();
		}
	|	TYPEDEF return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN SEMICOLON
		{
			Type* type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$9), $5 - 1);
			state->DefineType($6, type);
			$2->Release();
			free($6);
			delete $9;
		}
	|	TYPEDEF return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN SEMICOLON
		{
			Type* type = Type::ArrayType(Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$12), $5 - 1),
				$8->ComputeIntegerValue(state));
			state->DefineType($6, type);
			$2->Release();
			free($6);
			$8->Release();
			delete $12;
		}
	|	STRUCT ID SEMICOLON  { state->DefineStructType($2, Type::StructType(new Struct(true))); free($2); }
	|	struct_declaration
	|	UNION ID SEMICOLON  { state->DefineUnionType($2, Type::StructType(new Struct(false))); free($2); }
	|	union_declaration
	|	ENUM ID SEMICOLON  { state->DefineEnumType($2, Type::EnumType(new Enum())); free($2); }
	|	enum_declaration
	|	SEMICOLON
	;

struct_declaration: STRUCT ID LBRACE { state->DefineStructType($2, Type::StructType(new Struct(true))); } struct_member_list RBRACE 
					{
						$$ = $5;
						$$->GetStruct()->Complete();
						state->DefineStructType($2, $5);
						free($2);
					}
				|	STRUCT ID PACKED_TOK LBRACE { state->DefineStructType($2, Type::StructType(new Struct(true, true))); } struct_member_list_packed RBRACE 
					{
						$$ = $6;
						$$->GetStruct()->Complete();
						state->DefineStructType($2, $6);
						free($2);
					}
				;

union_declaration:	UNION ID LBRACE { state->DefineUnionType($2, Type::StructType(new Struct(false))); } union_member_list RBRACE
					{
						$$ = $5;
						$$->GetStruct()->Complete();
						state->DefineUnionType($2, $5);
						free($2);
					}
				|	UNION ID PACKED_TOK LBRACE { state->DefineUnionType($2, Type::StructType(new Struct(false, true))); } union_member_list_packed RBRACE
					{
						$$ = $6;
						$$->GetStruct()->Complete();
						state->DefineUnionType($2, $6);
						free($2);
					}
				;

enum_declaration:	ENUM ID LBRACE enum_member_list RBRACE
					{
						$$ = $4;
						$$->GetEnum()->Complete();
						state->DefineEnumType($2, $4);
						free($2);
					}
				;

var_declaration:	var_type var_init_list
			{
				$$ = state->BasicExpr(EXPR_SEQUENCE);
				$$->AddRef();
				for (vector<VarInitInfo>::iterator i = $2->begin(); i != $2->end(); i++)
				{
					Expr* expr = state->DeclareVariable($1, *i, false, false);
					if (expr->GetClass() == EXPR_SEQUENCE)
						$$->CopyChildren(expr);
					else
						$$->AddChild(expr);
				}
				$1->Release();
				delete $2;
			}
		|	return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($1, (CallingConvention)$3, *$8), $4 - 1);
				VarInitInfo info;
				info.name = $5;
				info.initialized = false;
				info.array = false;
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, false, false);
				$$->AddRef();
				$1->Release();
				free($5);
				delete $8;
			}
		|	return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($1, (CallingConvention)$3, *$11), $4 - 1);
				VarInitInfo info;
				info.name = $5;
				info.initialized = false;
				info.array = true;
				info.elements = $7->ComputeIntegerValue(state);
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, false, false);
				$$->AddRef();
				$1->Release();
				free($5);
				$7->Release();
				delete $11;
			}
		|	return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN ASSIGN expression
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($1, (CallingConvention)$3, *$8), $4 - 1);
				VarInitInfo info;
				info.name = $5;
				info.initialized = true;
				info.array = false;
				info.value = $11;
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, false, false);
				$$->AddRef();
				$1->Release();
				free($5);
				delete $8;
				$11->Release();
			}
		|	return_type LPAREN calling_convention ptr_decorator ID LBRACKET RBRACKET RPAREN LPAREN param_list RPAREN ASSIGN initializer
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($1, (CallingConvention)$3, *$10), $4 - 1);
				VarInitInfo info;
				info.name = $5;
				info.initialized = true;
				info.array = true;
				info.value = $13;
				info.elements = 0;
				info.location = state->GetLocation();

				if ($13->GetClass() == EXPR_INITIALIZER)
					info.elements = $13->GetChildren().size();
				else
					Code_error(state, "invalid array initializer");

				$$ = state->DeclareVariable(type, info, false, false);
				$$->AddRef();
				$1->Release();
				free($5);
				delete $10;
				$13->Release();
			}
		|	return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN ASSIGN initializer
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($1, (CallingConvention)$3, *$11), $4 - 1);
				VarInitInfo info;
				info.name = $5;
				info.initialized = true;
				info.array = true;
				info.value = $14;
				info.elements = $7->ComputeIntegerValue(state);
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, false, false);
				$$->AddRef();
				$1->Release();
				free($5);
				$7->Release();
				delete $11;
				$14->Release();
			}
		|	STATIC_TOK var_type var_init_list
			{
				$$ = state->BasicExpr(EXPR_SEQUENCE);
				$$->AddRef();
				for (vector<VarInitInfo>::iterator i = $3->begin(); i != $3->end(); i++)
				{
					Expr* expr = state->DeclareVariable($2, *i, true, false);
					if (expr->GetClass() == EXPR_SEQUENCE)
						$$->CopyChildren(expr);
					else
						$$->AddChild(expr);
				}
				$2->Release();
				delete $3;
			}
		|	STATIC_TOK return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$9), $5 - 1);
				VarInitInfo info;
				info.name = $6;
				info.initialized = false;
				info.array = false;
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, true, false);
				$$->AddRef();
				$2->Release();
				free($6);
				delete $9;
			}
		|	STATIC_TOK return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$12), $5 - 1);
				VarInitInfo info;
				info.name = $6;
				info.initialized = false;
				info.array = true;
				info.elements = $8->ComputeIntegerValue(state);
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, true, false);
				$$->AddRef();
				$2->Release();
				free($6);
				$8->Release();
				delete $12;
			}
		|	STATIC_TOK return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN ASSIGN expression
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$9), $5 - 1);
				VarInitInfo info;
				info.name = $6;
				info.initialized = true;
				info.array = false;
				info.value = $12;
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, true, false);
				$$->AddRef();
				$2->Release();
				free($6);
				delete $9;
				$12->Release();
			}
		|	STATIC_TOK return_type LPAREN calling_convention ptr_decorator ID LBRACKET RBRACKET RPAREN LPAREN param_list RPAREN ASSIGN initializer
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$11), $5 - 1);
				VarInitInfo info;
				info.name = $6;
				info.initialized = true;
				info.array = true;
				info.value = $14;
				info.elements = 0;
				info.location = state->GetLocation();

				if ($14->GetClass() == EXPR_INITIALIZER)
					info.elements = $14->GetChildren().size();
				else
					Code_error(state, "invalid array initializer");

				$$ = state->DeclareVariable(type, info, true, false);
				$$->AddRef();
				$2->Release();
				free($6);
				delete $11;
				$14->Release();
			}
		|	STATIC_TOK return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN ASSIGN initializer
			{
				Ref<Type> type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$12), $5 - 1);
				VarInitInfo info;
				info.name = $6;
				info.initialized = true;
				info.array = true;
				info.value = $15;
				info.elements = $8->ComputeIntegerValue(state);
				info.location = state->GetLocation();
				$$ = state->DeclareVariable(type, info, true, false);
				$$->AddRef();
				$2->Release();
				free($6);
				$8->Release();
				delete $12;
				$15->Release();
			}
		;

var_init_list:	var_init_list COMMA var_init  { $$ = $1; $$->push_back(*$3); delete $3; }
	|	var_init  { $$ = new vector<VarInitInfo>(); $$->push_back(*$1); delete $1; }
	;

var_init:	ID
		{
			$$ = new VarInitInfo;
			$$->name = $1;
			$$->initialized = false;
			$$->array = false;
			$$->location = state->GetLocation();
			free($1);
		}
	|	ID ASSIGN initializer
		{
			$$ = new VarInitInfo;
			$$->name = $1;
			$$->initialized = true;
			$$->array = false;
			$$->value = $3;
			$$->location = state->GetLocation();
			free($1);
			$3->Release();
		}
	|	ID LBRACKET RBRACKET ASSIGN initializer
		{
			$$ = new VarInitInfo;
			$$->name = $1;
			$$->initialized = true;
			$$->array = true;
			$$->value = $5;
			$$->elements = 0;
			$$->location = state->GetLocation();

			if ($5->GetClass() == EXPR_INITIALIZER)
				$$->elements = $5->GetChildren().size();
			else if ($5->GetClass() == EXPR_STRING)
				$$->elements = $5->GetStringValue().size();
			else
				Code_error(state, "invalid array initializer");

			free($1);
			$5->Release();
		}
	|	ID LBRACKET expression RBRACKET
		{
			$$ = new VarInitInfo;
			$$->name = $1;
			$$->initialized = false;
			$$->array = true;
			$$->elements = (size_t)$3->ComputeIntegerValue(state);
			$$->location = state->GetLocation();
			free($1);
			$3->Release();
		}
	|	ID LBRACKET expression RBRACKET ASSIGN initializer
		{
			$$ = new VarInitInfo;
			$$->name = $1;
			$$->initialized = true;
			$$->array = true;
			$$->value = $6;
			$$->elements = (size_t)$3->ComputeIntegerValue(state);
			$$->location = state->GetLocation();
			free($1);
			$3->Release();
			$6->Release();
		}
	;

initializer:	expression  { $$ = $1; }
	|	LBRACE initializer_list RBRACE  { $$ = $2; }
	;

initializer_list:	initializer_list_nonempty  { $$ = $1; }
		|	{ $$ = state->BasicExpr(EXPR_INITIALIZER); $$->AddRef(); }
		;

initializer_list_nonempty:	initializer_list_nonempty COMMA initializer  { $$ = $1; $$->AddChild($3); $3->Release(); }
			|	initializer  { $$ = state->UnaryExpr(EXPR_INITIALIZER, $1); $$->AddRef(); $1->Release(); }
			;

func_prototype:	func_type func_attr_list SEMICOLON
		{
			if ($2)
			{
				$1->CombineFunctionAttributes(*$2);
				delete $2;
			}
			state->DefineFunctionPrototype(*$1, false);
			delete $1;
		}
	|	STATIC_TOK func_type func_attr_list SEMICOLON
		{
			if ($3)
			{
				$2->CombineFunctionAttributes(*$3);
				delete $3;
			}
			state->DefineFunctionPrototype(*$2, true);
			delete $2;
		}
	|	EXTERN_TOK func_type func_attr_list SEMICOLON
		{
			if ($3)
			{
				$2->CombineFunctionAttributes(*$3);
				delete $3;
			}
			state->DefineFunctionPrototype(*$2, false);
			delete $2;
		}
	;

func_declaration:	func_type func_attr_list LBRACE { state->BeginFunctionScope(*$1); } stmt_list RBRACE
			{
				if ($2)
				{
					$1->CombineFunctionAttributes(*$2);
					delete $2;
				}
				state->DefineFunction(*$1, $5, false);
				state->PopScope();
				delete $1;
				$5->Release();
			}
		|	STATIC_TOK func_type func_attr_list LBRACE { state->BeginFunctionScope(*$2); } stmt_list RBRACE
			{
				if ($3)
				{
					$2->CombineFunctionAttributes(*$3);
					delete $3;
				}
				state->DefineFunction(*$2, $6, true);
				state->PopScope();
				delete $2;
				$6->Release();
			}
		;

func_attr_list:	func_attr_list_nonempty  { $$ = $1; }
	|	{ $$ = NULL; }
	;

func_attr_list_nonempty:	func_attr_list_nonempty func_attr
				{
					$$ = $1;
					$$->CombineFunctionAttributes(*$2);
					delete $2;
				}
			|	func_attr  { $$ = $1; }
			;

func_attr:	SUBARCH_TOK LPAREN ID RPAREN
		{
			$$ = new FunctionInfo;
			$$->callingConvention = CALLING_CONVENTION_DEFAULT;
			$$->noReturn = false;
			if (!strcmp($3, "x86"))
				$$->subarch = SUBARCH_X86;
			else if (!strcmp($3, "i386"))
				$$->subarch = SUBARCH_X86;
			else if (!strcmp($3, "x64"))
				$$->subarch = SUBARCH_X64;
			else if (!strcmp($3, "x86_64"))
				$$->subarch = SUBARCH_X64;
			else if (!strcmp($3, "amd64"))
				$$->subarch = SUBARCH_X64;
			else
				Code_error(state, "invalid subarchitecture");
			free($3);
		}
	|	NORETURN_TOK
		{
			$$ = new FunctionInfo;
			$$->callingConvention = CALLING_CONVENTION_DEFAULT;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = true;
		}
	;

func_type:	var_type ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = $1;
			$$->callingConvention = CALLING_CONVENTION_DEFAULT;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $2;
			$$->params = *$4;
			$$->location = state->GetLocation();
			$1->Release();
			free($2);
			delete $4;
		}
	|	var_type CDECL_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = $1;
			$$->callingConvention = CALLING_CONVENTION_CDECL;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $3;
			$$->params = *$5;
			$$->location = state->GetLocation();
			$1->Release();
			free($3);
			delete $5;
		}
	|	var_type STDCALL_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = $1;
			$$->callingConvention = CALLING_CONVENTION_STDCALL;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $3;
			$$->params = *$5;
			$$->location = state->GetLocation();
			$1->Release();
			free($3);
			delete $5;
		}
	|	var_type FASTCALL_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = $1;
			$$->callingConvention = CALLING_CONVENTION_FASTCALL;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $3;
			$$->params = *$5;
			$$->location = state->GetLocation();
			$1->Release();
			free($3);
			delete $5;
		}
	|	VOID_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = Type::VoidType();
			$$->callingConvention = CALLING_CONVENTION_DEFAULT;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $2;
			$$->params = *$4;
			$$->location = state->GetLocation();
			free($2);
			delete $4;
		}
	|	VOID_TOK CDECL_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = Type::VoidType();
			$$->callingConvention = CALLING_CONVENTION_CDECL;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $3;
			$$->params = *$5;
			$$->location = state->GetLocation();
			free($3);
			delete $5;
		}
	|	VOID_TOK STDCALL_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = Type::VoidType();
			$$->callingConvention = CALLING_CONVENTION_STDCALL;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $3;
			$$->params = *$5;
			$$->location = state->GetLocation();
			free($3);
			delete $5;
		}
	|	VOID_TOK FASTCALL_TOK ID LPAREN param_list RPAREN
		{
			$$ = new FunctionInfo;
			$$->returnValue = Type::VoidType();
			$$->callingConvention = CALLING_CONVENTION_FASTCALL;
			$$->subarch = SUBARCH_DEFAULT;
			$$->noReturn = false;
			$$->name = $3;
			$$->params = *$5;
			$$->location = state->GetLocation();
			free($3);
			delete $5;
		}
	;

param_list:	param_list_nonempty  { $$ = $1; }
	|	VOID_TOK  { $$ = new vector< pair< Ref<Type>, string > >(); }
	|	{ $$ = new vector< pair< Ref<Type>, string > >(); }
	;

param_list_nonempty:	param_list_nonempty COMMA param
			{
				$$ = $1;
				if (($$->size() > 0) && ((*$$)[$$->size() - 1].second == "..."))
				{
					Code_error(state, "variable arguments must be last");
					$$->erase($$->end() - 1);
				}
				$$->insert($$->end(), $3->begin(), $3->end());
				delete $3;
			}
		|	param  { $$ = $1; }
		;

param:	var_type ID
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >($1, $2));
		$1->Release();
		free($2);
	}
|	var_type ID LBRACKET RBRACKET
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::PointerType($1, 1), $2));
		$1->Release();
		free($2);
	}
|	var_type ID LBRACKET expression RBRACKET
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::ArrayType($1, $4->ComputeIntegerValue(state)), $2));
		$1->Release();
		free($2);
		$4->Release();
	}
|	var_type
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >($1, ""));
		$1->Release();
	}
|	var_type LBRACKET RBRACKET
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::PointerType($1, 1), ""));
		$1->Release();
	}
|	var_type LBRACKET expression RBRACKET
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::ArrayType($1, $3->ComputeIntegerValue(state)), ""));
		$1->Release();
		$3->Release();
	}
|	return_type LPAREN calling_convention ptr_decorator RPAREN LPAREN param_list RPAREN
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::PointerType(Type::FunctionType($1,
			(CallingConvention)$3, *$7), $4 - 1), ""));
		$1->Release();
		delete $7;
	}
|	return_type LPAREN calling_convention ptr_decorator LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::ArrayType(Type::PointerType(Type::FunctionType($1,
			(CallingConvention)$3, *$10), $4 - 1), (size_t)$6->ComputeIntegerValue(state)), ""));
		$1->Release();
		$6->Release();
		delete $10;
	}
|	return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::PointerType(Type::FunctionType($1,
			(CallingConvention)$3, *$8), $4 - 1), $5));
		$1->Release();
		free($5);
		delete $8;
	}
|	return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(Type::ArrayType(Type::PointerType(Type::FunctionType($1,
			(CallingConvention)$3, *$11), $4 - 1), (size_t)$7->ComputeIntegerValue(state)), $5));
		$1->Release();
		free($5);
		$7->Release();
		delete $11;
	}
|	ELLIPSIS
	{
		$$ = new vector< pair< Ref<Type>, string > >();
		$$->push_back(pair< Ref<Type>, string >(NULL, "..."));
	}
;

return_type:	var_type  { $$ = $1; }
	|	VOID_TOK  { $$ = Type::VoidType(); $$->AddRef(); }
	;

var_type:	primitive_type  { $$ = $1; }
	|	CONST_TOK primitive_type  { $$ = new Type($2); $$->AddRef(); $$->SetConst(true); $2->Release(); }
	|	primitive_type ptr_decorator { $$ = Type::PointerType($1, $2); $$->AddRef(); }
	|	VOID_TOK ptr_decorator { $$ = Type::PointerType(Type::VoidType(), $2); $$->AddRef(); }
	|	CONST_TOK primitive_type ptr_decorator { $$ = Type::PointerType($2, $3); $$->AddRef(); $$->SetConst(true); }
	|	CONST_TOK VOID_TOK ptr_decorator { $$ = Type::PointerType(Type::VoidType(), $3); $$->AddRef(); $$->SetConst(true); }
	|	STRUCT LBRACE struct_member_list RBRACE { $$ = $3; $$->GetStruct()->Complete(); }
	|	STRUCT PACKED_TOK LBRACE struct_member_list_packed RBRACE { $$ = $4; $$->GetStruct()->Complete(); }
	|	UNION LBRACE union_member_list RBRACE { $$ = $3; $$->GetStruct()->Complete(); }
	|	UNION PACKED_TOK LBRACE union_member_list_packed RBRACE { $$ = $4; $$->GetStruct()->Complete(); }
	|	ENUM LBRACE enum_member_list RBRACE  { $$ = $3; $$->GetEnum()->Complete(); }
	;

primitive_type:	BOOL_TOK  { $$ = Type::BoolType(); $$->AddRef(); }
	|	sign_type CHAR_TOK  { $$ = Type::IntType(1, $1); $$->AddRef(); }
	|	sign_type INT_TOK  { $$ = Type::IntType(4, $1); $$->AddRef(); }
	|	sign_type SHORT_TOK  { $$ = Type::IntType(2, $1); $$->AddRef(); }
	|	sign_type SHORT_TOK INT_TOK  { $$ = Type::IntType(2, $1); $$->AddRef(); }
	|	sign_type LONG_TOK  { $$ = Type::IntType(GetTargetPointerSize(), $1); $$->AddRef(); }
	|	sign_type LONG_TOK INT_TOK  { $$ = Type::IntType(GetTargetPointerSize(), $1); $$->AddRef(); }
	|	sign_type LONG_TOK LONG_TOK  { $$ = Type::IntType(8, $1); $$->AddRef(); }
	|	sign_type LONG_TOK LONG_TOK INT_TOK  { $$ = Type::IntType(8, $1); $$->AddRef(); }
	|	FLOAT_TOK  { $$ = Type::FloatType(4); $$->AddRef(); }
	|	DOUBLE_TOK  { $$ = Type::FloatType(8); $$->AddRef(); }
	|	INT8_TOK  { $$ = Type::IntType(1, true); $$->AddRef(); }
	|	INT16_TOK  { $$ = Type::IntType(2, true); $$->AddRef(); }
	|	INT32_TOK  { $$ = Type::IntType(4, true); $$->AddRef(); }
	|	INT64_TOK  { $$ = Type::IntType(8, true); $$->AddRef(); }
	|	UINT8_TOK  { $$ = Type::IntType(1, false); $$->AddRef(); }
	|	UINT16_TOK  { $$ = Type::IntType(2, false); $$->AddRef(); }
	|	UINT32_TOK  { $$ = Type::IntType(4, false); $$->AddRef(); }
	|	UINT64_TOK  { $$ = Type::IntType(8, false); $$->AddRef(); }
	|	SIZET  { $$ = Type::IntType(GetTargetPointerSize(), false); $$->AddRef(); }
	|	SSIZET  { $$ = Type::IntType(GetTargetPointerSize(), true); $$->AddRef(); }
	|	PTRDIFFT  { $$ = Type::IntType(GetTargetPointerSize(), true); $$->AddRef(); }
	|	STRUCT ID  { $$ = state->GetStructType($2); $$->AddRef(); free($2); }
	|	UNION ID  { $$ = state->GetUnionType($2); $$->AddRef(); free($2); }
	|	ENUM ID  { $$ = state->GetEnumType($2); $$->AddRef(); free($2); }
	|	TYPE_ID  { $$ = state->GetType($1); $$->AddRef(); free($1); }
	;

sign_type:	SIGNED_TOK  { $$ = true; }
	|	UNSIGNED_TOK  { $$ = false; }
	|	{ $$ = true; }
	;

calling_convention:	CDECL_TOK  { $$ = CALLING_CONVENTION_CDECL; }
		|	STDCALL_TOK  { $$ = CALLING_CONVENTION_STDCALL; }
		|	FASTCALL_TOK  { $$ = CALLING_CONVENTION_FASTCALL; }
		|	{ $$ = CALLING_CONVENTION_DEFAULT; }
		;

ptr_decorator:	ptr_decorator STAR  { $$ = $1 + 1; }
	|	STAR  { $$ = 1; }
	;

struct_member_list:	struct_member_list struct_member
			{
				$$ = $1;
				Struct* s = $1->GetStruct();
				s->CopyMembers(state, $2->GetStruct());
				$2->Release();
			}
		|	struct_member
			{
				Struct* s = new Struct(false);
				s->CopyMembers(state, $1->GetStruct());
				$$ = Type::StructType(s);
				$$->AddRef();
				$1->Release();
			}
		;

struct_member_list_packed:	struct_member_list_packed struct_member
					{
						$$ = $1;
						Struct* s = $1->GetStruct();
						s->CopyMembers(state, $2->GetStruct());
						$2->Release();
					}
				|	struct_member
					{
						Struct* s = new Struct(false, true);
						s->CopyMembers(state, $1->GetStruct());
						$$ = Type::StructType(s);
						$$->AddRef();
						$1->Release();
					}
				;

union_member_list:	union_member_list struct_member
			{
				$$ = $1;
				Struct* s = $1->GetStruct();
				s->CopyMembers(state, $2->GetStruct());
				$2->Release();
			}
		|	struct_member
			{
				Struct* s = new Struct(true);
				s->CopyMembers(state, $1->GetStruct());
				$$ = Type::StructType(s);
				$$->AddRef();
				$1->Release();
			}
		;

union_member_list_packed:	union_member_list_packed struct_member
					{
						$$ = $1;
						Struct* s = $1->GetStruct();
						s->CopyMembers(state, $2->GetStruct());
						$2->Release();
					}
				|	struct_member
					{
						Struct* s = new Struct(true, true);
						s->CopyMembers(state, $1->GetStruct());
						$$ = Type::StructType(s);
						$$->AddRef();
						$1->Release();
					}
				;

struct_member:	var_type id_list SEMICOLON
		{
			Struct* s = new Struct();
			for (std::vector<std::string>::iterator i = $2->begin(); i != $2->end(); i++)
				s->AddMember(NULL, $1, *i);
			$$ = Type::StructType(s);
			$$->AddRef();
			$1->Release();
			delete $2;
		}
	|	var_type ID COLON INT_VAL SEMICOLON
		{
			Code_error(state, "bit fields not yet supported");
			$$ = Type::StructMemberType($1, $2);
			$$->AddRef();
			$1->Release();
			free($2);
		}
	|	var_type ID LBRACKET expression RBRACKET SEMICOLON
		{
			$$ = Type::StructMemberType(Type::ArrayType($1, (size_t)$4->ComputeIntegerValue(state)), $2);
			$$->AddRef();
			$1->Release();
			free($2);
			$4->Release();
		}
	|	return_type LPAREN calling_convention ptr_decorator ID RPAREN LPAREN param_list RPAREN SEMICOLON
		{
			$$ = Type::StructMemberType(Type::PointerType(Type::FunctionType($1, (CallingConvention)$3, *$8), $4 - 1), $5);
			$$->AddRef();
			$1->Release();
			free($5);
			delete $8;
		}
	|	return_type LPAREN calling_convention ptr_decorator ID LBRACKET expression RBRACKET RPAREN LPAREN param_list RPAREN SEMICOLON
		{
			$$ = Type::StructMemberType(Type::ArrayType(Type::PointerType(Type::FunctionType($1,
				(CallingConvention)$3, *$11), $4 - 1), (size_t)$7->ComputeIntegerValue(state)), $5);
			$$->AddRef();
			$1->Release();
			free($5);
			$7->Release();
			delete $11;
		}
	|	STRUCT LBRACE struct_member_list RBRACE SEMICOLON  { $$ = $3; }
	|	STRUCT PACKED_TOK LBRACE struct_member_list_packed RBRACE SEMICOLON  { $$ = $4; }
	|	UNION LBRACE union_member_list RBRACE SEMICOLON  { $$ = $3; }
	|	UNION PACKED_TOK LBRACE union_member_list_packed RBRACE SEMICOLON  { $$ = $4; }
	;

enum_member_list:	enum_member_list COMMA enum_member
			{
				$$ = $1;
				Enum* e = $1->GetEnum();
				uint32_t value = $3->isDefault ? e->GetNextValue() : $3->value;
				e->AddMember($3->name, value);
				state->AddEnumMember($3->name, value);
				delete $3;
			}
		|	enum_member
			{
				Enum* e = new Enum();
				uint32_t value = $1->isDefault ? e->GetNextValue() : $1->value;
				e->AddMember($1->name, value);
				state->AddEnumMember($1->name, value);
				$$ = Type::EnumType(e);
				$$->AddRef();
				delete $1;
			}
		;

enum_member:	ID  { $$ = new EnumMember; $$->name = $1; $$->isDefault = true; free($1); }
	|	ID ASSIGN INT_VAL  { $$ = new EnumMember; $$->name = $1; $$->isDefault = false; $$->value = $3; free($1); }
	;

stmt_list:	stmt_list_nonempty
		{
			if (($1->GetClass() == EXPR_SEQUENCE) && ($1->GetChildren().size() == 1))
			{
				$$ = $1->GetChildren()[0];
				$$->AddRef();
				$1->Release();
			}
			else
			{
				$$ = $1;
			}
		}
	|	{ $$ = state->BasicExpr(EXPR_SEQUENCE); $$->AddRef(); }
	;

stmt_list_nonempty:	stmt_list_nonempty stmt
			{
				$$ = $1;
				if ($2->GetClass() == EXPR_SEQUENCE)
					$$->CopyChildren($2);
				else
					$$->AddChild($2);
				$2->Release();
			}
		|	stmt
			{
				if ($1->GetClass() == EXPR_SEQUENCE)
					$$ = $1;
				else
				{
					$$ = state->UnaryExpr(EXPR_SEQUENCE, $1);
					$$->AddRef();
					$1->Release();
				}
			}
		;

stmt:	SEMICOLON  { $$ = state->BasicExpr(EXPR_SEQUENCE); $$->AddRef(); }
|	var_declaration SEMICOLON  { $$ = $1; }
|	expression_with_comma SEMICOLON  { $$ = $1; }
|	LPAREN VOID_TOK RPAREN expression SEMICOLON  { $$ = $4; }
|	LBRACE { state->PushScope(); } stmt_list RBRACE  { state->PopScope(); $$ = $3; }
|	IF LPAREN expression_with_comma RPAREN stmt %prec IF_WITHOUT_ELSE
	{
		$$ = state->IfExpr($3->ConvertToBool(state), $5);
		$$->AddRef();
		$3->Release();
		$5->Release();
	}
|	IF LPAREN expression_with_comma RPAREN stmt ELSE stmt
	{
		$$ = state->IfElseExpr($3->ConvertToBool(state), $5, $7);
		$$->AddRef();
		$3->Release();
		$5->Release();
		$7->Release();
	}
|	FOR LPAREN { state->PushScope(); } for_initializer SEMICOLON optional_expression SEMICOLON optional_expression RPAREN stmt
	{
		if (($6->GetClass() == EXPR_SEQUENCE) && ($6->GetChildren().size() == 0))
			$$ = state->ForExpr($4, state->BoolExpr(true), $8, $10);
		else
			$$ = state->ForExpr($4, $6->ConvertToBool(state), $8, $10);
		$$->AddRef();
		$4->Release();
		$6->Release();
		$8->Release();
		$10->Release();
		state->PopScope();
	}
|	WHILE LPAREN expression_with_comma RPAREN stmt
	{
		$$ = state->WhileExpr($3->ConvertToBool(state), $5);
		$$->AddRef();
		$3->Release();
		$5->Release();
	}
|	DO stmt WHILE LPAREN expression_with_comma RPAREN SEMICOLON
	{
		$$ = state->DoWhileExpr($5->ConvertToBool(state), $2);
		$$->AddRef();
		$2->Release();
		$5->Release();
	}
|	RETURN SEMICOLON  { $$ = state->BasicExpr(EXPR_RETURN_VOID); $$->AddRef(); }
|	RETURN expression SEMICOLON  { $$ = state->UnaryExpr(EXPR_RETURN, $2); $$->AddRef(); $2->Release(); }
|	BREAK SEMICOLON  { $$ = state->BasicExpr(EXPR_BREAK); $$->AddRef(); }
|	CONTINUE SEMICOLON  { $$ = state->BasicExpr(EXPR_CONTINUE); $$->AddRef(); }
|	ID COLON  { $$ = state->LabelExpr($1); $$->AddRef(); free($1); }
|	GOTO ID SEMICOLON  { $$ = state->GotoLabelExpr($2); $$->AddRef(); free($2); }
|	GOTO STAR expression SEMICOLON  { $$ = state->UnaryExpr(EXPR_COMPUTED_GOTO, $3); $$->AddRef(); $3->Release(); }
|	SWITCH LPAREN expression RPAREN LBRACE stmt_list RBRACE
	{
		$$ = state->BinaryExpr(EXPR_SWITCH, $3, $6);
		$$->AddRef();
		$3->Release();
		$6->Release();
	}
|	CASE expression COLON
	{
		$$ = state->BasicExpr(EXPR_CASE);
		$$->SetIntValue($2->ComputeIntegerValue(state));
		$$->AddRef();
		$2->Release();
	}
|	DEFAULT COLON  { $$ = state->BasicExpr(EXPR_DEFAULT); $$->AddRef(); }
;

for_initializer:	var_declaration  { $$ = $1; }
		|	expression_with_comma  { $$ = $1; }
		|	{ $$ = state->BasicExpr(EXPR_SEQUENCE); $$->AddRef(); }
		;

optional_expression:	expression_with_comma  { $$ = $1; }
		|	{ $$ = state->BasicExpr(EXPR_SEQUENCE); $$->AddRef(); }
		;

expression:	INT_VAL  { $$ = state->IntExpr($1); $$->AddRef(); }
	|	FLOAT_VAL  { $$ = state->FloatExpr($1); $$->AddRef(); }
	|	CHAR_VAL  { $$ = state->IntExpr(state->CharStringToValue(ParserState::ProcessEscapedString($1))); $$->AddRef(); free($1); }
	|	STRING_VAL  { $$ = state->StringExpr(ParserState::ProcessEscapedString($1)); $$->AddRef(); free($1); }
	|	TRUE_VAL  { $$ = state->BoolExpr(true); $$->AddRef(); }
	|	FALSE_VAL  { $$ = state->BoolExpr(false); $$->AddRef(); }
	|	ID  { $$ = state->ResolveIdentifierExpr($1); $$->AddRef(); free($1); }
	|	UNDEFINED  { $$ = state->BasicExpr(EXPR_UNDEFINED); $$->AddRef(); }
	|	expression PLUS expression  { $$ = state->BinaryExpr(EXPR_PLUS, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression MINUS expression { $$ = state->BinaryExpr(EXPR_MINUS, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression STAR expression  { $$ = state->BinaryExpr(EXPR_MULT, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression DIV expression  { $$ = state->BinaryExpr(EXPR_DIV, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression MOD expression  { $$ = state->BinaryExpr(EXPR_MOD, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression AND expression  { $$ = state->BinaryExpr(EXPR_AND, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression OR expression  { $$ = state->BinaryExpr(EXPR_OR, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression XOR expression  { $$ = state->BinaryExpr(EXPR_XOR, $1, $3); $$->AddRef(); $1->Release(); $3->Release(); }
	|	expression SHIFT_LEFT expression
		{
			$$ = state->BinaryExpr(EXPR_SHIFT_LEFT, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression SHIFT_RIGHT expression
		{
			$$ = state->BinaryExpr(EXPR_SHIFT_RIGHT, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression LOGICAL_AND expression
		{
			$$ = state->BinaryExpr(EXPR_LOGICAL_AND, $1->ConvertToBool(state), $3->ConvertToBool(state));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression LOGICAL_OR expression
		{
			$$ = state->BinaryExpr(EXPR_LOGICAL_OR, $1->ConvertToBool(state), $3->ConvertToBool(state));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression LESS_THAN expression
		{
			$$ = state->BinaryExpr(EXPR_LESS_THAN, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression LESS_EQUAL expression
		{
			$$ = state->BinaryExpr(EXPR_LESS_EQUAL, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression EQUAL expression
		{
			$$ = state->BinaryExpr(EXPR_EQUAL, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression NOT_EQUAL expression
		{
			$$ = state->BinaryExpr(EXPR_NOT_EQUAL, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression GREATER_EQUAL expression
		{
			$$ = state->BinaryExpr(EXPR_GREATER_EQUAL, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression GREATER_THAN expression
		{
			$$ = state->BinaryExpr(EXPR_GREATER_THAN, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression QUESTION expression COLON expression
		{
			$$ = state->IfElseValueExpr($1->ConvertToBool(state), $3, $5);
			$$->AddRef();
			$1->Release();
			$3->Release();
			$5->Release();
		}
	|	expression DOT ID
		{
			if ($1->GetClass() == EXPR_ARRAY_INDEX)
				$$ = state->ArrowExpr(state->UnaryExpr(EXPR_ADDRESS_OF, $1), $3);
			else
				$$ = state->DotExpr($1, $3);
			$$->AddRef();
			$1->Release();
			free($3);
		}
	|	expression ARROW ID  { $$ = state->ArrowExpr($1, $3); $$->AddRef(); $1->Release(); free($3); }
	|	PLUS expression %prec UNARY_SIGN  { $$ = $2; }
	|	MINUS expression %prec UNARY_SIGN  { $$ = state->UnaryExpr(EXPR_NEG, $2); $$->AddRef(); $2->Release(); }
	|	NOT expression  { $$ = state->UnaryExpr(EXPR_NOT, $2); $$->AddRef(); $2->Release(); }
	|	LOGICAL_NOT expression
		{
			$$ = state->UnaryExpr(EXPR_LOGICAL_NOT, $2->ConvertToBool(state));
			$$->AddRef();
			$2->Release();
		}
	|	STAR expression %prec DEREF  { $$ = state->UnaryExpr(EXPR_DEREF, $2); $$->AddRef(); $2->Release(); }
	|	INCREMENT expression  { $$ = state->UnaryExpr(EXPR_PRE_INCREMENT, $2); $$->AddRef(); $2->Release(); }
	|	DECREMENT expression  { $$ = state->UnaryExpr(EXPR_PRE_DECREMENT, $2); $$->AddRef(); $2->Release(); }
	|	expression INCREMENT %prec POST_INCREMENT
		{
			$$ = state->UnaryExpr(EXPR_POST_INCREMENT, $1);
			$$->AddRef();
			$1->Release();
		}
	|	expression DECREMENT %prec POST_DECREMENT
		{
			$$ = state->UnaryExpr(EXPR_POST_DECREMENT, $1);
			$$->AddRef();
			$1->Release();
		}
	|	AND expression %prec ADDRESS_OF  { $$ = state->UnaryExpr(EXPR_ADDRESS_OF, $2); $$->AddRef(); $2->Release(); }
	|	expression LBRACKET expression RBRACKET
		{
			$$ = state->BinaryExpr(EXPR_ARRAY_INDEX, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression ASSIGN expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, $3);
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression PLUS_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_PLUS, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression MINUS_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_MINUS, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression MULT_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_MULT, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression DIV_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_DIV, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression MOD_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_MOD, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression AND_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_AND, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression OR_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_OR, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression XOR_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_XOR, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression SHIFT_LEFT_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_SHIFT_LEFT, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	expression SHIFT_RIGHT_EQ expression
		{
			$$ = state->BinaryExpr(EXPR_ASSIGN, $1, state->BinaryExpr(EXPR_SHIFT_RIGHT, $1, $3));
			$$->AddRef();
			$1->Release();
			$3->Release();
		}
	|	LPAREN expression_with_comma RPAREN  { $$ = $2; }
	|	LPAREN LBRACE { state->PushScope(); } stmt_list RBRACE RPAREN  { state->PopScope(); $$ = $4; }
	|	LPAREN var_type RPAREN expression %prec CAST
		{
			$$ = state->CastExpr($2, $4);
			$$->AddRef();
			$2->Release();
			$4->Release();
		}
	|	LPAREN return_type LPAREN calling_convention ptr_decorator RPAREN LPAREN param_list RPAREN RPAREN expression %prec CAST
		{
			Type* type = Type::PointerType(Type::FunctionType($2, (CallingConvention)$4, *$8), $5 - 1);
			$$ = state->CastExpr(type, $11);
			$$->AddRef();
			$2->Release();
			delete $8;
			$11->Release();
		}
	|	expression LPAREN arg_list RPAREN  { $$ = state->CallExpr($1, *$3); $$->AddRef(); $1->Release(); delete $3; }
	|	MIN LPAREN arg_list_nonempty RPAREN  { $$ = state->BuiltinCallExpr(EXPR_MIN, *$3); $$->AddRef(); delete $3; }
	|	MAX LPAREN arg_list_nonempty RPAREN  { $$ = state->BuiltinCallExpr(EXPR_MAX, *$3); $$->AddRef(); delete $3; }
	|	ABS LPAREN expression RPAREN  { $$ = state->UnaryExpr(EXPR_ABS, $3); $$->AddRef(); $3->Release(); }
	|	ALLOCA LPAREN expression RPAREN  { $$ = state->UnaryExpr(EXPR_ALLOCA, $3); $$->AddRef(); $3->Release(); }
	|	MEMCPY LPAREN expression COMMA expression COMMA expression RPAREN
		{
			$$ = state->BasicExpr(EXPR_MEMCPY);
			$$->AddChild($3);
			$$->AddChild($5);
			$$->AddChild($7);
			$$->AddRef();
			$3->Release();
			$5->Release();
			$7->Release();
		}
	|	MEMSET LPAREN expression COMMA expression COMMA expression RPAREN
		{
			$$ = state->BasicExpr(EXPR_MEMSET);
			$$->AddChild($3);
			$$->AddChild($5);
			$$->AddChild($7);
			$$->AddRef();
			$3->Release();
			$5->Release();
			$7->Release();
		}
	|	STRLEN LPAREN expression RPAREN  { $$ = state->UnaryExpr(EXPR_STRLEN, $3); $$->AddRef(); $3->Release(); }
	|	SIZEOF LPAREN var_type RPAREN  { $$ = state->IntExpr($3->GetWidth()); $$->AddRef(); $3->Release(); }
	|	SIZEOF LPAREN expression RPAREN
		{
			Ref<Type> type = $3->ComputeType(state, NULL);
			$$ = state->IntExpr(type->GetWidth());
			$$->AddRef();
			$3->Release();
		}
	|	SIZEOF LPAREN var_type LPAREN calling_convention ptr_decorator RPAREN LPAREN param_list RPAREN RPAREN
		{
			$$ = state->IntExpr(GetTargetPointerSize());
			$$->AddRef();
			$3->Release();
			delete $9;
		}
	|	SYSCALL_TOK LPAREN arg_list RPAREN  { $$ = state->BuiltinCallExpr(EXPR_SYSCALL, *$3); $$->AddRef(); delete $3; }
	|	SYSCALL2_TOK LPAREN arg_list RPAREN  { $$ = state->BuiltinCallExpr(EXPR_SYSCALL2, *$3); $$->AddRef(); delete $3; }
	|	RDTSC_TOK LPAREN RPAREN  { $$ = new Expr(EXPR_RDTSC); $$->AddRef(); }
	|	RDTSC_LOW LPAREN RPAREN  { $$ = new Expr(EXPR_RDTSC_LOW); $$->AddRef(); }
	|	RDTSC_HIGH LPAREN RPAREN  { $$ = new Expr(EXPR_RDTSC_HIGH); $$->AddRef(); }
	|	NEXT_ARG LPAREN expression COMMA expression RPAREN
		{
			$$ = state->BinaryExpr(EXPR_NEXT_ARG, $3, $5);
			$$->AddRef();
			$3->Release();
			$5->Release();
		}
	|	PREV_ARG LPAREN expression COMMA expression RPAREN
		{
			$$ = state->BinaryExpr(EXPR_PREV_ARG, $3, $5);
			$$->AddRef();
			$3->Release();
			$5->Release();
		}
	|	BYTESWAP LPAREN expression RPAREN  { $$ = state->UnaryExpr(EXPR_BYTESWAP, $3); $$->AddRef(); $3->Release(); }
	|	BREAKPOINT LPAREN RPAREN  { $$ = new Expr(EXPR_BREAKPOINT); $$->AddRef(); }
	;

expression_with_comma:	expression_with_comma COMMA expression  { $$ = $1; $$->AddChild($3); $3->Release(); }
		|	expression  { $$ = state->UnaryExpr(EXPR_SEQUENCE, $1); $$->AddRef(); $1->Release(); }
		;

arg_list:	arg_list_nonempty  { $$ = $1; }
	|	{ $$ = new vector< Ref<Expr> >(); }
	;

arg_list_nonempty:	arg_list_nonempty COMMA expression  { $$ = $1; $$->push_back($3); $3->Release(); }
		|	expression  { $$ = new vector< Ref<Expr> >(); $$->push_back($1); $1->Release(); }
		;

id_list:	id_list COMMA ID  { $$ = $1; $$->push_back($3); free($3); }
	|	ID  { $$ = new std::vector<std::string>(); $$->push_back($1); free($1); }
	;

%%

