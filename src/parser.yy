%skeleton "lalr1.cc"
%require  "3.2"
%language "c++"
%defines 
%define api.parser.class {yyParser}
%debug
%locations
%define api.value.type variant
%define parse.assert

%parse-param { std::vector<std::unique_ptr<Stmt>> &stmts }
%parse-param { UserErrorTracker *user_error_tracker }

%define parse.error verbose

%code requires {
   #include <iostream>
   #include <string>
   #include <vector>
   #include <memory>
   #include <optional>

   #include "ast_node/index.h"

   #include "../include/exceptions/bird_exception.h"
   #include "../include/exceptions/user_exception.h"
   #include "../include/exceptions/user_error_tracker.h"

   #include "token.h"

   // #define YYDEBUG 1
}

%code {
   extern int yylex(yy::yyParser::semantic_type *yylval, yy::yyParser::location_type *yyloc);
}

%token END 0 _("end of file")

%token <Token> 
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
RBRACE "}"
LBRACE "{"
RPAREN ")"
LPAREN "("
COLON ":"
BANG "!"
ARROW "->"


%type <std::unique_ptr<Stmt>> 
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

%type <std::unique_ptr<Expr>> 
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

%type <Token> 
primary
ASSIGN_OP
COMPARISON_OP
TERM_OP
FACTOR_OP
UNARY_OP
EQUALITY_OP


%type <std::optional<Token>>
return_type

%type <std::optional<std::unique_ptr<Stmt>>>
maybe_stmt

%type <std::optional<std::unique_ptr<Expr>>>
maybe_expr

%type <std::vector<std::unique_ptr<Stmt>>>
maybe_stmts
stmts

%type <std::vector<std::shared_ptr<Expr>>> 
maybe_arg_list
arg_list

%type <std::pair<Token,Token>>
param

%type <std::vector<std::pair<Token,Token>>> 
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

program: 
   maybe_stmts { stmts = std::move($1); }

maybe_stmts:
    %empty { $$ = std::vector<std::unique_ptr<Stmt>>(); }
   | stmts { $$ = std::move($1); }

stmts: 
   stmt 
      { $$ = std::vector<std::unique_ptr<Stmt>>(); $$.push_back(std::move($1)); }
   | stmts stmt 
      { $1.push_back(std::move($2)); $$ = std::move($1); }

stmt: 
   decl_stmt SEMICOLON { $$ = std::move($1); }
   | if_stmt { $$ = std::move($1); }
   | const_stmt SEMICOLON { $$ = std::move($1); }
   | print_stmt SEMICOLON { $$ = std::move($1); }
   | block { $$ = std::move($1); }
   | func { $$ = std::move($1); }
   | while_stmt { $$ = std::move($1); }
   | for_stmt { $$ = std::move($1); }
   | return_stmt SEMICOLON { $$ = std::move($1); }
   | break_stmt SEMICOLON { $$ = std::move($1); }
   | continue_stmt SEMICOLON { $$ = std::move($1); }
   | expr_stmt SEMICOLON { $$ = std::move($1); }
   | type_stmt SEMICOLON { $$ = std::move($1); }
   | error {$$ = std::make_unique<Block>(std::vector<std::unique_ptr<Stmt>>()); /*this is an arbitrary stmt to silence errors*/}


decl_stmt: 
   VAR IDENTIFIER EQUAL expr 
      { $$ = std::make_unique<DeclStmt>($2, std::nullopt, false, std::move($4)); }
   | VAR IDENTIFIER COLON TYPE_LITERAL EQUAL expr
      { $$ = std::make_unique<DeclStmt>($2, $4, true, std::move($6)); }
   | VAR IDENTIFIER COLON IDENTIFIER EQUAL expr
      { $$ = std::make_unique<DeclStmt>($2, $4, false, std::move($6)); }

if_stmt: 
   IF expr block %prec THEN 
      { $$ = std::make_unique<IfStmt>(
            $1, 
            std::move($2), 
            std::move($3), 
            std::nullopt); }
   | IF expr block ELSE block 
      { $$ = std::make_unique<IfStmt>(
            $1, 
            std::move($2), 
            std::move($3), 
            std::move($5)); }
   | IF expr block ELSE if_stmt 
      { $$ = std::make_unique<IfStmt>(
            $1, 
            std::move($2), 
            std::move($3), 
            std::move($5)); }

const_stmt: 
   CONST IDENTIFIER EQUAL expr 
      { $$ = std::make_unique<ConstStmt>($2, std::nullopt, false, std::move($4)); }
   | CONST IDENTIFIER COLON TYPE_LITERAL EQUAL expr 
      { $$ = std::make_unique<ConstStmt>($2, $4, true, std::move($6)); }
   | CONST IDENTIFIER COLON IDENTIFIER EQUAL expr 
      { $$ = std::make_unique<ConstStmt>($2, $4, false, std::move($6)); }

print_stmt: 
   PRINT arg_list 
      { $$ = std::make_unique<PrintStmt>(std::move($2)); }

block: 
   LBRACE maybe_stmts RBRACE 
      { $$ = std::make_unique<Block>(std::move($2)); }

func: 
   FN IDENTIFIER LPAREN maybe_param_list RPAREN return_type block 
      { $$ = std::make_unique<Func>($2, $6, $4, std::move($7)); }

while_stmt: 
   WHILE expr block 
      { $$ = std::make_unique<WhileStmt>($1, std::move($2), std::move($3)); }

