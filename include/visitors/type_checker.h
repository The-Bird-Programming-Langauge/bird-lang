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
#include "visitor_adapter.h"
#include "hoist_visitor.h"

bool is_bird_type(Token token)
{
    return token.lexeme == "int" || token.lexeme == "float" || token.lexeme == "bool" || token.lexeme == "str" || token.lexeme == "void";
}

/*
 * Visitor that checks types of the AST
 */
class TypeChecker : public Visitor
{
public:
    Environment<std::shared_ptr<BirdType>> env;
    Environment<std::shared_ptr<BirdFunction>> call_table;

    Environment<std::shared_ptr<BirdType>> type_table;

    std::set<std::string> struct_names;

    Stack<std::shared_ptr<BirdType>> stack;
    std::optional<std::shared_ptr<BirdType>> return_type;
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
    std::map<Token::Type, std::map<std::pair<BirdTypeType, BirdTypeType>, BirdTypeType>> binary_operations = {
        {Token::Type::PLUS, {
                                {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::INT},
                                {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::FLOAT},
                                {{BirdTypeType::STRING, BirdTypeType::STRING}, BirdTypeType::STRING},
                            }},
        {Token::Type::MINUS, {
                                 {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::INT},
                                 {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::FLOAT},
                             }},
        {Token::Type::STAR, {
                                {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::INT},
                                {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::FLOAT},
                            }},
        {Token::Type::SLASH, {
                                 {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::INT},
                                 {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::FLOAT},
                             }},
        {Token::Type::EQUAL_EQUAL, {
                                       {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::BOOL},
                                       {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::BOOL},
                                       {{BirdTypeType::STRING, BirdTypeType::STRING}, BirdTypeType::BOOL},
                                       {{BirdTypeType::BOOL, BirdTypeType::BOOL}, BirdTypeType::BOOL},
                                   }},
        {Token::Type::BANG_EQUAL, {
                                      {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::BOOL},
                                      {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::BOOL},
                                      {{BirdTypeType::STRING, BirdTypeType::STRING}, BirdTypeType::BOOL},
                                      {{BirdTypeType::BOOL, BirdTypeType::BOOL}, BirdTypeType::BOOL},
                                  }},
        {Token::Type::GREATER, {
                                   {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::BOOL},
                                   {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::BOOL},
                               }},
        {Token::Type::GREATER_EQUAL, {
                                         {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::BOOL},
                                         {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::BOOL},
                                     }},
        {Token::Type::LESS, {
                                {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::BOOL},
                                {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::BOOL},
                            }},
        {Token::Type::LESS_EQUAL, {
                                      {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::BOOL},
                                      {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::BOOL},
                                  }},
        {Token::Type::PERCENT, {
                                   {{BirdTypeType::INT, BirdTypeType::INT}, BirdTypeType::INT},
                                   {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, BirdTypeType::FLOAT},
                               }},
    };

    void check_types(std::vector<std::unique_ptr<Stmt>> *stmts)
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
        auto result = this->stack.pop();

        if (result->type == BirdTypeType::VOID)
        {
            this->user_error_tracker->type_error("cannot declare void type", decl_stmt->identifier);
            this->env.declare(decl_stmt->identifier.lexeme, std::make_shared<ErrorType>());
            return;
        }

