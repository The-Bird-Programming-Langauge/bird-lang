#pragma once

#include <memory>
#include <vector>
#include "stmt.h"
#include "expr.h"

#include "binary.h"
#include "unary.h"
#include "primary.h"

#include "decl_stmt.h"
#include "print_stmt.h"
#include "expr_stmt.h"

class AstPrinter : public Visitor
{
public:
    void print_ast(std::vector<std::unique_ptr<Stmt>> *stmts)
    {

        for (auto &stmt : *stmts)
        {
            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
            }
            std::cout << std::endl;
        }
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        std::cout << "let ";
        std::cout << decl_stmt->identifier.lexeme << " = ";
        decl_stmt->value->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        std::cout << "puts ";
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
        std::cout << ";";
    }

    void visit_binary(Binary *binary)
    {
        std::cout << "(";
        std::cout << binary->op.lexeme << " ";
        binary->left->accept(this);
        std::cout << " ";
        binary->right->accept(this);
        std::cout << ")";
    }

    void visit_unary(Unary *unary)
    {
        std::cout << unary->op.lexeme;
        unary->expr->accept(this);
    }

    void visit_primary(Primary *primary)
    {
        std::cout << primary->value.lexeme;
    }
};
