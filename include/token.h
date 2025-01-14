#pragma once

#include <string>

class Token
{
public:
    enum Type
    {
        // keywords
        VAR,
        PRINT,
        IF,
        ELSE,
        WHILE,
        FOR,
        DO,
        FN,
        RETURN,
        BREAK,
        CONTINUE,
        CONST,
        TYPE,

        // types
        TYPE_LITERAL,

        // single character
        STAR,
        SLASH,
        PLUS,
        MINUS,
        SEMICOLON,
        QUESTION, // adding ternary tokens to lexer

        PERCENT,
        EQUAL,
        COMMA,
        RBRACE,
        LBRACE,
        RPAREN,
        LPAREN,
        COLON,
        GREATER,
        LESS,
        BANG,

        // two character
        GREATER_EQUAL,
        LESS_EQUAL,
        EQUAL_EQUAL,
        BANG_EQUAL,
        ARROW,
        PLUS_EQUAL,
        MINUS_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        PERCENT_EQUAL,

        // values
        IDENTIFIER,
        NUMBER,
        INT_LITERAL,
        FLOAT_LITERAL,
        BOOL_LITERAL,
        STR_LITERAL,

        // end
        END
    };

    Type token_type;
    std::string lexeme;
    unsigned int line_num;
    unsigned int char_num;

    void print_token();

    Token() = default;

    Token(Type token_type, std::string lexeme, unsigned int line_num, unsigned int char_num) : token_type(token_type), lexeme(lexeme), line_num(line_num), char_num(char_num) {}
};