#include "../include/token.h"

#include <map>
#include <iostream>
static const std::map<Token::Type, std::string> token_strings = {
    {Token::Type::INT_LITERAL, "INT_LITERAL"},
    {Token::Type::IDENTIFIER, "IDENTIFIER"},
    {Token::Type::VAR, "VAR"},
    {Token::Type::CONST, "CONST"},
    {Token::Type::MINUS, "MINUS"},
    {Token::Type::PERCENT, "PERCENT"},
    {Token::Type::PLUS, "PLUS"},
    {Token::Type::SLASH, "SLASH"},
    {Token::Type::STAR, "STAR"},
    {Token::Type::SEMICOLON, "SEMICOLON"},
    {Token::Type::QUESTION, "QUESTION"},
    {Token::Type::EQUAL, "EQUAL"},
    {Token::Type::PRINT, "PRINT"},
    {Token::Type::COMMA, "COMMA"},
    {Token::Type::RBRACE, "RBRACE"},
    {Token::Type::LBRACE, "LBRACE"},
    {Token::Type::RPAREN, "RPAREN"},
    {Token::Type::LPAREN, "LPAREN"},
    {Token::Type::RBRACKET, "RBRACKET"},
    {Token::Type::LBRACKET, "LBRACKET"},
    {Token::Type::TYPE_LITERAL, "TYPE_LITERAL"},
    {Token::Type::FLOAT_LITERAL, "FLOAT_LITERAL"},
    {Token::Type::BOOL_LITERAL, "BOOL_LITERAL"},
    {Token::Type::STR_LITERAL, "STR_LITERAL"},
    {Token::Type::COLON, "COLON"},
    {Token::Type::IF, "IF"},
    {Token::Type::ELSE, "ELSE"},
    {Token::Type::WHILE, "WHILE"},
    {Token::Type::FOR, "FOR"},
    {Token::Type::DO, "DO"},
    {Token::Type::GREATER, "GREATER"},
    {Token::Type::LESS, "LESS"},
    {Token::Type::BANG, "BANG"},
    {Token::Type::ARROW, "ARROW"},
    {Token::Type::GREATER_EQUAL, "GREATER_EQUAL"},
    {Token::Type::LESS_EQUAL, "LESS_EQUAL"},
    {Token::Type::EQUAL_EQUAL, "EQUAL_EQUAL"},
    {Token::Type::BANG_EQUAL, "BANG_EQUAL"},
    {Token::Type::PLUS_EQUAL, "PLUS_EQUAL"},
    {Token::Type::MINUS_EQUAL, "MINUS_EQUAL"},
    {Token::Type::STAR_EQUAL, "STAR_EQUAL"},
    {Token::Type::SLASH_EQUAL, "SLASH_EQUAL"},
    {Token::Type::PERCENT_EQUAL, "PERCENT_EQUAL"},
    {Token::Type::RETURN, "RETURN"},
    {Token::Type::BREAK, "BREAK"},
    {Token::Type::CONTINUE, "CONTINUE"},
    {Token::Type::FN, "FN"},
    {Token::Type::END, "END"},
};

void Token::print_token()
{
    std::cout << "{ "
              << "token_type: " << token_strings.at(this->token_type)
              << ",  lexeme: " << this->lexeme
              << " }"
              << std::endl;
}