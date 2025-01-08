#include "callable.h"
#include "visitors/interpreter.h"
#include "sym_table.h"
#include "value.h"
#include "exceptions/bird_exception.h"

void Callable::call(Interpreter *interpreter, std::vector<std::shared_ptr<Expr>> args)
{
    // create new scope
    if (args.size() != this->param_list.size())
    {
        throw BirdException("Mismatched arguments, expected: " + std::to_string(param_list.size()) + ", found: " + std::to_string(args.size()));
    }

    std::vector<Value> evaluated_args;
    for (auto &arg : args)
    {
        arg->accept(interpreter);
        auto value = interpreter->stack.pop();
        evaluated_args.push_back(value);
    }

    interpreter->env.push_env();

    for (int i = 0; i < this->param_list.size(); i++)
    {
        if (param_list[i].second.lexeme == "int" && !is_type<int>(evaluated_args[i]))
            throw BirdException("Type mismatch");

        if (param_list[i].second.lexeme == "str" && !is_type<std::string>(evaluated_args[i]))
            throw BirdException("Type mismatch");

        if (param_list[i].second.lexeme == "bool" && !is_type<bool>(evaluated_args[i]))
            throw BirdException("Type mismatch");

        if (param_list[i].second.lexeme == "float" && !is_type<double>(evaluated_args[i]))
            throw BirdException("Type mismatch");

        interpreter->env.declare(param_list[i].first.lexeme, evaluated_args[i]);
    }

    for (auto &stmt : dynamic_cast<Block *>(this->block.get())->stmts)
    {
        try
        {
            stmt->accept(interpreter);
        }
        catch (ReturnException e)
        {
            interpreter->env.pop_env();
            return;
        }
    }

    interpreter->env.pop_env();
}