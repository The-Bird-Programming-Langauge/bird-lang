#pragma once

#include <vector>
#include <memory>
#include <optional>

#include "./token.h"

class Stmt;
class Expr;
class Interpreter;

class Callable
{
public:
    std::vector<std::pair<Token, Token>> param_list;
    std::shared_ptr<Stmt> block;
    std::optional<Token> return_type;

    Callable(
        std::vector<std::pair<Token, Token>> param_list,
        std::shared_ptr<Stmt> block,
        std::optional<Token> return_type)
        : param_list(param_list),
          block(std::move(block)),
          return_type(return_type) {}
    Callable() = default;
    Callable(const Callable &other) : param_list(other.param_list),
                                      block(std::move(other.block)),
                                      return_type(other.return_type)
    {
    }

    void call(Interpreter *Interpreter, std::vector<std::shared_ptr<Expr>>);
};

struct SemanticCallable
{
    int param_count;

    SemanticCallable(int param_count) : param_count(param_count) {}
    SemanticCallable() = default;
};