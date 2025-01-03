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
   #include <utility>
   
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
   #include "../include/ast_node/stmt/type_stmt.h"

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

   std::optional<Token> *maybe_token_ptr;
   std::vector<Stmt*> *stmt_vec;
   std::vector<Expr*> *expr_vec;
   std::pair<Token, Token> *param_pair;
   std::vector<std::pair<Token, Token>> *param_list_vec;
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
TYPE "type"

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
QUESTION "?"

%token 
SEMICOLON ";"
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
type_stmt
maybe_stmt

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
grouping
maybe_expr

%type <token_ptr> 
primary
ASSIGN_OP
COMPARISON_OP
TERM_OP
FACTOR_OP
UNARY_OP
EQUALITY_OP


%type <maybe_token_ptr>
return_type

%type <stmt_vec>
maybe_stmts
stmts

%type <expr_vec> 
maybe_arg_list
arg_list

%type <param_pair>
param

%type <param_list_vec> 
maybe_param_list
param_list

%right ASSIGN
   EQUAL
   PLUS_EQUAL
   MINUS_EQUAL
   STAR_EQUAL
   SLASH_EQUAL
   PERCENT_EQUAL
%right TERNARY
   QUESTION
%left EQUALITY
   EQUAL_EQUAL
   BANG_EQUAL
%left COMPARISON
   GREATER
   GREATER_EQUAL
   LESS
   LESS_EQUAL
%left TERM
   MINUS
   PLUS
%left FACTOR
   STAR
   SLASH
   PERCENT
%right UNARY
%left CALL
   LPAREN
%nonassoc GROUPING
   INT_LITERAL FLOAT_LITERAL BOOL_LITERAL STR_LITERAL IDENTIFIER

%nonassoc THEN
%nonassoc ELSE

%locations

%start program

%%

program: maybe_stmts { 
   for (Stmt* stmt : *$1)
   {
      stmts.push_back(std::unique_ptr<Stmt>(stmt));
   }
}

maybe_stmts: %empty { $$ = new std::vector<Stmt*>(); }
   | stmts { $$ = $1; }

stmts: stmt { $$ = new std::vector<Stmt*>{$1}; }
   | stmt stmts { $2->push_back($1); $$ = $2; }

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
   | type_stmt

decl_stmt: VAR IDENTIFIER EQUAL expr SEMICOLON 
      { $$ = new DeclStmt(*$1, std::nullopt, false, $4); }
   | VAR IDENTIFIER COLON TYPE_LITERAL EQUAL expr SEMICOLON
      { $$ = new DeclStmt(*$1, *$2, true, $6); }
   | VAR IDENTIFIER COLON IDENTIFIER EQUAL expr SEMICOLON
      { $$ = new DeclStmt(*$1, *$2, false, $6); }

if_stmt: IF expr stmt %prec THEN { $$ = new IfStmt(*$1, $2, $3, std::nullopt); }
   | IF expr stmt ELSE stmt { $$ = new IfStmt(*$1, $2, $3, $5); }

const_stmt: CONST IDENTIFIER EQUAL expr SEMICOLON 
      { $$ = new ConstStmt(*$2, std::nullopt, false, $4); }
   | CONST IDENTIFIER COLON TYPE_LITERAL PLUS expr SEMICOLON 
      { $$ = new ConstStmt(*$2, *$4, true, $6); }
   | CONST IDENTIFIER COLON IDENTIFIER PLUS expr SEMICOLON 
      { $$ = new ConstStmt(*$2, *$4, false, $6); }

print_stmt: PRINT arg_list SEMICOLON { $$ = new PrintStmt(*$2); }

block: LBRACE stmts RBRACE { $$ = new Block(*$2); }

func: FN IDENTIFIER LPAREN maybe_param_list RPAREN return_type block 
   { $$ = new Func(*$2, *$6, *$4, $7); }

while_stmt: WHILE expr stmt { $$ = new WhileStmt(*$1, $2, $3); }