for_stmt: 
   FOR maybe_stmt maybe_expr SEMICOLON maybe_expr DO stmt 
      { $$ = std::make_unique<ForStmt>(  
            $1, 
            std::move($2), 
            std::move($3), 
            std::move($5), 
            std::move($7)); }

return_stmt: 
   RETURN 
      { $$ = std::make_unique<ReturnStmt>($1, std::nullopt); }
   | RETURN expr 
      { $$ = std::make_unique<ReturnStmt>($1, std::move($2)); }

break_stmt: 
   BREAK 
      { $$ = std::make_unique<BreakStmt>($1); }

continue_stmt: 
   CONTINUE 
      { $$ = std::make_unique<ContinueStmt>($1); }

expr_stmt: 
   expr 
      { $$ = std::make_unique<ExprStmt>(std::move($1)); }

type_stmt: 
   TYPE IDENTIFIER EQUAL TYPE_LITERAL 
      { $$ =  std::make_unique<TypeStmt>($2, $4, true); }
   | TYPE IDENTIFIER EQUAL IDENTIFIER 
      { $$ = std::make_unique<TypeStmt>($2, $4, false); }

maybe_arg_list: 
   %empty { $$ = (std::vector<std::shared_ptr<Expr>>()); }
   | arg_list { $$ = std::move($1); }

arg_list: 
   expr 
      { $$ = std::vector<std::shared_ptr<Expr>>(); $$.push_back(std::move($1)); }
   | arg_list COMMA expr 
      { $1.push_back(std::move($3)); $$ = std::move($1); }

maybe_param_list: 
   %empty { $$ = std::vector<std::pair<Token, Token>>{}; }
   | param_list

param_list: 
   param 
      { $$ = std::vector{$1}; }
   | param_list COMMA param 
      { $1.push_back($3); $$ = $1; }

param: 
   IDENTIFIER COLON TYPE_LITERAL 
      { $$ = std::pair<Token, Token>($1, $3); }
   | IDENTIFIER COLON IDENTIFIER
      { $$ = std::pair<Token, Token>($1, $3); }

return_type: 
   %empty { $$ = std::optional<Token>{}; }
   | ARROW TYPE_LITERAL { $$ = std::optional<Token>($2); }
   | ARROW IDENTIFIER { $$ = std::optional<Token>($2); }

maybe_stmt: 
   SEMICOLON { $$ = std::nullopt; }
   | stmt { $$ = std::move($1); }

maybe_expr: 
   %empty { $$ = std::nullopt; }
   | expr { $$ = std::move($1); }


expr: 
   assign_expr { $$ = std::move($1); }
   | ternary_expr { $$ = std::move($1); }
   | equality_expr { $$ = std::move($1); }
   | comparison_expr { $$ = std::move($1); }
   | term_expr { $$ = std::move($1); }
   | factor_expr { $$ = std::move($1); }
   | unary_expr { $$ = std::move($1); }
   | call_expr { $$ = std::move($1); }
   | primary { $$ = std::make_unique<Primary>($1); }
   | grouping { $$ = std::move($1); }


assign_expr: 
   expr ASSIGN_OP expr %prec ASSIGN 
      { 
         if(auto *identifier = dynamic_cast<Primary *>($1.get()))
         {
            if (identifier->value.token_type != Token::Type::IDENTIFIER)
            {
               // TODO: throw an error here
            }
            $$ = std::make_unique<AssignExpr>(identifier->value, $2, std::move($3));
         }
      }

ternary_expr: 
   expr QUESTION expr COLON expr %prec TERNARY 
      { $$ = std::make_unique<Ternary>(  
            std::move($1), 
            $2, 
            std::move($3), 
            std::move($5)); }

equality_expr: 
   expr EQUALITY_OP expr %prec EQUALITY 
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

comparison_expr: 
   expr COMPARISON_OP expr %prec COMPARISON 
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

term_expr: 
   expr TERM_OP expr %prec TERM 
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

factor_expr: 
   expr FACTOR_OP expr %prec FACTOR 
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

unary_expr: 
   UNARY_OP expr %prec UNARY 
      { $$ = std::make_unique<Unary>($1, std::move($2)); }

call_expr: 
   expr LPAREN maybe_arg_list RPAREN %prec CALL 
      { 
         if(auto *identifier = dynamic_cast<Primary *>($1.get()))
         {
            if (identifier->value.token_type != Token::Type::IDENTIFIER)
            {
               // TODO: throw an error here
            }
            $$ = std::make_unique<Call>(identifier->value, std::move($3));
         }
      }

primary: 
   IDENTIFIER 
   | INT_LITERAL 
   | FLOAT_LITERAL
   | BOOL_LITERAL
   | STR_LITERAL

grouping: 
   LPAREN expr RPAREN %prec GROUPING
      { $$ = std::move($2); }

ASSIGN_OP: 
   EQUAL
   | PLUS_EQUAL
   | MINUS_EQUAL
   | STAR_EQUAL
   | SLASH_EQUAL
   | PERCENT_EQUAL

EQUALITY_OP: 
   EQUAL_EQUAL
   | BANG_EQUAL

COMPARISON_OP: 
   GREATER
   | GREATER_EQUAL
   | LESS
   | LESS_EQUAL

FACTOR_OP: 
   STAR
   | SLASH
   | PERCENT

TERM_OP: 
   PLUS
   | MINUS

UNARY_OP: 
   MINUS

%%

void yy::yyParser::error( const location_type &loc, const std::string &err_message )
{
   user_error_tracker->parse_error(err_message, loc.begin.line, loc.begin.column);
}