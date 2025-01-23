#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <set>

#include "ast_node/index.h"

#include "sym_table.h"
#include "exceptions/bird_exception.h"
#include "exceptions/return_exception.h"
#include "exceptions/break_exception.h"
#include "exceptions/continue_exception.h"
#include "exceptions/user_error_tracker.h"
#include "value.h"
#include "callable.h"

/*
 * Visitor that analyzes semantics of the AST
 */
class SemanticAnalyzer : public Visitor
{

public:
    Environment<SemanticValue> env;
    Environment<SemanticCallable> call_table;
    Environment<SemanticType> type_table;
    UserErrorTracker *user_error_tracker;
    int loop_depth;
    int function_depth;

    SemanticAnalyzer(UserErrorTracker *user_error_tracker) : user_error_tracker(user_error_tracker)
    {
        this->env.push_env();
        this->call_table.push_env();
        this->type_table.push_env();
        this->loop_depth = 0;
        this->function_depth = 0;
    }

    void analyze_semantics(std::vector<std::unique_ptr<Stmt>> *stmts)
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
        if (this->identifer_in_any_environment(decl_stmt->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + decl_stmt->identifier.lexeme + "' is already declared.", decl_stmt->identifier);
            return;
        }

        decl_stmt->value->accept(this);

        SemanticValue mutable_value;
        mutable_value.is_mutable = true;
        this->env.declare(decl_stmt->identifier.lexeme, mutable_value);
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        if (!this->env.contains(assign_expr->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + assign_expr->identifier.lexeme + "' is not initialized.", assign_expr->identifier);
            return;
        }

        auto previous_value = this->env.get(assign_expr->identifier.lexeme);

        if (!previous_value.is_mutable)
        {
            this->user_error_tracker->semantic_error("Identifier '" + assign_expr->identifier.lexeme + "' is not mutable.", assign_expr->identifier);
            return;
        }

        assign_expr->value->accept(this);
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
        if (this->identifer_in_any_environment(const_stmt->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + const_stmt->identifier.lexeme + "' is already declared.", const_stmt->identifier);
            return;
        }

        const_stmt->value->accept(this);

        this->env.declare(const_stmt->identifier.lexeme, SemanticValue());
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        this->loop_depth += 1;

        while_stmt->condition->accept(this);
        while_stmt->stmt->accept(this);

        this->loop_depth -= 1;
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        this->loop_depth += 1;
        this->env.push_env();

        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }

        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
        }

        for_stmt->body->accept(this);

        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
        }

        this->env.pop_env();

        this->loop_depth -= 1;
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);
    }

    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
    }

    void visit_primary(Primary *primary)
    {
        if (primary->value.token_type == Token::Type::IDENTIFIER && !this->env.contains(primary->value.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + primary->value.lexeme + "' is not initialized.", primary->value);
            return;
        }
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);
        ternary->true_expr->accept(this);
        ternary->false_expr->accept(this);
    }

    void visit_func(Func *func)
    {
        this->function_depth += 1;

        if (this->identifer_in_any_environment(func->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + func->identifier.lexeme + "' is already declared.", func->identifier);
            return;
        }

        this->call_table.declare(func->identifier.lexeme, SemanticCallable(func->param_list.size()));

        this->function_depth -= 1;
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        if_stmt->then_branch->accept(this);

        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch.value()->accept(this);
        }
    }

    void visit_call(Call *call)
    {
        if (!this->call_table.contains(call->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Function call identifier '" + call->identifier.lexeme + "' is not declared.", call->identifier);
            return;
        }

        auto function = this->call_table.get(call->identifier.lexeme);

        if (function.param_count != call->args.size())
        {
            this->user_error_tracker->semantic_error("Function call identifer '" + call->identifier.lexeme + "' does not use the correct number of arguments.", call->identifier);
            return;
        }
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (this->function_depth == 0)
        {
            this->user_error_tracker->semantic_error("Return statement is declared outside of a function.", return_stmt->return_token);
            return;
        }

        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
        }
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        if (this->loop_depth == 0)
        {
            this->user_error_tracker->semantic_error("Break statement is declared outside of a loop.", break_stmt->break_token);
            return;
        }
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        if (this->loop_depth == 0)
        {
            this->user_error_tracker->semantic_error("Continue statement is declared outside of a loop.", continue_stmt->continue_token);
            return;
        }
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        if (this->identifer_in_any_environment(type_stmt->identifier.lexeme))
        {
            this->user_error_tracker->semantic_error("Identifier '" + type_stmt->identifier.lexeme + "' is already declared.", type_stmt->identifier);
            return;
        }

        this->type_table.declare(type_stmt->identifier.lexeme, SemanticType());
    }

    bool identifer_in_any_environment(std::string identifer)
    {
        return this->env.current_contains(identifer) ||
               this->call_table.current_contains(identifer) ||
               this->type_table.current_contains(identifer);
    }

    void visit_subscript(Subscript *subscript)
    {
        subscript->subscriptable->accept(this);
        subscript->index->accept(this);
    };

    void visit_struct_decl(StructDecl *struct_decl)
    {
        // if (this->identifer_in_any_environment(struct_decl->identifier.lexeme))
        // {
        //     this->user_error_tracker->semantic_error("Identifier '" + struct_decl->identifier.lexeme + "' is already declared.", struct_decl->identifier);
        //     return;
        // }

        this->type_table.declare(struct_decl->identifier.lexeme, SemanticType());

        // do nothing
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        direct_member_access->accessable->accept(this);
    }

    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        // for (auto &field_assignment : struct_initialization->field_assignments)
        // {
        //     field_assignment.second->accept(this);
        // }
    }

    void visit_member_assign(MemberAssign *member_assign)
    {
        member_assign->accessable->accept(this);
    }
};