for_stmt: FOR maybe_stmt maybe_expr SEMICOLON maybe_expr DO stmt 
   { $$ = new ForStmt(*$1, $2, $3, $5, $7); }

return_stmt: RETURN SEMICOLON { $$ = new ReturnStmt(*$1); }
   | RETURN expr SEMICOLON { $$ = new ReturnStmt(*$1, $2); }

break_stmt: BREAK SEMICOLON { $$ = new BreakStmt(*$1); }

continue_stmt: CONTINUE SEMICOLON { $$ = new ContinueStmt(*$1); }

expr_stmt: expr SEMICOLON { $$ = new ExprStmt($1); }

type_stmt: TYPE IDENTIFIER EQUAL TYPE_LITERAL SEMICOLON { $$ =  new TypeStmt(*$2, *$4, true); }
   | TYPE IDENTIFIER EQUAL IDENTIFIER SEMICOLON { $$ = new TypeStmt(*$2, *$4, false); }

maybe_arg_list: %empty { $$ = new std::vector<Expr*>(); }
   | arg_list

arg_list: expr { $$ = new std::vector{$1}; }
   | expr COMMA arg_list { $3->push_back($1); $$ = $3; }

maybe_param_list: %empty { $$ = new std::vector<std::pair<Token, Token>>{}; }
   | param_list

param_list: param { $$ = new std::vector{*$1}; }
   | param COMMA param_list { $3->push_back(*$1); $$ = $3; }

param: IDENTIFIER COLON TYPE_LITERAL { $$ = new std::pair<Token, Token>(*$1, *$3); }

return_type: %empty { $$ = new std::optional<Token>{}; }
   | ARROW TYPE_LITERAL { $$ = new std::optional<Token>(*$2); }

maybe_stmt: SEMICOLON { $$ = nullptr; }
   | stmt { $$ = $1; }

maybe_expr: %empty { $$ = nullptr; }
   | expr { $$ = $1; }


expr: 
   assign_expr
   | ternary_expr
   | equality_expr
   | comparison_expr
   | term_expr
   | factor_expr
   | unary_expr
   | call_expr
   | primary { $$ = new Primary(*$1); }
   | grouping


assign_expr: expr ASSIGN_OP expr %prec ASSIGN { 
   if(auto *identifier = dynamic_cast<Primary *>($1))
   {
      if (identifier->value.token_type != Token::Type::IDENTIFIER)
      {
         // TODO: throw an error here
      }
      $$ = new AssignExpr(identifier->value, *$2, $3);
   }
}

ternary_expr: expr QUESTION expr COLON expr %prec TERNARY { $$ = new Ternary($1, *$2, $3, $5); }

equality_expr: expr EQUALITY_OP expr %prec EQUALITY { $$ = new Binary($1, *$2, $3); }

comparison_expr: expr COMPARISON_OP expr %prec COMPARISON { $$ = new Binary($1, *$2, $3); }

term_expr: expr TERM_OP expr %prec TERM { $$ = new Binary($1, *$2, $3); }

factor_expr: expr FACTOR_OP expr %prec FACTOR { $$ = new Binary($1, *$2, $3); }

unary_expr: UNARY_OP expr %prec UNARY { $$ = new Unary(*$1, $2); }

call_expr: expr LPAREN maybe_arg_list RPAREN %prec CALL { 
   if(auto *identifier = dynamic_cast<Primary *>($1))
   {
      if (identifier->value.token_type != Token::Type::IDENTIFIER)
      {
         // TODO: throw an error here
      }
      $$ = new Call(identifier->value, *$3);
   }
}

primary: IDENTIFIER 
   | INT_LITERAL 
   | FLOAT_LITERAL
   | BOOL_LITERAL
   | STR_LITERAL

grouping: LPAREN expr RPAREN %prec GROUPING { $$ = $2; }

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

UNARY_OP: MINUS /* %prec UNARY */

%%

void Bird::Parser::error( const location_type &loc, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at line " << loc << "\n";
}