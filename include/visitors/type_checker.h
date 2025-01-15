#pragma once

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#include "ast_node/index.h"

#include "sym_table.h"
#include "exceptions/bird_exception.h"
#include "exceptions/return_exception.h"
#include "exceptions/user_error_tracker.h"
#include "bird_type.h"
#include "stack.h"
#include "type.h"

/*
 * Visitor that checks types of the AST
 */
class TypeChecker : public Visitor
{
public:
    Environment<BirdType> env;
    Environment<BirdFunction> call_table;
    Environment<Type> type_table;
    Stack<BirdType> stack;
    std::optional<BirdType> return_type;
    UserErrorTracker *user_error_tracker;

    TypeChecker(UserErrorTracker *user_error_tracker) : user_error_tracker(user_error_tracker)
    {
        this->env.push_env();
        this->call_table.push_env();
        this->type_table.push_env();
    }

    std::map<Token::Type, Token::Type> assign_to_binary_map = {
        {Token::Type::PLUS_EQUAL, Token::Type::PLUS},
        {Token::Type::MINUS_EQUAL, Token::Type::MINUS},
        {Token::Type::STAR_EQUAL, Token::Type::STAR},
        {Token::Type::SLASH_EQUAL, Token::Type::SLASH},
        {Token::Type::PERCENT_EQUAL, Token::Type::PERCENT},
    };

    // map of binary operations and their return types
    std::map<Token::Type, std::map<std::pair<BirdType, BirdType>, BirdType>> binary_operations = {
        {Token::Type::PLUS, {
                                {{BirdType::INT, BirdType::INT}, BirdType::INT},
                                {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::FLOAT},
                                {{BirdType::INT, BirdType::FLOAT}, BirdType::INT},
                                {{BirdType::FLOAT, BirdType::INT}, BirdType::FLOAT},
                                {{BirdType::STRING, BirdType::STRING}, BirdType::STRING},
                            }},
        {Token::Type::MINUS, {
                                 {{BirdType::INT, BirdType::INT}, BirdType::INT},
                                 {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::FLOAT},
                                 {{BirdType::INT, BirdType::FLOAT}, BirdType::INT},
                                 {{BirdType::FLOAT, BirdType::INT}, BirdType::FLOAT},
                             }},
        {Token::Type::STAR, {
                                {{BirdType::INT, BirdType::INT}, BirdType::INT},
                                {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::FLOAT},
                                {{BirdType::INT, BirdType::FLOAT}, BirdType::INT},
                                {{BirdType::FLOAT, BirdType::INT}, BirdType::FLOAT},
                            }},
        {Token::Type::SLASH, {
                                 {{BirdType::INT, BirdType::INT}, BirdType::INT},
                                 {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::FLOAT},
                                 {{BirdType::INT, BirdType::FLOAT}, BirdType::INT},
                                 {{BirdType::FLOAT, BirdType::INT}, BirdType::FLOAT},
                             }},
        {Token::Type::EQUAL_EQUAL, {
                                       {{BirdType::INT, BirdType::INT}, BirdType::BOOL},
                                       {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::BOOL},
                                       {{BirdType::INT, BirdType::FLOAT}, BirdType::BOOL},
                                       {{BirdType::FLOAT, BirdType::INT}, BirdType::BOOL},
                                       {{BirdType::STRING, BirdType::STRING}, BirdType::BOOL},
                                       {{BirdType::BOOL, BirdType::BOOL}, BirdType::BOOL},
                                   }},
        {Token::Type::BANG_EQUAL, {
                                      {{BirdType::INT, BirdType::INT}, BirdType::BOOL},
                                      {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::BOOL},
                                      {{BirdType::INT, BirdType::FLOAT}, BirdType::BOOL},
                                      {{BirdType::FLOAT, BirdType::INT}, BirdType::BOOL},
                                      {{BirdType::STRING, BirdType::STRING}, BirdType::BOOL},
                                      {{BirdType::BOOL, BirdType::BOOL}, BirdType::BOOL},
                                  }},
        {Token::Type::GREATER, {
                                   {{BirdType::INT, BirdType::INT}, BirdType::BOOL},
                                   {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::BOOL},
                                   {{BirdType::INT, BirdType::FLOAT}, BirdType::BOOL},
                                   {{BirdType::FLOAT, BirdType::INT}, BirdType::BOOL},
                               }},
        {Token::Type::GREATER_EQUAL, {
                                         {{BirdType::INT, BirdType::INT}, BirdType::BOOL},
                                         {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::BOOL},
                                         {{BirdType::INT, BirdType::FLOAT}, BirdType::BOOL},
                                         {{BirdType::FLOAT, BirdType::INT}, BirdType::BOOL},
                                     }},
        {Token::Type::LESS, {
                                {{BirdType::INT, BirdType::INT}, BirdType::BOOL},
                                {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::BOOL},
                                {{BirdType::INT, BirdType::FLOAT}, BirdType::BOOL},
                                {{BirdType::FLOAT, BirdType::INT}, BirdType::BOOL},
                            }},
        {Token::Type::LESS_EQUAL, {
                                      {{BirdType::INT, BirdType::INT}, BirdType::BOOL},
                                      {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::BOOL},
                                      {{BirdType::INT, BirdType::FLOAT}, BirdType::BOOL},
                                      {{BirdType::FLOAT, BirdType::INT}, BirdType::BOOL},
                                  }},
        {Token::Type::PERCENT, {
                                   {{BirdType::INT, BirdType::INT}, BirdType::INT},
                                   {{BirdType::FLOAT, BirdType::FLOAT}, BirdType::FLOAT},
                                   {{BirdType::INT, BirdType::FLOAT}, BirdType::INT},
                                   {{BirdType::FLOAT, BirdType::INT}, BirdType::FLOAT},
                               }},
    };

