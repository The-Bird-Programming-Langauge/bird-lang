%option noyywrap
%option debug
%option nodefault

%{
#include "./parser.tab.hh"
#include <string>
#include <unistd.h>

#include "token.h"

#undef YYDECL
#define YY_DECL int yylex(yy::Parser::semantic_type* yylval, yy::Parser::location_type* location)

yy::Parser::semantic_type* yylval;
yy::Parser::location_type* location;

using token = yy::Parser::token;
using T = Token::Type;

#define yyterminate() return token::END
#define YY_NO_UNISTD_H
#define YY_USER_ACTION location->step(); location->columns(yyleng);
#define make_token(type) Token(type, yytext, location->begin.line, location->begin.column)
%}


EXP             ([Ee][-+]?[0-9]+)
IDENT           [a-zA-Z][a-zA-Z0-9_\-]*
TYPE_IDENT      "int"|"float"|"bool"|"string"|"void"

%%

%{
    // code to run at the start of the lexer
%}

"//".*\n                { location->lines(); }

"var"                   { yylval->emplace<Token>() = make_token(T::VAR);
                          return token::VAR; }

"const"                 { yylval->emplace<Token>() = make_token(T::CONST);
                          return token::CONST; }

"print"                 { yylval->emplace<Token>() = make_token(T::PRINT);
                          return token::PRINT; }

"if"                    { yylval->emplace<Token>() = make_token(T::IF);
                          return token::IF; }

"else"                  { yylval->emplace<Token>() = make_token(T::ELSE);
                          return token::ELSE; }

"while"                 { yylval->emplace<Token>() = make_token(T::WHILE);
                          return token::WHILE; }

"for"                   { yylval->emplace<Token>() = make_token(T::FOR);
                          return token::FOR; }

"do"                    { yylval->emplace<Token>() = make_token(T::DO);
                          return token::DO; }

"fn"                    { yylval->emplace<Token>() = make_token(T::FN);
                          return token::FN; }

"return"                { yylval->emplace<Token>() = make_token(T::RETURN);
                          return token::RETURN; }

"break"                 { yylval->emplace<Token>() = make_token(T::BREAK);
                          return token::BREAK; }

"continue"              { yylval->emplace<Token>() = make_token(T::CONTINUE);
                          return token::CONTINUE; }

"type"                  { yylval->emplace<Token>() = make_token(T::TYPE);
                          return token::TYPE; }

"true"                  { yylval->emplace<Token>() = make_token(T::BOOL_LITERAL);
                          return token::BOOL_LITERAL;} 

"false"                 { yylval->emplace<Token>() = make_token(T::BOOL_LITERAL);
                          return token::BOOL_LITERAL;}

{TYPE_IDENT}            { yylval->emplace<Token>() = make_token(T::TYPE_LITERAL);
                          return token::TYPE_LITERAL;} 

[0-9]+"."[0-9]+{EXP}?   { yylval->emplace<Token>() = make_token(T::FLOAT_LITERAL);
                          return token::FLOAT_LITERAL;} 

[0-9]+                  { yylval->emplace<Token>() = make_token(T::INT_LITERAL);
                          return token::INT_LITERAL;} 

\".*\"                  { yylval->emplace<Token>() = make_token(T::STR_LITERAL);
                          return token::STR_LITERAL;} 

{IDENT}                 { yylval->emplace<Token>() = make_token(T::IDENTIFIER);
                          return token::IDENTIFIER;} 


"?"                     { yylval->emplace<Token>() = make_token(T::QUESTION);
                          return token::QUESTION; } 

"-"                     { yylval->emplace<Token>() = make_token(T::MINUS);
                          return token::MINUS; } 

"%"                     { yylval->emplace<Token>() = make_token(T::PERCENT);
                          return token::PERCENT; } 

"+"                     { yylval->emplace<Token>() = make_token(T::PLUS);
                          return token::PLUS; }

"/"                     { yylval->emplace<Token>() = make_token(T::SLASH);
                          return token::SLASH; } 

"*"                     { yylval->emplace<Token>() = make_token(T::STAR);
                          return token::STAR; } 

"="                     { yylval->emplace<Token>() = make_token(T::EQUAL);
                          return token::EQUAL; } 

">"                     { yylval->emplace<Token>() = make_token(T::GREATER);
                          return token::GREATER; } 

">="                    { yylval->emplace<Token>() = make_token(T::GREATER_EQUAL);
                          return token::GREATER_EQUAL; } 

"<"                     { yylval->emplace<Token>() = make_token(T::LESS);
                          return token::LESS; } 

"<="                    { yylval->emplace<Token>() = make_token(T::LESS_EQUAL);
                          return token::LESS_EQUAL; } 

"=="                    { yylval->emplace<Token>() = make_token(T::EQUAL_EQUAL);
                          return token::EQUAL_EQUAL; } 

"!="                    { yylval->emplace<Token>() = make_token(T::BANG_EQUAL);
                          return token::BANG_EQUAL; } 

"+="                    { yylval->emplace<Token>() = make_token(T::PLUS_EQUAL);
                          return token::PLUS_EQUAL; } 

"-="                    { yylval->emplace<Token>() = make_token(T::MINUS_EQUAL);
                          return token::MINUS_EQUAL; } 

"*="                    { yylval->emplace<Token>() = make_token(T::STAR_EQUAL);
                          return token::STAR_EQUAL; } 

"/="                    { yylval->emplace<Token>() = make_token(T::SLASH_EQUAL);
                          return token::SLASH_EQUAL; } 

"%="                    { yylval->emplace<Token>() = make_token(T::PERCENT_EQUAL);
                          return token::PERCENT_EQUAL; } 


":"                     { return token::COLON; } 
","                     { return token::COMMA; } 
"}"                     { return token::RBRACE; } 
"{"                     { return token::LBRACE; } 
")"                     { return token::RPAREN; } 
"("                     { return token::LPAREN; }
";"                     { return token::SEMICOLON; } 
"!"                     { return token::BANG; } 
"->"                    { return token::ARROW; } 

\n                      { location->lines(); }

[ \t\r]*                { /* do nothing */ }

<<EOF>>                 { yyterminate(); }

%%