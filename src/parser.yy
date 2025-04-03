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
   #include <utility>
   #include <variant>

   #include "../include/ast_node/index.h"

   #include "../include/exceptions/user_error_tracker.h"
   #include "../include/parse_type.h"

   #include "../include/token.h"

   #define yynerrs_ _  // define to silence unused variable warning generated by bison

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
INT_LITERAL _("int literal")
FLOAT_LITERAL _("float literal")
STR_LITERAL _("string literal")
TRUE "true"
FALSE "false"
INT "int"
FLOAT "float"
BOOL "bool"
STR "str"
VOID "void"
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
STRUCT "struct"
SELF "self"
AS "as"
AND "and"
XOR "xor"
OR "or"
NOT "not"
MATCH "match"
NAMESPACE "namespace"

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
COLON_COLON "::"
MINUS "-"
PERCENT "%"
PLUS "+"
SLASH "/"
STAR "*"
QUESTION "?"
SEMICOLON ";"
COMMA ","
RBRACE "}"
LBRACE "{"
RPAREN ")"
LPAREN "("
RBRACKET "]"
LBRACKET "["
COLON ":"
BANG "!"
ARROW "->"
FAT_ARROW "=>"
DOT "."

%type <std::unique_ptr<Stmt>> 
top_level_stmt
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
struct_decl
namespace_stmt
namespace_declaration_stmt

%type <std::unique_ptr<Expr>> 
expr
assign_expr
type_cast
ternary_expr
equality_expr
comparison_expr
term_expr
factor_expr
unary_expr
call_expr
struct_initialization
array_initialization
match
direct_member_access
method_call
index_assign
grouping
and_expr
xor_expr
or_expr
match_else_arm
scope_resolution

%type <std::unique_ptr<Subscript>>
subscript_expr

%type <Token> 
primary
ASSIGN_OP
COMPARISON_OP
TERM_OP
FACTOR_OP
PREFIX_UNARY_OP
EQUALITY_OP


%type <std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>>
maybe_match_arms
match_arms

%type <std::vector<std::pair<std::string, std::unique_ptr<Expr>>>>
maybe_struct_initialization_list
struct_initialization_list

%type <std::optional<std::shared_ptr<ParseType::Type>>>
return_type

%type <std::optional<std::unique_ptr<Stmt>>>
maybe_stmt

%type <std::optional<std::unique_ptr<Expr>>>
maybe_expr

%type <std::vector<std::unique_ptr<Stmt>>>
maybe_top_level_stmts
top_level_stmts
namespace_declaration_stmts
maybe_namespace_stmts
maybe_stmts
stmts

%type <std::vector<std::shared_ptr<Expr>>> 
maybe_arg_list
arg_list

%type <std::variant<std::shared_ptr<Stmt>, std::pair<Token, std::shared_ptr<ParseType::Type>>>>
member_decl

%type <std::vector<std::variant<std::shared_ptr<Stmt>, std::pair<Token, std::shared_ptr<ParseType::Type>>>>>
maybe_member_decls
member_decls

%type <std::pair<Token, std::shared_ptr<ParseType::Type>>>
param
prop_decl

%type <std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>> 
maybe_param_list
param_list

%type <std::shared_ptr<ParseType::Type>>
type_identifier

%right ASSIGN
   EQUAL
   PLUS_EQUAL
   MINUS_EQUAL
   STAR_EQUAL
   SLASH_EQUAL
   PERCENT_EQUAL
%right TERNARY
   QUESTION
%left OR
%left XOR
%left AND
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
   NOT
%left CAST
   AS
%left CALL
   LPAREN
%left SUBSCRIPT
   LBRACKET
%left DIRECT_MEMBER_ACCESS
   DOT
%left STRUCT_INITIALIZATION
   IDENTIFIER 
%left ARRAY_INITIALIZATION
   RBRACKET 
%left MATCH_EXPR
   MATCH
%left SCOPE_RESOLUTION_EXPR
   COLON_COLON

%nonassoc THEN
%nonassoc ELSE

%locations

%start program

%%

program: 
   maybe_top_level_stmts { stmts = std::move($1); }

maybe_top_level_stmts:
    %empty { $$ = std::vector<std::unique_ptr<Stmt>>(); }
   | top_level_stmts { $$ = std::move($1); }

top_level_stmts: 
   top_level_stmt 
      { $$ = std::vector<std::unique_ptr<Stmt>>(); $$.push_back(std::move($1)); }
   | top_level_stmts top_level_stmt 
      { $1.push_back(std::move($2)); $$ = std::move($1); }