    void check_types(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
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

        if (result == BirdType::VOID)
        {
            this->user_error_tracker->type_error("cannot declare void type", decl_stmt->identifier);
            this->env.declare(decl_stmt->identifier.lexeme, BirdType::ERROR);
            return;
        }

        if (decl_stmt->type_token.has_value())
        {
            BirdType type;
            if (decl_stmt->type_is_literal)
            {
                type = this->get_type_from_token(decl_stmt->type_token.value());
            }
            else
            {
                type = this->get_type_from_token(this->type_table.get(decl_stmt->type_token.value().lexeme).type);
            }

            if (type != result)
            {
                if (result == BirdType::INT && type == BirdType::FLOAT)
                {
                    this->env.declare(decl_stmt->identifier.lexeme, BirdType::INT);
                    return;
                }
                if (result == BirdType::FLOAT && type == BirdType::INT)
                {
                    this->env.declare(decl_stmt->identifier.lexeme, BirdType::FLOAT);
                    return;
                }
                this->user_error_tracker->type_mismatch("in declaration", decl_stmt->type_token.value());

                this->env.declare(decl_stmt->identifier.lexeme, BirdType::ERROR);
                return;
            }
        }

        this->env.declare(decl_stmt->identifier.lexeme, result);
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        if (!this->env.contains(assign_expr->identifier.lexeme))
        {
            this->user_error_tracker->type_error("identifier not declared", assign_expr->identifier);
            this->env.set(assign_expr->identifier.lexeme, BirdType::ERROR);
            return;
        }

        assign_expr->value->accept(this);
        auto result = std::move(this->stack.pop());

        auto previous = this->env.get(assign_expr->identifier.lexeme);

        if (assign_expr->assign_operator.token_type == Token::Type::EQUAL)
        {
            if (previous != result)
            {
                if (previous == BirdType::INT && result == BirdType::FLOAT)
                {
                    this->env.set(assign_expr->identifier.lexeme, BirdType::INT);
                    return;
                }
                if (previous == BirdType::FLOAT && result == BirdType::INT)
                {
                    this->env.set(assign_expr->identifier.lexeme, BirdType::INT);
                    return;
                }

                this->user_error_tracker->type_mismatch("in assignment", assign_expr->assign_operator);
                this->env.set(assign_expr->identifier.lexeme, BirdType::INT);
                return;
            }

            this->env.set(assign_expr->identifier.lexeme, result);
            return;
        }

        auto binary_operator = this->assign_to_binary_map.at(assign_expr->assign_operator.token_type);
        auto type_map = this->binary_operations.at(binary_operator);

        if (type_map.find({previous, result}) == type_map.end())
        {
            this->user_error_tracker->type_mismatch("in assignment", assign_expr->assign_operator);
            this->env.set(assign_expr->identifier.lexeme, BirdType::ERROR);
            return;
        }

        auto new_type = type_map.at({previous, result});
        this->env.set(assign_expr->identifier.lexeme, new_type);
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
        }
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
        auto result = std::move(this->stack.pop());

        if (result == BirdType::VOID)
        {
            this->user_error_tracker->type_error("cannot declare void type", const_stmt->identifier);
            this->env.declare(const_stmt->identifier.lexeme, BirdType::ERROR);
            return;
        }

