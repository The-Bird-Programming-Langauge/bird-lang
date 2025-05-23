#pragma once

#include <string>

class Token {
public:
  enum Type {
    // keywords
    BREAK,
    CONST,
    CONTINUE,
    DO,
    ELSE,
    FN,
    FOR,
    IF,
    PRINT,
    RETURN,
    SELF,
    STRUCT,
    TYPE,
    VAR,
    WHILE,
    AS,
    AND,
    XOR,
    OR,
    NOT,
    MATCH,
    IN,
    IMPORT,
    FROM,
    NAMESPACE,

    // types
    INT,
    FLOAT,
    BOOL,
    STR,
    CHAR,
    VOID,

    // single character
    BANG,
    COLON,
    COMMA,
    DOT,
    EQUAL,
    GREATER,
    LBRACE,
    LBRACKET,
    LESS,
    LPAREN,
    MINUS,
    PERCENT,
    PLUS,
    QUESTION, // adding ternary tokens to lexer
    RBRACE,
    RBRACKET,
    RPAREN,
    SEMICOLON,
    SLASH,
    STAR,

    // two character
    ARROW,
    BANG_EQUAL,
    EQUAL_EQUAL,
    FAT_ARROW,
    GREATER_EQUAL,
    LESS_EQUAL,
    MINUS_EQUAL,
    PERCENT_EQUAL,
    PLUS_EQUAL,
    SLASH_EQUAL,
    STAR_EQUAL,
    COLON_COLON,

    // values
    TRUE,
    FALSE,
    FLOAT_LITERAL,
    IDENTIFIER,
    INT_LITERAL,
    NUMBER,
    STR_LITERAL,
    CHAR_LITERAL,

    // end
    END
  };

  Type token_type;
  std::string lexeme;
  unsigned int line_num;
  unsigned int char_num;

  void print_token();

  Token() = default;

  Token(Type token_type, std::string lexeme, unsigned int line_num,
        unsigned int char_num)
      : token_type(token_type), lexeme(lexeme), line_num(line_num),
        char_num(char_num) {}
};