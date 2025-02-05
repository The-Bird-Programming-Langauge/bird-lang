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

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
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
};