        if (const_stmt->type_token.has_value())
        {
            BirdType type;
            if (const_stmt->type_is_literal)
            {
                type = this->get_type_from_token(const_stmt->type_token.value());
            }
            else
            {
                type = this->get_type_from_token(this->type_table.get(const_stmt->type_token.value().lexeme).type);
            }

            if (type != result)
            {
                this->user_error_tracker->type_mismatch("in declaration", const_stmt->type_token.value());
                this->env.declare(const_stmt->identifier.lexeme, BirdType::ERROR);
                return;
            }
        }

        this->env.declare(const_stmt->identifier.lexeme, result);
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->condition->accept(this);
        auto condition_result = std::move(this->stack.pop());

        if (condition_result != BirdType::BOOL)
        {
            this->user_error_tracker->type_error("expected bool in while statement condition", while_stmt->while_token);
        }

        while_stmt->stmt->accept(this);
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        this->env.push_env();

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
            auto condition_result = std::move(this->stack.pop());

            if (condition_result != BirdType::BOOL)
            {
                this->user_error_tracker->type_error("expected bool in for statement condition", for_stmt->for_token);
            }
        }

        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
        }

        this->env.pop_env();
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = std::move(this->stack.pop());

        // TODO: investigate these moves
        auto left = std::move(this->stack.pop());

        auto operator_options = this->binary_operations.at(binary->op.token_type);
        if (operator_options.find({left, right}) == operator_options.end())
        {
            this->user_error_tracker->type_mismatch("in binary operation", binary->op);
            this->stack.push(BirdType::ERROR);
            return;
        }

        this->stack.push(operator_options.at({left, right}));
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto result = std::move(this->stack.pop());

        if (result == BirdType::INT)
        {
            this->stack.push(BirdType::INT);
        }
        else if (result == BirdType::FLOAT)
        {
            this->stack.push(BirdType::FLOAT);
        }
        else
        {
            this->user_error_tracker->type_error("expected int or float in unary operation", unary->op);
            this->stack.push(BirdType::ERROR);
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
        {
            this->stack.push(BirdType::FLOAT);
            break;
        }
        case Token::Type::INT_LITERAL:
        {
            this->stack.push(BirdType::INT);
            break;
        }
        case Token::Type::BOOL_LITERAL:
        {
            this->stack.push(BirdType::BOOL);
            break;
        }
        case Token::Type::STR_LITERAL:
        {
            this->stack.push(BirdType::STRING);
            break;
        }
        case Token::Type::IDENTIFIER:
        {
            this->stack.push(
                this->env.get(primary->value.lexeme));
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);
        auto condition = std::move(this->stack.pop());

        ternary->true_expr->accept(this);
        auto true_expr = std::move(this->stack.pop());

        ternary->false_expr->accept(this);
        auto false_expr = std::move(this->stack.pop());

        if (true_expr != false_expr)
        {
            this->user_error_tracker->type_mismatch("in ternary operation", ternary->ternary_token);
            true_expr = BirdType::ERROR;
        }

        if (condition != BirdType::BOOL)
        {
            this->user_error_tracker->type_error("expected bool in ternary condition", ternary->ternary_token);
            this->stack.push(BirdType::ERROR);
        }
        else
        {
            this->stack.push(true_expr);
        }
    }

    BirdType get_type_from_token(Token token)
    {
        auto type = token.lexeme;
        if (type == "int")
        {
            return BirdType::INT;
        }
        else if (type == "float")
        {
            return BirdType::FLOAT;
        }
        else if (type == "bool")
        {
            return BirdType::BOOL;
        }
        else if (type == "str")
        {
            return BirdType::STRING;
        }
        else if (type == "void")
        {
            return BirdType::VOID;
        }
        else
        {
            if (this->type_table.contains(type))
            {
                return this->get_type_from_token(this->type_table.get(type).type);
            }

            this->user_error_tracker->type_error("unknown type", token);
            return BirdType::ERROR;
        }
    }

    bool is_bird_type(Token token)
    {
        return token.lexeme == "int" || token.lexeme == "float" || token.lexeme == "bool" || token.lexeme == "str" || token.lexeme == "void";
    }

    void visit_func(Func *func)
    {
        std::vector<BirdType>
            params;
        std::transform(func->param_list.begin(), func->param_list.end(), std::back_inserter(params), [&](auto param)
                       { return this->get_type_from_token(param.second); });

        BirdType ret = func->return_type.has_value() ? this->get_type_from_token(func->return_type.value()) : BirdType::VOID;
        auto previous_return_type = this->return_type;
        this->return_type = ret;

        this->call_table.declare(func->identifier.lexeme, BirdFunction(params, ret));
        this->env.push_env();

        for (auto &param : func->param_list)
        {
            if (!this->is_bird_type(param.second))
            {
                this->env.declare(param.first.lexeme, this->get_type_from_token(this->type_table.get(param.second.lexeme).type));
                continue;
            }

            this->env.declare(param.first.lexeme, this->get_type_from_token(param.second));
        }

        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts) // TODO: figure out how not to dynamic cast
        {
            stmt->accept(this);
        }

        this->return_type = previous_return_type;
        this->env.pop_env();
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        auto condition = std::move(this->stack.pop());

        if (condition != BirdType::BOOL)
        {
            this->user_error_tracker->type_error("expected bool in if statement condition", if_stmt->if_token);
        }

        if_stmt->then_branch->accept(this);

        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch.value()->accept(this);
        }
    }

    void visit_call(Call *call)
    {
        auto function = this->call_table.get(call->identifier.lexeme);

        for (int i = 0; i < function.params.size(); i++)
        {
            call->args[i]->accept(this);
            auto arg = std::move(this->stack.pop());

            if (arg == BirdType::INT && function.params[i] == BirdType::FLOAT)
            {
                continue;
            }

            if (arg == BirdType::FLOAT && function.params[i] == BirdType::INT)
            {
                continue;
            }

            if (arg != function.params[i])
            {
                this->user_error_tracker->type_mismatch("in function call", call->identifier);
            }
        }

        this->stack.push(function.ret);
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            auto result = std::move(this->stack.pop());

            if (result == BirdType::INT && this->return_type == BirdType::FLOAT)
            {
                this->stack.push(BirdType::FLOAT);
                return;
            }

            if (result == BirdType::FLOAT && this->return_type == BirdType::INT)
            {
                this->stack.push(BirdType::INT);
                return;
            }

            if (result != this->return_type)
            {
                this->user_error_tracker->type_mismatch("in return statement", return_stmt->return_token);
            }
        }
        else
        {
            if (this->return_type != BirdType::VOID)
            {
                this->user_error_tracker->type_error("expected return value in non-void function", return_stmt->return_token);
            }
        }
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        // do nothing
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        // do nothing
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        if (type_stmt->type_is_literal)
        {
            this->type_table.declare(type_stmt->identifier.lexeme, Type(type_stmt->type_token));
        }
        else
        {
            this->type_table.declare(type_stmt->identifier.lexeme, Type(this->type_table.get(type_stmt->type_token.lexeme).type));
        }
    }

    void visit_subscript(Subscript *subscript)
    {
        subscript->subscriptable->accept(this);
        auto subscriptable = this->stack.pop();

        subscript->index->accept(this);
        auto index = this->stack.pop();

        if (subscriptable != BirdType::STRING)
        {
            this->user_error_tracker->type_error("expected string in subscriptable", subscript->subscript_token);
            this->stack.push(BirdType::ERROR);
            return;
        }

        if (index != BirdType::INT)
        {
            this->user_error_tracker->type_error("expected int in subscript index", subscript->subscript_token);
            this->stack.push(BirdType::ERROR);
            return;
        }

        this->stack.push(BirdType::INT);
    }

    void visit_struct_decl(StructDecl *struct_decl)
    {
        this->type_table.declare(struct_decl->identifier.lexeme, Type(struct_decl->identifier));
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        // direct_member_access->accessable->accept(this);
        // auto accessable = this->stack.pop();

        // if (accessable == BirdType::ERROR)
        // {
        //     this->stack.push(BirdType::ERROR);
        //     return;
        // }

        // if (accessable != BirdType::STRUCT)
        // {
        //     this->user_error_tracker->type_error("expected struct in direct member access", direct_member_access->identifier);
        //     this->stack.push(BirdType::ERROR);
        //     return;
        // }

        // auto struct_type = this->type_table.get(accessable.lexeme).type;
        // auto field = struct_type.fields.find(direct_member_access->identifier.lexeme);

        // if (field == struct_type.fields.end())
        // {
        //     this->user_error_tracker->type_error("field not found in struct", direct_member_access->identifier);
        //     this->stack.push(BirdType::ERROR);
        //     return;
        // }

        // this->stack.push(field->second);
    }

    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        // std::shared_ptr<std::unordered_map<std::string, BirdType>> struct_instance = std::make_shared<std::unordered_map<std::string, BirdType>>();

        // for (auto &field_assignment : struct_initialization->field_assignments)
        // {
        //     field_assignment.second->accept(this);
        //     auto result = this->stack.pop();

        //     (*struct_instance)[field_assignment.first] = result;
        // }

        // this->stack.push(BirdType::STRUCT);
        // this->stack.push(
        //     this->type_table.get(struct_initialization->identifier.lexeme));

        // this->stack.push(BirdType::STRUCT);
    }
};