top_level_stmt: 
   func { $$ = std::move($1); }
   | struct_decl SEMICOLON {$$ = std::move($1); }
   | stmt { $$ = std::move($1); }
   | error {$$ = std::make_unique<Block>(std::vector<std::unique_ptr<Stmt>>()); /*this is an arbitrary stmt to silence errors*/}

stmt:
   decl_stmt SEMICOLON { $$ = std::move($1); }
   | if_stmt { $$ = std::move($1); }
   | const_stmt SEMICOLON { $$ = std::move($1); }
   | print_stmt SEMICOLON { $$ = std::move($1); }
   | block { $$ = std::move($1); }
   | while_stmt { $$ = std::move($1); }
   | for_stmt { $$ = std::move($1); }
   | return_stmt SEMICOLON { $$ = std::move($1); }
   | break_stmt SEMICOLON { $$ = std::move($1); }
   | namespace_stmt { $$ = std::move($1); }
   | continue_stmt SEMICOLON { $$ = std::move($1); }
   | expr_stmt SEMICOLON { $$ = std::move($1); }
   | type_stmt SEMICOLON { $$ = std::move($1); }

maybe_namespace_stmts:
   %empty { $$ = std::vector<std::unique_ptr<Stmt>>(); }
   | namespace_declaration_stmts { $$ = std::move($1); }

namespace_declaration_stmts:
   namespace_declaration_stmt 
      { $$ = std::vector<std::unique_ptr<Stmt>>(); $$.push_back(std::move($1)); }
   | namespace_declaration_stmts namespace_declaration_stmt 
      { $1.push_back(std::move($2)); $$ = std::move($1); }

namespace_declaration_stmt:
   namespace_stmt { $$ = std::move($1); }
   | decl_stmt SEMICOLON { $$ = std::move($1); }
   | const_stmt SEMICOLON { $$ = std::move($1); }
   | struct_decl SEMICOLON { $$ = std::move($1); }
   | type_stmt SEMICOLON { $$ = std::move($1); }
   | func { $$ = std::move($1); }

namespace_stmt:
   NAMESPACE IDENTIFIER LBRACE maybe_namespace_stmts RBRACE
      { $$ = std::make_unique<NamespaceStmt>($2, std::move($4)); }

scope_resolution:
    IDENTIFIER COLON_COLON expr %prec SCOPE_RESOLUTION_EXPR
    { $$ = std::make_unique<ScopeResolutionExpr>($1, std::move($3)); }

struct_decl:
   STRUCT IDENTIFIER LBRACE maybe_member_decls RBRACE 
      { $$ = std::make_unique<StructDecl>($2, std::move($4)); }

maybe_member_decls:
   %empty { $$ = std::vector<std::variant<std::shared_ptr<Stmt>, std::pair<Token, std::shared_ptr<ParseType::Type>>>>(); }
   | member_decls { $$ = std::move($1); }

member_decls:
   member_decl {$$ = std::vector<std::variant<std::shared_ptr<Stmt>, std::pair<Token, std::shared_ptr<ParseType::Type>>>>(); $$.push_back(std::move($1)); }
   | member_decls member_decl { $$ = std::move($1); $$.push_back(std::move($2)); }

member_decl:
   func { $$ = std::move($1); }
   | prop_decl { $$ = std::move($1); }

prop_decl: 
   IDENTIFIER COLON type_identifier SEMICOLON { $$ = std::pair<Token, std::shared_ptr<ParseType::Type>>({$1, $3}); }

decl_stmt: 
   VAR IDENTIFIER EQUAL expr 
      { $$ = std::make_unique<DeclStmt>($2, std::nullopt, std::move($4)); }
   | VAR IDENTIFIER COLON type_identifier EQUAL expr
      { $$ = std::make_unique<DeclStmt>($2, $4, std::move($6)); }

const_stmt: 
   CONST IDENTIFIER EQUAL expr 
      { $$ = std::make_unique<ConstStmt>($2, std::nullopt, std::move($4)); }
   | CONST IDENTIFIER COLON type_identifier EQUAL expr 
      { $$ = std::make_unique<ConstStmt>($2, std::move($4), std::move($6)); }

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

while_stmt: 
   WHILE expr block 
      { $$ = std::make_unique<WhileStmt>($1, std::move($2), std::move($3)); }