        if (decl_stmt->type_token.has_value())
        {
            std::shared_ptr<BirdType> type = this->get_type_from_token(decl_stmt->type_token.value());

            if (*type != *result)
            {
                this->user_error_tracker->type_mismatch("in declaration", decl_stmt->type_token.value());
                this->env.declare(decl_stmt->identifier.lexeme, std::make_unique<ErrorType>());
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
            this->env.set(assign_expr->identifier.lexeme, std::make_shared<ErrorType>());
            return;
        }

        assign_expr->value->accept(this);
        auto result = this->stack.pop();

        auto previous = this->env.get(assign_expr->identifier.lexeme);

        if (assign_expr->assign_operator.token_type == Token::Type::EQUAL)
        {
            if (*previous != *result)
            {
                this->user_error_tracker->type_mismatch("in assignment", assign_expr->assign_operator);
                this->env.set(assign_expr->identifier.lexeme, std::make_shared<ErrorType>());
                return;
            }

            this->env.set(assign_expr->identifier.lexeme, result);
            return;
        }

        auto binary_operator = this->assign_to_binary_map.at(assign_expr->assign_operator.token_type);
        auto type_map = this->binary_operations.at(binary_operator);

        if (type_map.find({previous->type, result->type}) == type_map.end())
        {
            this->user_error_tracker->type_mismatch("in assignment", assign_expr->assign_operator);
            this->env.set(assign_expr->identifier.lexeme, std::make_shared<ErrorType>());
            return;
        }

        auto new_type = type_map.at({previous->type, result->type});
        this->env.set(assign_expr->identifier.lexeme, bird_type_type_to_bird_type(new_type));
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
        auto result = this->stack.pop();

        if (result->type == BirdTypeType::VOID)
        {
            this->user_error_tracker->type_error("cannot declare void type", const_stmt->identifier);
            this->env.declare(const_stmt->identifier.lexeme, std::make_shared<ErrorType>());
            return;
        }

        if (const_stmt->type_token.has_value())
        {
            std::shared_ptr<BirdType> type = this->get_type_from_token(const_stmt->type_token.value());

            if (*type != *result)
            {
                this->user_error_tracker->type_mismatch("in declaration", const_stmt->type_token.value());
                this->env.declare(const_stmt->identifier.lexeme, std::make_shared<ErrorType>());
                return;
            }
        }

        this->env.declare(const_stmt->identifier.lexeme, result);
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->condition->accept(this);
        auto condition_result = this->stack.pop();

        if (condition_result->type != BirdTypeType::BOOL)
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
            auto condition_result = this->stack.pop();

            if (condition_result->type != BirdTypeType::BOOL)
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

        auto right = this->stack.pop();
        auto left = this->stack.pop();

        if (right->type == BirdTypeType::ALIAS && left->type == BirdTypeType::ALIAS)
        {
            if (right != left)
            {
                this->user_error_tracker->type_mismatch("in binary operation", binary->op);
                this->stack.push(std::make_shared<ErrorType>());
                return;
            }
            right = safe_dynamic_pointer_cast<AliasType>(right)->alias;
            left = safe_dynamic_pointer_cast<AliasType>(left)->alias;
        }

        auto operator_options = this->binary_operations.at(binary->op.token_type);
        if (operator_options.find({left->type, right->type}) == operator_options.end())
        {
            this->user_error_tracker->type_mismatch("in binary operation", binary->op);
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        this->stack.push(bird_type_type_to_bird_type(operator_options.at({left->type, right->type})));
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto result = this->stack.pop();
        switch (unary->op.token_type)
        {
        case Token::Type::MINUS:
        {
            if (result->type == BirdTypeType::FLOAT)
            {
                this->stack.push(std::make_shared<FloatType>());
            }
            else if (result->type == BirdTypeType::INT)
            {
                this->stack.push(std::make_shared<IntType>());
            }
            else
            {
                this->user_error_tracker->type_error("expected int or float in unary operation, found: " + bird_type_to_string(result), unary->op);
                this->stack.push(std::make_shared<ErrorType>());
            }
            break;
        }
        case Token::Type::QUESTION:
        {
            if (result->type == BirdTypeType::STRUCT)
            {
                this->stack.push(std::make_shared<BoolType>());
            }
            else if (result->type == BirdTypeType::PLACEHOLDER)
            {
                this->stack.push(std::make_shared<BoolType>());
            }
            else
            {
                this->user_error_tracker->type_error("expected struct in unary operation, found: " + bird_type_to_string(result), unary->op);
                this->stack.push(std::make_shared<ErrorType>());
            }
            break;
        }
        default:
        {
            throw BirdException("Undefined unary operator, " + unary->op.lexeme);
        }
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::FLOAT_LITERAL:
        {
            this->stack.push(std::make_shared<FloatType>());
            break;
        }
        case Token::Type::INT_LITERAL:
        {
            this->stack.push(std::make_shared<IntType>());
            break;
        }
        case Token::Type::BOOL_LITERAL:
        {
            this->stack.push(std::make_shared<BoolType>());
            break;
        }
        case Token::Type::STR_LITERAL:
        {
            this->stack.push(std::make_shared<StringType>());
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
        auto condition = this->stack.pop();

        ternary->true_expr->accept(this);
        auto true_expr = this->stack.pop();

        ternary->false_expr->accept(this);
        auto false_expr = this->stack.pop();

        if (*true_expr != *false_expr)
        {
            this->user_error_tracker->type_mismatch("in ternary operation", ternary->ternary_token);
            true_expr = std::make_shared<ErrorType>();
        }

        if (condition->type != BirdTypeType::BOOL)
        {
            this->user_error_tracker->type_error("expected bool in ternary condition", ternary->ternary_token);
            this->stack.push(std::make_shared<ErrorType>());
        }
        else
        {
            this->stack.push(true_expr);
        }
    }

    std::shared_ptr<BirdType> get_type_from_token(Token token)
    {
        auto type_name = token.lexeme;
        if (type_name == "int")
        {
            return std::make_shared<IntType>();
        }
        else if (type_name == "float")
        {
            return std::make_shared<FloatType>();
        }
        else if (type_name == "bool")
        {
            return std::make_shared<BoolType>();
        }
        else if (type_name == "str")
        {
            return std::make_shared<StringType>();
        }
        else if (type_name == "void")
        {
            return std::make_shared<VoidType>();
        }
        else
        {
            // type_name is not primitive
            if (this->type_table.contains(type_name))
            {
                return this->type_table.get(type_name);
            }

            if (this->struct_names.find(type_name) != this->struct_names.end())
            {
                return std::make_shared<PlaceholderType>(type_name);
            }

            this->user_error_tracker->type_error("unknown type", token);
            return std::make_shared<ErrorType>();
        }
    }
    void visit_func(Func *func)
    {
        std::vector<std::shared_ptr<BirdType>> params;

        std::transform(func->param_list.begin(), func->param_list.end(), std::back_inserter(params), [&](auto param)
                       { return this->get_type_from_token(param.second); });

        std::shared_ptr<BirdType> ret = func->return_type.has_value() ? this->get_type_from_token(func->return_type.value()) : std::shared_ptr<BirdType>(new VoidType());
        auto previous_return_type = this->return_type;
        this->return_type = ret;

        std::shared_ptr<BirdFunction> bird_function = std::make_shared<BirdFunction>(params, ret);
        this->call_table.declare(func->identifier.lexeme, bird_function);
        this->env.push_env();

        for (auto &param : func->param_list)
        {
            if (!is_bird_type(param.second))
            {
                this->env.declare(param.first.lexeme, this->get_type_from_token(param.second));
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
        auto condition = this->stack.pop();

        if (condition->type != BirdTypeType::BOOL)
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

        for (int i = 0; i < function->params.size(); i++)
        {
            call->args[i]->accept(this);
            auto arg = this->stack.pop();

            auto param = function->params[i];

            if (arg->type == BirdTypeType::PLACEHOLDER && param->type == BirdTypeType::STRUCT)
            {
                auto placeholder_type = safe_dynamic_pointer_cast<PlaceholderType>(arg);
                auto struct_type = safe_dynamic_pointer_cast<StructType>(param);

                if (struct_type->name == placeholder_type->name)
                {
                    continue;
                }
            }

            if (*arg != *param)
            {
                this->user_error_tracker->type_mismatch("in function call", call->identifier);
            }
        }

        this->stack.push(function->ret);
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            auto result = this->stack.pop();

            if (this->return_type.has_value())
            {
                if (*result != *this->return_type.value())
                {
                    this->user_error_tracker->type_mismatch("in return statement", return_stmt->return_token);
                }
            }
            else
            {
                this->user_error_tracker->type_error("unexpected return value in void function", return_stmt->return_token);
            }
        }
        else
        {
            if (!this->return_type.has_value() || this->return_type.value()->type != BirdTypeType::VOID)
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
        if (this->type_table.contains(type_stmt->identifier.lexeme))
        {
            this->user_error_tracker->type_error("type already declared", type_stmt->identifier);
            return;
        }

        if (type_stmt->type_is_literal)
        {
            auto alias = std::make_shared<AliasType>(type_stmt->identifier.lexeme, this->get_type_from_token(type_stmt->type_token));
            this->type_table.declare(type_stmt->identifier.lexeme, alias);
        }
        else
        {
            if (!this->type_table.contains(type_stmt->type_token.lexeme))
            {
                this->user_error_tracker->type_error("undefined type", type_stmt->type_token);
                return;
            }
            auto parent_type = this->type_table.get(type_stmt->type_token.lexeme);

            if (parent_type->type == BirdTypeType::STRUCT)
            {
                auto alias = std::make_shared<AliasType>(type_stmt->identifier.lexeme, parent_type);
                this->type_table.declare(type_stmt->identifier.lexeme, alias);
                return;
            }
            if (parent_type->type == BirdTypeType::ALIAS)
            {
                // alias types will only have one level of aliasing
                auto alias = safe_dynamic_pointer_cast<AliasType>(parent_type);
                auto new_alias = std::make_shared<AliasType>(type_stmt->identifier.lexeme, alias->alias);
                this->type_table.declare(type_stmt->identifier.lexeme, new_alias);
                return;
            }

            throw BirdException("invalid type parent");
        }
    }

    void visit_subscript(Subscript *subscript)
    {
        subscript->subscriptable->accept(this);
        auto subscriptable = this->stack.pop();

        subscript->index->accept(this);
        auto index = this->stack.pop();

        if (subscriptable->type != BirdTypeType::STRING)
        {
            this->user_error_tracker->type_error("expected string in subscriptable", subscript->subscript_token);
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        if (index->type != BirdTypeType::INT)
        {
            this->user_error_tracker->type_error("expected int in subscript index", subscript->subscript_token);
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        this->stack.push(std::make_shared<IntType>());
    }

    void visit_struct_decl(StructDecl *struct_decl)
    {
        std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> struct_fields;
        std::transform(struct_decl->fields.begin(), struct_decl->fields.end(), std::back_inserter(struct_fields), [&](std::pair<std::string, Token> field)
                       { return std::make_pair(field.first, this->get_type_from_token(field.second)); });

        auto struct_type = std::make_shared<StructType>(struct_decl->identifier.lexeme, struct_fields);
        this->type_table.declare(struct_decl->identifier.lexeme, struct_type);
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        direct_member_access->accessable->accept(this);
        auto accessable = this->stack.pop();

        if (accessable->type == BirdTypeType::ERROR)
        {
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        if (accessable->type == BirdTypeType::ALIAS)
        {
            auto alias = safe_dynamic_pointer_cast<AliasType>(accessable);
            accessable = alias->alias;
        }

        if (accessable->type == BirdTypeType::PLACEHOLDER)
        {
            auto placeholder = safe_dynamic_pointer_cast<PlaceholderType>(accessable);
            if (this->struct_names.find(placeholder->name) == this->struct_names.end())
            {
                this->user_error_tracker->type_error("struct not declared", Token());
                this->stack.push(std::make_shared<ErrorType>());
                return;
            }

            accessable = this->type_table.get(placeholder->name);
        }

        if (accessable->type != BirdTypeType::STRUCT)
        {
            this->user_error_tracker->type_error("expected struct in direct member access, found: " + bird_type_to_string(accessable), direct_member_access->identifier);
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        auto struct_type = safe_dynamic_pointer_cast<StructType>(accessable);

        for (auto &f : struct_type->fields)
        {
            if (f.first == direct_member_access->identifier.lexeme)
            {
                this->stack.push(f.second);
                return;
            }
        }

        return;
    }

    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        if (!this->type_table.contains(struct_initialization->identifier.lexeme))
        {
            this->user_error_tracker->type_error("struct not declared", struct_initialization->identifier);
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        auto original_type = this->type_table.get(struct_initialization->identifier.lexeme);
        auto type = original_type;

        if (type->type == BirdTypeType::ALIAS)
        {
            auto alias = safe_dynamic_pointer_cast<AliasType>(type);
            type = alias->alias;
        }

        auto struct_type = safe_dynamic_pointer_cast<StructType>(type);
        for (auto &field : struct_type->fields)
        {
            for (auto &field_assignment : struct_initialization->field_assignments)
            {
                field_assignment.second->accept(this);
                auto field_type = this->stack.pop();
                if (field.first == field_assignment.first)
                {
                    if (field_type->type == BirdTypeType::ERROR)
                    {
                        this->stack.push(std::make_shared<ErrorType>());
                        return;
                    }

                    if (field.second->type == BirdTypeType::PLACEHOLDER)
                    {
                        auto placeholder = safe_dynamic_pointer_cast<PlaceholderType>(field.second);
                        if (this->struct_names.find(placeholder->name) == this->struct_names.end())
                        {
                            this->user_error_tracker->type_error("struct not declared", Token());
                            this->stack.push(std::make_shared<ErrorType>());
                            return;
                        }

                        field.second = this->type_table.get(placeholder->name);
                    }

                    if (field_type->type == BirdTypeType::PLACEHOLDER)
                    {
                        auto placeholder = safe_dynamic_pointer_cast<PlaceholderType>(field_type);
                        if (this->struct_names.find(placeholder->name) == this->struct_names.end())
                        {
                            this->user_error_tracker->type_error("struct not declared", Token());
                            this->stack.push(std::make_shared<ErrorType>());
                            return;
                        }

                        field_type = this->type_table.get(placeholder->name);
                    }

                    if (*field.second != *field_type)
                    {
                        this->user_error_tracker->type_mismatch("in struct initialization", struct_initialization->identifier);
                        this->stack.push(std::make_shared<ErrorType>());
                        return;
                    }

                    break;
                }
            }
        }

        this->stack.push(original_type);
    }

    void visit_member_assign(MemberAssign *member_assign)
    {
        member_assign->accessable->accept(this);
        auto accessable = this->stack.pop();

        if (accessable->type == BirdTypeType::ERROR)
        {
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        if (accessable->type != BirdTypeType::STRUCT)
        {
            this->user_error_tracker->type_error("expected struct in member assign", member_assign->identifier);
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        auto struct_type = safe_dynamic_pointer_cast<StructType>(accessable);

        for (auto &f : struct_type->fields)
        {
            if (f.first == member_assign->identifier.lexeme)
            {
                member_assign->value->accept(this);
                auto value = this->stack.pop();

                if (*f.second != *value)
                {
                    this->user_error_tracker->type_mismatch("in member assign", member_assign->identifier);
                    this->stack.push(std::make_shared<ErrorType>());
                    return;
                }

                this->stack.push(f.second);
                return;
            }
        }
    }

    void visit_as_cast(AsCast *as_cast)
    {
        as_cast->expr->accept(this);
        auto expr = this->stack.pop();

        if (expr->type == BirdTypeType::ERROR)
        {
            this->stack.push(std::make_shared<ErrorType>());
            return;
        }

        auto to_type = this->get_type_from_token(as_cast->type);

        if (expr->type == BirdTypeType::ALIAS)
        {
            auto alias = safe_dynamic_pointer_cast<AliasType>(expr);
            expr = alias->alias;
        }

        auto original_type = to_type;
        if (original_type->type == BirdTypeType::ALIAS)
        {
            auto alias = safe_dynamic_pointer_cast<AliasType>(original_type);
            original_type = alias->alias;
        }

        if (original_type->type == expr->type)
        {
            this->stack.push(to_type);
            return;
        }

        if (original_type->type == BirdTypeType::FLOAT && expr->type == BirdTypeType::INT)
        {
            this->stack.push(to_type);
            return;
        }

        if (original_type->type == BirdTypeType::INT && expr->type == BirdTypeType::FLOAT)
        {
            this->stack.push(to_type);
            return;
        }

        this->user_error_tracker->type_mismatch("in 'as' type cast", as_cast->type);
        this->stack.push(std::make_shared<ErrorType>());
    }

    void visit_array_decl(ArrayDecl *array_decl)
    {
        throw BirdException("implement visit_array_decl in type checker");
    }
};
