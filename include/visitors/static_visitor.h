#pragma once
#include <set>
#include <vector>
#include <memory>
#include "exceptions/bird_exception.h"
#include "visitor_adapter.h"
#include "ast_node/index.h"

class StaticVisitor : public VisitorAdapter
{
public:
    std::vector<std::string> *static_strings;

    StaticVisitor(std::vector<std::string> *static_strings)
    {
        this->static_strings = static_strings;
    }
    ~StaticVisitor() {}

    void static_pass(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
                continue;
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
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

            if (auto func = dynamic_cast<Func *>(stmt.get()))
            {
                func->accept(this);
                continue;
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                continue;
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
                continue;
            }

            if (auto ternary_expr = dynamic_cast<Ternary *>(stmt.get()))
            {
                ternary_expr->accept(this);
                continue;
            }

            if (auto member_assign = dynamic_cast<MemberAssign *>(stmt.get()))
            {
                member_assign->accept(this);
                continue;
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
                continue;
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                continue;
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                continue;
            }

            if (auto as_cast = dynamic_cast<AsCast *>(stmt.get()))
            {
                as_cast->accept(this);
                continue;
            }
        }
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            assign_expr->value->accept(this);
            break;
        }
        default:
            break;
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::STR_LITERAL:
        {
            this->static_strings->push_back(primary->value.lexeme);
            break;
        }
        default:
            break;
        }
    }
    
    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }
    }

    void visit_struct_initialization(StructInitialization *struct_intialization)
    {
        for (auto &field_assignment : struct_intialization->field_assignments)
        {
            field_assignment.second->accept(this);
        }
    }

    void visit_block(Block *block)
    {
        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
        }
    }

    void visit_func(Func *func)
    {
        func->block->accept(this);
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->then_branch->accept(this);
        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch->get()->accept(this);
        }
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr->get()->accept(this);
        }
    }

    void visit_ternary(Ternary *ternary_expr)
    {
        ternary_expr->condition->accept(this);
        ternary_expr->true_expr->accept(this);
        ternary_expr->false_expr->accept(this);
    }

    void visit_member_assign(MemberAssign *member_assign)
    {
        member_assign->value->accept(this);
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        for_stmt->body->accept(this);
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        while_stmt->stmt->accept(this);
    }

    void visit_as_cast(AsCast *as_cast)
    {
        as_cast->expr->accept(this);
    }
};