for_stmt: 
   FOR maybe_stmt maybe_expr SEMICOLON maybe_expr block 
      { $$ = std::make_unique<ForStmt>(  
            $1, 
            std::move($2), 
            std::move($3), 
            std::move($5), 
            std::move($6)); }

print_stmt: 
   PRINT arg_list 
      { $$ = std::make_unique<PrintStmt>(std::move($2), $1); }

block: 
   LBRACE maybe_stmts RBRACE 
      { $$ = std::make_unique<Block>(std::move($2)); }

maybe_stmts:
    %empty { $$ = std::vector<std::unique_ptr<Stmt>>(); }
   | stmts { $$ = std::move($1); }

stmts: 
   stmt 
      { $$ = std::vector<std::unique_ptr<Stmt>>(); $$.push_back(std::move($1)); }
   | stmts stmt 
      { $1.push_back(std::move($2)); $$ = std::move($1); }

func: 
   FN IDENTIFIER LPAREN maybe_param_list RPAREN return_type block 
      { $$ = std::make_unique<Func>($2, $6, $4, std::move($7)); }

maybe_stmt: 
   SEMICOLON { $$ = std::nullopt; }
   | stmt { $$ = std::move($1); }

maybe_expr: 
   %empty { $$ = std::nullopt; }
   | expr { $$ = std::move($1); }

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
   TYPE IDENTIFIER EQUAL type_identifier 
      { $$ = std::make_unique<TypeStmt>($2, $4); }

maybe_arg_list: 
   %empty { $$ = (std::vector<std::shared_ptr<Expr>>()); }
   | arg_list { $$ = std::move($1); }

arg_list: 
   expr 
      { $$ = std::vector<std::shared_ptr<Expr>>(); $$.push_back(std::move($1)); }
   | arg_list COMMA expr 
      { $1.push_back(std::move($3)); $$ = std::move($1); }

maybe_param_list: 
   %empty { $$ = std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>{}; }
   | param_list

param_list: 
   param 
      { $$ = std::vector{$1}; }
   | param_list COMMA param 
      { $1.push_back($3); $$ = $1; }

param: 
   IDENTIFIER COLON type_identifier 
      { $$ = std::pair<Token, std::shared_ptr<ParseType::Type>>($1, $3); }

return_type: 
   %empty { $$ = std::nullopt; }
   | ARROW type_identifier { $$ = std::optional<std::shared_ptr<ParseType::Type>>($2); }



expr: 
   assign_expr { $$ = std::move($1); }
   | ternary_expr { $$ = std::move($1); }
   | equality_expr { $$ = std::move($1); }
   | comparison_expr { $$ = std::move($1); }
   | term_expr { $$ = std::move($1); }
   | factor_expr { $$ = std::move($1); }
   | and_expr { $$ = std::move($1); }
   | xor_expr { $$ = std::move($1); }
   | or_expr { $$ = std::move($1); }
   | unary_expr { $$ = std::move($1); }
   | type_cast {$$ = std::move($1); }
   | call_expr { $$ = std::move($1); }
   | method_call {$$ = std::move($1); }
   | subscript_expr { $$ = std::move($1); }
   | index_assign { $$ = std::move($1); }
   | direct_member_access { $$ = std::move($1); }
   | struct_initialization { $$ = std::move($1); }
   | array_initialization { $$ = std::move($1); }
   | match { $$ = std::move($1); }
   | scope_resolution { $$ = std::move($1); }
   | primary { $$ = std::make_unique<Primary>($1); }
   | grouping { $$ = std::move($1); }


assign_expr:
   primary ASSIGN_OP expr %prec ASSIGN 
   { $$ = std::make_unique<AssignExpr>($1, $2, std::move($3)); }
   | direct_member_access ASSIGN_OP expr %prec ASSIGN { 
      auto member_access = dynamic_cast<DirectMemberAccess *>($1.get());
      $$ = std::make_unique<MemberAssign>(std::move(member_access->accessable), member_access->identifier, $2, std::move($3));
   }

index_assign:
   subscript_expr ASSIGN_OP expr %prec ASSIGN 
   { $$ = std::make_unique<IndexAssign>(std::move($1), std::move($3), $2); }

type_cast:
   expr AS type_identifier %prec CAST {$$ = std::make_unique<AsCast>(std::move($1), $3);}

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
   PREFIX_UNARY_OP expr %prec UNARY 
      { $$ = std::make_unique<Unary>($1, std::move($2)); }
   | expr QUESTION %prec UNARY  {$$ = std::make_unique<Unary>($2, std::move($1));}

