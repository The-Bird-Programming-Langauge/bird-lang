%skeleton "lalr1.cc"
%require  "3.2"
%defines 
%define api.namespace {Bird}
%define api.parser.class {Parser}
%debug


%code requires{
   #include <iostream>
   #include <string>
   #include <vector>
   #include <memory>
   
   #include "../include/ast_node/expr/binary.h"
   #include "../include/ast_node/expr/unary.h"
   #include "../include/ast_node/expr/primary.h"
   #include "../include/ast_node/expr/ternary.h"
   #include "../include/ast_node/expr/call.h"

   #include "../include/ast_node/stmt/decl_stmt.h"
   #include "../include/ast_node/expr/assign_expr.h"
   #include "../include/ast_node/stmt/print_stmt.h"
   #include "../include/ast_node/stmt/if_stmt.h"
   #include "../include/ast_node/stmt/expr_stmt.h"
   #include "../include/ast_node/stmt/const_stmt.h"
   #include "../include/ast_node/stmt/while_stmt.h"
   #include "../include/ast_node/stmt/for_stmt.h"
   #include "../include/ast_node/stmt/return_stmt.h"
   #include "../include/ast_node/stmt/block.h"
   #include "../include/ast_node/stmt/func.h"
   #include "../include/ast_node/stmt/break_stmt.h"
   #include "../include/ast_node/stmt/continue_stmt.h"

   #include "../include/exceptions/bird_exception.h"
   #include "../include/exceptions/user_exception.h"
   #include "../include/exceptions/user_error_tracker.h"

   #include "../include/token.h"


   #define YYDEBUG 1

   namespace Bird {
      class Scanner;
   }
}

%lex-param {Bird::Scanner &scanner}
%parse-param {Bird::Scanner &scanner}
%parse-param {std::vector<std::unique_ptr<Stmt>> &stmts}

%code {
   #include "../src/scanner.hpp"
   static Bird::Parser::token_kind_type yylex(Bird::Parser::semantic_type* value, Bird::Parser::location_type* location, Bird::Scanner& scanner)
   { return scanner.next_token(value, location); }
}

%define parse.assert

%union {
   Token* token_ptr;
   Stmt* stmt_ptr;
   Expr* expr_ptr;

   std::vector<Token> *token_vec;
}

%token END 0 _("end of file")


%token <token_ptr> 
VAR "var"
CONST "const"
IDENTIFIER _("identifier")
TYPE_LITERAL _("type literal")
INT_LITERAL _("int literal")
FLOAT_LITERAL _("float literal")
BOOL_LITERAL _("bool literal")
STR_LITERAL _("string literal")
IF "if"
ELSE "else"
WHILE "while"
FOR "for"
DO "do"
RETURN "return"
BREAK "break"
CONTINUE "continue"
FN "fn"
PRINT "print"

EQUAL "="
PLUS_EQUAL "+="
MINUS_EQUAL "-="
STAR_EQUAL "*="
SLASH_EQUAL "/="
PERCENT_EQUAL "%="
EQUAL_EQUAL "=="
BANG_EQUAL "!="
GREATER ">"
GREATER_EQUAL ">="
LESS "<"
LESS_EQUAL "<="
MINUS "-"
PERCENT "%"
PLUS "+"
SLASH "/"
STAR "*"

%token 
SEMICOLON ";"
QUESTION "?"
COMMA ","
RBRACE "]"
LBRACE "["
RPAREN ")"
LPAREN "("
COLON ":"
BANG "!"
ARROW "->"


%type <stmt_ptr> 
stmt
decl_stmt
if_stmt
const_stmt
print_stmt
block
func
while_stmt
for_stmt
return_stmt
break_stmt
continue_stmt
expr_stmt

%type <expr_ptr> 
expr
assign_expr
ternary_expr
equality_expr
comparison_expr
term_expr
factor_expr
unary_expr
call_expr

%type <expr_ptr> grouping
%type <token_ptr> primary

%type <token_ptr>
ASSIGN_OP
COMPARISON_OP
TERM_OP
FACTOR_OP
UNARY_OP
EQUALITY_OP

%type <token_vec> arg_list

%right ASSIGN
%right TERNARY
%left EQUALITY
%left COMPARISON
%left TERM
%left FACTOR
%right UNARY
%left CALL
%nonassoc PRIMARY GROUPING

%nonassoc THEN
%nonassoc ELSE

%locations

%start program

%%

program: stmts

stmts: 
   | stmt stmts

stmt: decl_stmt
   | if_stmt
   | const_stmt
   | print_stmt
   | block
   | func
   | while_stmt
   | for_stmt
   | return_stmt
   | break_stmt
   | continue_stmt
   | expr_stmt

decl_stmt: VAR IDENTIFIER EQUAL expr SEMICOLON
   | VAR IDENTIFIER COLON TYPE_LITERAL EQUAL expr SEMICOLON

if_stmt: IF expr stmt      %prec THEN
   |     IF expr stmt ELSE stmt

const_stmt: CONST IDENTIFIER EQUAL expr SEMICOLON
   | CONST IDENTIFIER COLON TYPE_LITERAL PLUS expr SEMICOLON

print_stmt: PRINT arg_list SEMICOLON

block: LBRACE stmts RBRACE

func: FN IDENTIFIER LPAREN param_list RPAREN return_type block

while_stmt: WHILE expr stmt

for_stmt: FOR maybe_stmt maybe_stmt expr DO stmt

return_stmt: RETURN SEMICOLON
   | RETURN expr SEMICOLON

break_stmt: BREAK SEMICOLON

continue_stmt: CONTINUE SEMICOLON

expr_stmt: expr SEMICOLON

arg_list: 
   | expr
   | expr COMMA arg_list

param_list: 
   | param
   | param COMMA param_list

param: IDENTIFIER COLON TYPE_LITERAL

return_type: 
   | ARROW TYPE_LITERAL

maybe_stmt: SEMICOLON
   | stmt


expr: assign_expr %prec ASSIGN
   | ternary_expr %prec TERNARY
   | equality_expr %prec EQUALITY
   | comparison_expr %prec COMPARISON
   | term_expr %prec TERM
   | factor_expr %prec FACTOR
   | unary_expr %prec UNARY
   | call_expr %prec CALL
   | primary %prec PRIMARY
   | grouping %prec GROUPING


assign_expr: expr ASSIGN_OP expr

ternary_expr: expr QUESTION expr COLON expr

equality_expr: expr EQUALITY_OP expr

comparison_expr: expr COMPARISON_OP expr

term_expr: expr TERM_OP expr

factor_expr: expr FACTOR_OP expr

unary_expr: UNARY_OP expr

call_expr: expr LPAREN arg_list RPAREN

primary: IDENTIFIER
   | INT_LITERAL
   | FLOAT_LITERAL
   | BOOL_LITERAL
   | STR_LITERAL

grouping: LPAREN expr RPAREN


ASSIGN_OP: EQUAL
   | PLUS_EQUAL
   | MINUS_EQUAL
   | STAR_EQUAL
   | SLASH_EQUAL
   | PERCENT_EQUAL

EQUALITY_OP: EQUAL_EQUAL
   | BANG_EQUAL

COMPARISON_OP: GREATER
   | GREATER_EQUAL
   | LESS
   | LESS_EQUAL

FACTOR_OP: STAR
   | SLASH
   | PERCENT

TERM_OP: PLUS
   | MINUS

UNARY_OP: MINUS

%%

void Bird::Parser::error( const location_type &loc, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at line " << loc << "\n";
}