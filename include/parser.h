#pragma once

#include <vector>
#include <memory>

#include "lexer.h"

// foward declarations
class Expr;
class Stmt;

/*
 * Parser will take the tokens as input
 * and, using the grammar, create an AST
 * (Abstract Syntax Tree) and report as many
 * errors as it can find
 */
class Parser
{
public:
    const std::vector<Token> tokens;
    unsigned int position;

    Parser(std::vector<Token> tokens);

    std::vector<std::unique_ptr<Stmt>> parse();

    std::unique_ptr<Stmt> stmt();

    std::unique_ptr<Stmt> expr_stmt();

    std::unique_ptr<Stmt> var_decl();

    std::unique_ptr<Stmt> block();

    std::unique_ptr<Stmt> print_stmt();

    std::unique_ptr<Expr> expr();

    std::unique_ptr<Expr> term();

    std::unique_ptr<Expr> factor();

    std::unique_ptr<Expr> unary();

    std::unique_ptr<Expr> primary();

    Token advance();

    Token peek();

    bool is_at_end();
};
