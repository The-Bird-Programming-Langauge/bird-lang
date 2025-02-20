#pragma once
#include <set>
#include <vector>
#include <memory>
#include "exceptions/bird_exception.h"
#include "visitor_adapter.h"
#include "ast_node/index.h"

class HoistVisitor : public VisitorAdapter
{
public:
    std::set<std::string> *struct_names;
    HoistVisitor(std::set<std::string> *struct_names)
    {
        this->struct_names = struct_names;
    }

    void hoist(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        for (auto &stmt : *stmts)
        {
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
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        auto type_name = type_stmt->type_token->get_token().lexeme;

        if (this->struct_names->find(type_name) != this->struct_names->end())
        {
            return;
        }

        this->struct_names->insert(type_name);
    }

    void visit_struct_decl(StructDecl *struct_decl)
    {
        if (this->struct_names->find(struct_decl->identifier.lexeme) != this->struct_names->end())
        {
            return;
        }

        this->struct_names->insert(struct_decl->identifier.lexeme);
    }

    void visit_array_init(ArrayInit *array_init)
    {
    }
};