and_expr:
   expr AND expr %prec AND
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

xor_expr:
   expr XOR expr %prec XOR
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

or_expr:
   expr OR expr %prec OR
      { $$ = std::make_unique<Binary>(std::move($1), $2, std::move($3)); }

call_expr: 
   IDENTIFIER LPAREN maybe_arg_list RPAREN %prec CALL 
      { $$ = std::make_unique<Call>($1, std::move($3)); }

struct_initialization:
   IDENTIFIER LBRACE maybe_struct_initialization_list RBRACE 
      { $$ = std::make_unique<StructInitialization>($1, std::move($3)); }

maybe_struct_initialization_list:
   %empty { $$ = std::vector<std::pair<std::string, std::unique_ptr<Expr>>>(); }
   | struct_initialization_list { $$ = std::move($1); }
   | struct_initialization_list COMMA { $$ = std::move($1); }

struct_initialization_list:
   IDENTIFIER EQUAL expr 
      { $$ = std::vector<std::pair<std::string, std::unique_ptr<Expr>>>();
        $$.push_back(std::make_pair($1.lexeme, std::move($3))); }
   | struct_initialization_list COMMA IDENTIFIER EQUAL expr 
      { $$ = std::move($1);
        $$.push_back(std::make_pair($3.lexeme, std::move($5))); }

array_initialization: 
   LBRACKET maybe_arg_list RBRACKET %prec ARRAY_INITIALIZATION {
      $$ = std::make_unique<ArrayInit>($2);
   }

subscript_expr:
   expr LBRACKET expr RBRACKET %prec SUBSCRIPT
      { $$ = std::make_unique<Subscript>(std::move($1), std::move($3), $2); }

direct_member_access:
   expr DOT IDENTIFIER %prec SUBSCRIPT
      { $$ = std::make_unique<DirectMemberAccess>(std::move($1), $3); }

method_call:
   expr DOT call_expr %prec CALL
      { $$ = std::make_unique<MethodCall>(std::move($1), dynamic_cast<Call *>($3.get())); }

match:
   MATCH expr LBRACE maybe_match_arms match_else_arm RBRACE %prec MATCH_EXPR
   { $$ = std::make_unique<MatchExpr>($1, std::move($2), std::move($4), std::move($5)); }

maybe_match_arms:
   %empty { $$ = std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>(); }
   | match_arms { $$ = std::move($1); }
   | match_arms COMMA { $$ = std::move($1); }

match_arms:
   expr FAT_ARROW expr 
      {  $$ = std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>(); 
         $$.push_back(std::make_pair(std::move($1), std::move($3)));
      }
   | match_arms COMMA expr FAT_ARROW expr 
      {
         $$ = std::move($1);
         $$.push_back(std::make_pair(std::move($3), std::move($5)));
      }

match_else_arm:
   ELSE FAT_ARROW expr
   { $$ = std::move($3); }

primary: 
   IDENTIFIER 
   | SELF
   | INT_LITERAL 
   | FLOAT_LITERAL
   | TRUE
   | FALSE
   | STR_LITERAL

grouping: 
   LPAREN expr RPAREN 
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

PREFIX_UNARY_OP: 
   MINUS
   | NOT

type_identifier:
   IDENTIFIER { $$ = std::make_shared<ParseType::UserDefined>($1); }
   | type_identifier COLON_COLON IDENTIFIER { 
       $$ = std::make_shared<ParseType::UserDefined>(
           Token(Token::Type::IDENTIFIER,
                 $1->get_token().lexeme + "::" + $3.lexeme, 
                 $3.line_num, 
                 $3.char_num)
       ); 
   }
   | INT { $$ = std::make_shared<ParseType::Primitive>($1); }
   | FLOAT { $$ = std::make_shared<ParseType::Primitive>($1); }
   | BOOL { $$ = std::make_shared<ParseType::Primitive>($1); }
   | STR { $$ = std::make_shared<ParseType::Primitive>($1); }
   | VOID { $$ = std::make_shared<ParseType::Primitive>($1); }
   | type_identifier LBRACKET RBRACKET { $$ = std::make_shared<ParseType::Array>($1); }

%%

void yy::yyParser::error( const location_type &loc, const std::string &err_message )
{
   user_error_tracker->parse_error(err_message, loc.begin.line, loc.begin.column);
}