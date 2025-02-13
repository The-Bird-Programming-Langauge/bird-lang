#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <iostream>
#include <algorithm>
#include <set>

#include "ast_node/index.h"
#include "hoist_visitor.h"

#include "sym_table.h"
#include "exceptions/bird_exception.h"
#include "exceptions/return_exception.h"
#include "exceptions/break_exception.h"
#include "exceptions/continue_exception.h"
#include "value.h"
#include "callable.h"
#include "stack.h"
#include "type.h"
#include "bird_type.h"

/*
 * Visitor that interprets and evaluates the AST
 */
class Interpreter : public Visitor
{

public:
    Environment<Value> env;
    Environment<Callable> call_table;
    Environment<std::shared_ptr<BirdType>> type_table;
    Stack<Value> stack;
    std::set<std::string> struct_names;

    Interpreter()
    {
        this->env.push_env();
        this->call_table.push_env();
        this->type_table.push_env();
    }

    void evaluate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        HoistVisitor hoist_visitor(&this->struct_names);
        hoist_visitor.hoist(stmts);

        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
                continue;
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
                continue;
            }

            if (auto assign_expr = dynamic_cast<AssignExpr *>(stmt.get()))
            {
                assign_expr->accept(this);
                continue;
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                continue;
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                continue;
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                continue;
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                continue;
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                continue;
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                continue;
            }

            if (auto func = dynamic_cast<Func *>(stmt.get()))
            {
                func->accept(this);
                continue;
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
                continue;
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                continue;
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                continue;
            }

            if (auto type_stmt = dynamic_cast<TypeStmt *>(stmt.get()))
            {
                type_stmt->accept(this);
                continue;
            }

            if (auto struct_decl = dynamic_cast<StructDecl *>(stmt.get()))
            {
                struct_decl->accept(this);
                continue;
            }
        }

        while (!this->stack.empty())
        {
            this->stack.pop();
        }
    }

    void visit_block(Block *block)
    {
        this->env.push_env();
        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }

        this->env.pop_env();
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        auto result = std::move(this->stack.pop());
        result.is_mutable = true;

        this->env.declare(decl_stmt->identifier.lexeme, std::move(result));
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        auto previous_value = this->env.get(assign_expr->identifier.lexeme);

        assign_expr->value->accept(this);
        auto value = std::move(this->stack.pop());

        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            previous_value = value;
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            previous_value = previous_value + value;
            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            previous_value = previous_value - value;
            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            previous_value = previous_value * value;
            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            previous_value = previous_value / value;
            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            previous_value = previous_value % value;
            break;
        }
        default:
            throw std::runtime_error("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
        }

        this->env.set(assign_expr->identifier.lexeme, previous_value);
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = std::move(this->stack.pop());

            if (is_type<bool>(result)) {
                std::cout << (as_type<bool>(result) ? "true" : "false");
            } else {
                std::cout << result;
            }
        }
        std::cout << std::endl;
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);

        auto result = std::move(this->stack.pop());

        this->env.declare(const_stmt->identifier.lexeme, std::move(result));
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->condition->accept(this);
        auto condition_result = std::move(this->stack.pop());

        auto num_envs = this->env.envs.size();

        while (as_type<bool>(condition_result))
        {
            try
            {
                while_stmt->stmt->accept(this);
            }
            catch (BreakException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                break;
            }
            catch (ContinueException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                while_stmt->condition->accept(this);
                condition_result = std::move(this->stack.pop());
                continue;
            }

            while_stmt->condition->accept(this);
            condition_result = std::move(this->stack.pop());
        }
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        this->env.push_env();

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        auto num_envs = this->env.envs.size();

        while (true)
        {
            if (for_stmt->condition.has_value())
            {
                for_stmt->condition.value()->accept(this);
                auto condition_result = std::move(this->stack.pop());

                if (!as_type<bool>(condition_result.data))
                {
                    break;
                }
            }

            try
            {
                for_stmt->body->accept(this);
            }
            catch (BreakException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                break;
            }
            catch (ContinueException e)
            {
                auto previous_size = this->env.envs.size();
                for (int i = 0; i < previous_size - num_envs; i++)
                {
                    this->env.pop_env();
                }

                if (for_stmt->increment.has_value())
                {
                    for_stmt->increment.value()->accept(this);
                }
                continue;
            }

            if (for_stmt->increment.has_value())
            {
                for_stmt->increment.value()->accept(this);
            }
        }

        this->env.pop_env();
    }

    void visit_binary(Binary *binary)
    {
        switch (binary->op.token_type)
        {
        case Token::Type::AND:
        case Token::Type::OR:
            visit_binary_short_circuit(binary);
            break;
        default:
            visit_binary_normal(binary);
        }
    }

    void visit_binary_short_circuit(Binary *binary)
    {
        binary->left->accept(this);
        auto left = std::move(this->stack.pop());

        switch (binary->op.token_type)
        {
        case Token::Type::AND:
        {
            if (!is_type<bool>(left))
            {
                throw BirdException("The 'and' binary operator could not be used to interpret these values.");
            }
            if (as_type<bool>(left) == false)
            {
                this->stack.push(Value(false));
            }
            else
            {
                binary->right->accept(this);
            }
            break;
        }
        case Token::Type::OR:
        {
            if (!is_type<bool>(left))
            {
                throw BirdException("The 'or' binary operator could not be used to interpret these values.");
            }
            if (as_type<bool>(left) == true)
            {
                this->stack.push(Value(true));
            }
            else
            {
                binary->right->accept(this);
            }
            break;
        }
        default:
        {
            throw std::runtime_error("Undefined binary short-circuit operator.");
        }
        }
    }

    void visit_binary_normal(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = std::move(this->stack.pop());
        auto left = std::move(this->stack.pop());

        switch (binary->op.token_type)
        {
        case Token::Type::XOR:
        {
            this->stack.push(left != right);
            break;
        }
        case Token::Type::PLUS:
        {
            this->stack.push(left + right);
            break;
        }
        case Token::Type::MINUS:
        {
            this->stack.push(left - right);
            break;
        }
        case Token::Type::SLASH:
        {
            this->stack.push(left / right);
            break;
        }
        case Token::Type::STAR:
        {
            this->stack.push(left * right);
            break;
        }
        case Token::Type::GREATER:
        {
            this->stack.push(left > right);
            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            this->stack.push(left >= right);
            break;
        }
        case Token::Type::LESS:
        {
            this->stack.push(left < right);
            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            this->stack.push(left <= right);
            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            this->stack.push(left != right);
            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            this->stack.push(left == right);
            break;
        }
        case Token::Type::PERCENT:
        {
            this->stack.push(left % right);
            break;
        }
        default:
        {
            throw std::runtime_error("Undefined binary operator.");
        }
        }
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = std::move(this->stack.pop());

        switch (unary->op.token_type)
        {
        case Token::Type::MINUS:
        {
            this->stack.push(-expr);
            break;
        }
        case Token::Type::QUESTION:
        {
            this->stack.push(expr != Value(nullptr));
            break;
        }
        case Token::Type::NOT:
        {
            this->stack.push(!expr);
            break;
        }
        default:
        {
            throw std::runtime_error("Undefined unary operator.");
        }
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
            this->stack.push(Value(
                variant(std::stod(primary->value.lexeme))));
            break;
        case Token::Type::BOOL_LITERAL:
            this->stack.push(Value(
                variant(primary->value.lexeme == "true" ? true : false)));
            break;
        case Token::Type::STR_LITERAL:
            this->stack.push(Value(
                variant(primary->value.lexeme)));
            break;
        case Token::Type::INT_LITERAL:
            this->stack.push(Value(
                variant(std::stoi(primary->value.lexeme))));
            break;
        case Token::Type::IDENTIFIER:
            this->stack.push(
                this->env.get(primary->value.lexeme));
            break;
        default:
            throw std::runtime_error("undefined primary value");
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        auto result = std::move(this->stack.pop());

        if (as_type<bool>(result))
            ternary->true_expr->accept(this);
        else
            ternary->false_expr->accept(this);
    }

    void visit_func(Func *func)
    {
        Callable callable = Callable(func->param_list,
                                     func->block,
                                     func->return_type);

        this->call_table.declare(func->identifier.lexeme, callable);
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        auto result = std::move(this->stack.pop());

        if (as_type<bool>(result))
            if_stmt->then_branch->accept(this);
        else if (if_stmt->else_branch.has_value())
            if_stmt->else_branch.value()->accept(this);
    }

    void visit_call(Call *call)
    {
        auto callable = this->call_table.get(call->identifier.lexeme);
        callable.call(this, call->args);
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
        }

        throw ReturnException();
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        throw BreakException();
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        throw ContinueException();
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        if (type_stmt->type_is_literal)
        {
            this->type_table.declare(type_stmt->identifier.lexeme, token_to_bird_type(type_stmt->type_token));
        }
        else
        {
            this->type_table.declare(type_stmt->identifier.lexeme, this->type_table.get(type_stmt->type_token.lexeme));
        }
    }

    void visit_subscript(Subscript *Subscript)
    {
        Subscript->subscriptable->accept(this);
        auto subscriptable = this->stack.pop();

        Subscript->index->accept(this);
        auto index = this->stack.pop();

        this->stack.push(subscriptable[index]);
    }

    void visit_struct_decl(StructDecl *struct_decl)
    {
        std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> struct_fields;
        std::transform(struct_decl->fields.begin(), struct_decl->fields.end(), std::back_inserter(struct_fields), [&](std::pair<std::string, Token> field)
                       { 
                        if (this->type_table.contains(field.second.lexeme))
                        {
                            return std::make_pair(field.first, this->type_table.get(field.second.lexeme));
                        } 

                        if (this->struct_names.find(field.second.lexeme) != this->struct_names.end()) {
                            return std::make_pair(field.first, std::shared_ptr<BirdType>(new PlaceholderType(field.second.lexeme)));
                        }

                            return std::make_pair(field.first, token_to_bird_type(field.second)); });

        this->type_table.declare(struct_decl->identifier.lexeme, std::make_shared<StructType>(struct_decl->identifier.lexeme, std::move(struct_fields)));
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        direct_member_access->accessable->accept(this);
        auto accessable = this->stack.pop();

        if (is_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(accessable))
        {
            auto struct_type = as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(accessable);
            this->stack.push((*struct_type.get())[direct_member_access->identifier.lexeme]);
        }
        else
        {
            throw std::runtime_error("Cannot access member of non-struct type.");
        }
    }

    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        std::shared_ptr<std::unordered_map<std::string, Value>> struct_instance = std::make_shared<std::unordered_map<std::string, Value>>();
        auto type = this->type_table.get(struct_initialization->identifier.lexeme);

        auto struct_type = safe_dynamic_pointer_cast<StructType>(type);

        for (auto &field : struct_type->fields)
        {
            bool found = false;
            for (auto &field_assignment : struct_initialization->field_assignments)
            {
                if (field.first == field_assignment.first)
                {
                    found = true;
                    field_assignment.second->accept(this);
                    auto result = this->stack.pop();

                    (*struct_instance)[field_assignment.first] = result;
                    break;
                }
            }

            if (!found)
            {
                if (field.second->type == BirdTypeType::BOOL)
                {
                    (*struct_instance)[field.first] = Value(false);
                }
                else if (field.second->type == BirdTypeType::INT)
                {
                    (*struct_instance)[field.first] = Value(0);
                }
                else if (field.second->type == BirdTypeType::FLOAT)
                {
                    (*struct_instance)[field.first] = Value(0.0);
                }
                else if (field.second->type == BirdTypeType::STRING)
                {
                    (*struct_instance)[field.first] = Value("");
                }
                else if (field.second->type == BirdTypeType::STRUCT)
                {
                    (*struct_instance)[field.first] = Value(nullptr);
                }
                else if (field.second->type == BirdTypeType::PLACEHOLDER)
                {
                    (*struct_instance)[field.first] = Value(nullptr);
                }
                else
                {
                    throw std::runtime_error("Cannot assign member of non-struct type.");
                }
            }
        }

        this->stack.push(Value(struct_instance));
    }

    void visit_member_assign(MemberAssign *member_assign)
    {
        member_assign->accessable->accept(this);
        auto accessable = this->stack.pop();

        if (is_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(accessable))
        {
            auto struct_type = as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(accessable);

            member_assign->value->accept(this);
            auto value = this->stack.pop();

            (*struct_type.get())[member_assign->identifier.lexeme] = value;
        }
        else
        {
            throw std::runtime_error("Cannot assign member of non-struct type.");
        }
    }

    void visit_as_cast(AsCast *as_cast)
    {
        as_cast->expr->accept(this);
        auto expr = this->stack.pop();

        if (as_cast->type.lexeme == "int" && is_type<double>(expr))
        {
            this->stack.push(Value((int)as_type<double>(expr)));
            return;
        }

        if (as_cast->type.lexeme == "float" && is_type<int>(expr))
        {
            this->stack.push(Value((double)as_type<int>(expr)));
            return;
        }

        this->stack.push(expr);
    }
};