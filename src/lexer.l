%{
#include <string>

#include "../src/scanner.hpp"
#include "token.h"

using token = Bird::Parser::token;
using T = Token::Type;

#define yyterminate() return token::END

#define YY_NO_UNISTD_H

#define YY_USER_ACTION location->step(); location->columns(yyleng);

#define make_token(type) new Token(type, yytext, location->begin.line, location->begin.column)

%}

%option debug
%option nodefault
%option noyywrap
%option yyclass = "Bird::Scanner"
%option c++

EXP             ([Ee][-+]?[0-9]+)
IDENT           [a-zA-Z][a-zA-Z0-9_\-]*
TYPE_IDENT      "int"|"float"|"bool"|"string"|"void"

%%

%{
    // code to run at the start of the lexer
%}

"//".*\n        { location->lines(); }

"var"           { yylval.token_ptr = make_token(T::VAR);
                  return token::VAR; }

"const"         { yylval.token_ptr = make_token(T::CONST);
                  return token::CONST; }

"print"         { yylval.token_ptr = make_token(T::PRINT);
                  return token::PRINT; }

"if"            { yylval.token_ptr = make_token(T::IF);
                  return token::IF; }

"else"          { yylval.token_ptr = make_token(T::ELSE);
                  return token::ELSE; }

"while"         { yylval.token_ptr = make_token(T::WHILE);
                  return token::WHILE; }

"for"           { yylval.token_ptr = make_token(T::FOR);
                  return token::FOR; }

"do"            { yylval.token_ptr = make_token(T::DO);
                  return token::DO; }

"fn"            { yylval.token_ptr = make_token(T::FN);
                  return token::FN; }

"return"        { yylval.token_ptr = make_token(T::RETURN);
                  return token::RETURN; }

"break"         { yylval.token_ptr = make_token(T::BREAK);
                  return token::BREAK; }

"continue"      { yylval.token_ptr = make_token(T::CONTINUE);
                  return token::CONTINUE; }


{TYPE_IDENT}              { yylval.token_ptr = make_token(T::TYPE_LITERAL);
                           return token::TYPE_LITERAL;} 

[0-9]+"."[0-9]+{EXP}?     {yylval.token_ptr = make_token(T::FLOAT_LITERAL);
                            return token::FLOAT_LITERAL;} 

[0-9]+                    {yylval.token_ptr = make_token(T::INT_LITERAL);
                           return token::INT_LITERAL;} 

"true"                    {yylval.token_ptr = make_token(T::BOOL_LITERAL);
                           return token::BOOL_LITERAL;} 

"false"                   {yylval.token_ptr = make_token(T::BOOL_LITERAL);
                           return token::BOOL_LITERAL;}

\".*\"                    {yylval.token_ptr = make_token(T::STR_LITERAL);
                           return token::STR_LITERAL;} 

{IDENT}                 {yylval.token_ptr = make_token(T::IDENTIFIER);
                         return token::IDENTIFIER;} 

","                     {return token::COMMA;} 
"}"                     {return token::RBRACE;} 
"{"                     {return token::LBRACE;} 
")"                     {return token::RPAREN;} 
"("                     {return token::LPAREN;} 
"-"                     {return token::MINUS;} 
"%"                     {return token::PERCENT;} 
"+"                     {return token::PLUS;} 
"/"                     {return token::SLASH;} 
"*"                     {return token::STAR;} 
";"                     {return token::SEMICOLON;} 
"?"                     {return token::QUESTION;} 
"="                     {return token::EQUAL;} 
":"                     {return token::COLON;} 
">"                     {return token::GREATER;} 
">="                    {return token::GREATER_EQUAL;} 
"<"                     {return token::LESS;} 
"<="                    {return token::LESS_EQUAL;} 
"=="                    {return token::EQUAL_EQUAL;} 
"!="                    {return token::BANG_EQUAL;} 
"+="                    {return token::PLUS_EQUAL;} 
"-="                    {return token::MINUS_EQUAL;} 
"*="                    {return token::STAR_EQUAL;} 
"/="                    {return token::SLASH_EQUAL;} 
"%="                    {return token::PERCENT_EQUAL;} 
"!"                     {return token::BANG;} 
"->"                    {return token::ARROW;} 

\n                      {location->lines();}

[ \t\r]*                {}

<<EOF>>     {yyterminate();}

%%