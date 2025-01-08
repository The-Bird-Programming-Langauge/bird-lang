#pragma once
#include <vector>
#include "../src/lexer.cpp"
#include "exceptions/user_error_tracker.h"
#include "lexer.h"

std::vector<Token> lex_code(const std::string &code)
{
    UserErrorTracker error_tracker(code);
    Lexer lexer(code, &error_tracker);
    return lexer.lex();
}