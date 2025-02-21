#include "../include/token.h"

#include <map>
#include <iostream>
static const std::map<Token::Type, std::string> token_strings = {
    {Token::Type::AS, "AS"},
    {Token::Type::ARROW, "ARROW"},
    {Token::Type::BANG, "BANG"},
    {Token::Type::BANG_EQUAL, "BANG_EQUAL"},
    {Token::Type::BOOL_LITERAL, "BOOL_LITERAL"},
    {Token::Type::BREAK, "BREAK"},
    {Token::Type::COLON, "COLON"},
    {Token::Type::COMMA, "COMMA"},
    {Token::Type::CONST, "CONST"},
    {Token::Type::CONTINUE, "CONTINUE"},
    {Token::Type::DO, "DO"},
    {Token::Type::DOT, "DOT"},
    {Token::Type::ELSE, "ELSE"},
    {Token::Type::END, "END"},
    {Token::Type::EQUAL, "EQUAL"},
    {Token::Type::EQUAL_EQUAL, "EQUAL_EQUAL"},
    {Token::Type::FAT_ARROW, "FAT_ARROW"},
    {Token::Type::FLOAT_LITERAL, "FLOAT_LITERAL"},
    {Token::Type::FN, "FN"},
    {Token::Type::FOR, "FOR"},
    {Token::Type::GREATER, "GREATER"},
    {Token::Type::GREATER_EQUAL, "GREATER_EQUAL"},
    {Token::Type::IDENTIFIER, "IDENTIFIER"},
    {Token::Type::IF, "IF"},
    {Token::Type::INT_LITERAL, "INT_LITERAL"},
    {Token::Type::LBRACE, "LBRACE"},
    {Token::Type::LBRACKET, "LBRACKET"},
    {Token::Type::LESS, "LESS"},
    {Token::Type::LESS_EQUAL, "LESS_EQUAL"},
    {Token::Type::LPAREN, "LPAREN"},
    {Token::Type::MATCH, "MATCH"},
    {Token::Type::MINUS, "MINUS"},
    {Token::Type::MINUS_EQUAL, "MINUS_EQUAL"},
    {Token::Type::PERCENT, "PERCENT"},
    {Token::Type::PERCENT_EQUAL, "PERCENT_EQUAL"},
    {Token::Type::PLUS, "PLUS"},
    {Token::Type::PLUS_EQUAL, "PLUS_EQUAL"},
    {Token::Type::PRINT, "PRINT"},
    {Token::Type::QUESTION, "QUESTION"},
    {Token::Type::RBRACE, "RBRACE"},
    {Token::Type::RBRACKET, "RBRACKET"},
    {Token::Type::RETURN, "RETURN"},
    {Token::Type::RPAREN, "RPAREN"},
    {Token::Type::SELF, "SELF"},
    {Token::Type::SEMICOLON, "SEMICOLON"},
    {Token::Type::SLASH, "SLASH"},
    {Token::Type::SLASH_EQUAL, "SLASH_EQUAL"},
    {Token::Type::STAR, "STAR"},
    {Token::Type::STAR_EQUAL, "STAR_EQUAL"},
    {Token::Type::STRUCT, "STRUCT"},
    {Token::Type::STR_LITERAL, "STR_LITERAL"},
    {Token::Type::TYPE_LITERAL, "TYPE_LITERAL"},
    {Token::Type::VAR, "VAR"},
    {Token::Type::WHILE, "WHILE"},
    {Token::Type::AND, "AND"},
    {Token::Type::XOR, "XOR"},
    {Token::Type::OR, "OR"},
    {Token::Type::NOT, "NOT"},
};

void Token::print_token()
{
    std::cout << "{ "
              << "token_type: " << token_strings.at(this->token_type)
              << ",  lexeme: " << this->lexeme
              << " }"
              << std::endl;
}