#pragma once

#include <memory>
#include <vector>
#include <map>
#include "ast_node/index.h"
#include "bird_type.h"

/*
 * Visitor that prints the Abstract Syntax Tree
 * using prefix notation and expressions wrapped in parentheses
 */
class AstPrinter : public Visitor
{
public:
    void print_ast(std::vector<std::unique_ptr<Stmt>> *stmts)
    {

        for (auto &stmt : *stmts)
        {
            stmt->accept(this);
            std::cout << std::endl;
        }
    }

    void visit_block(Block *block)
    {
        std::cout << "{ ";
        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
            std::cout << "; ";
        }
        std::cout << "}";
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        std::cout << "var ";
        std::cout << decl_stmt->identifier.lexeme;
        if (decl_stmt->type.has_value())
        {
            std::cout << ": ";
            print_parse_type(decl_stmt->type.value());
        }
        std::cout << " = ";
        decl_stmt->value->accept(this);
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        std::cout << assign_expr->identifier.lexeme << " " << assign_expr->assign_operator.lexeme << " ";
        assign_expr->value->accept(this);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        std::cout << "print ";
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
        }
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
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
        std::cout << "(" << unary->op.lexeme << " ";
        unary->expr->accept(this);
        std::cout << ")";
    }

    void visit_primary(Primary *primary)
    {
        std::cout << primary->value.lexeme;
    }

    void visit_ternary(Ternary *ternary)
    {
        ternary->condition->accept(this);

        std::cout << " ? ";
        ternary->true_expr->accept(this);

        std::cout << " : ";
        ternary->false_expr->accept(this);
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        std::cout << "const ";
        std::cout << const_stmt->identifier.lexeme;
        if (const_stmt->type.has_value())
        {
            std::cout << ": ";
            print_parse_type(const_stmt->type.value());
        }
        std::cout << " = ";
        const_stmt->value->accept(this);
    }

    void visit_func(Func *func)
    {
        std::cout << "fn ";
        std::cout << func->identifier.lexeme;
        std::cout << "(";

        for (int i = 0; i < func->param_list.size(); ++i)
        {
            auto pair = func->param_list[i];
            std::cout << pair.first.lexeme << ": " << pair.second->get_token().lexeme;

            if (i < func->param_list.size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << ")";

        std::cout << " -> " << (func->return_type.has_value() ? func->return_type.value()->get_token().lexeme : "void") << " ";

        func->block->accept(this);
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        std::cout << "if ";
        if_stmt->condition->accept(this);

        std::cout << " ";
        if_stmt->then_branch->accept(this);

        if (if_stmt->else_branch.has_value())
        {
            std::cout << " else ";
            if_stmt->else_branch.value()->accept(this);
        }
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        std::cout << "while (";
        while_stmt->condition->accept(this);
        std::cout << ") ";
        while_stmt->stmt->accept(this);
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        std::cout << "for (";
        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
        }
        std::cout << "; ";
        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
        }
        std::cout << "; ";

        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
        }
        std::cout << ") ";
        for_stmt->body->accept(this);
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        std::cout << "return ";
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
        }
    }

    void visit_call(Call *call)
    {
        std::cout << call->identifier.lexeme;
        std::cout << "(";

        for (auto &arg : call->args)
        {
            arg->accept(this);
        }
        std::cout << ")";
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        std::cout << "break";
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        std::cout << "continue";
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        std::cout << "type " << type_stmt->identifier.lexeme << " = " << type_stmt->type_token->get_token().lexeme;
    }

    void visit_subscript(Subscript *subscript)
    {
        subscript->subscriptable->accept(this);
        std::cout << "[";
        subscript->index->accept(this);
        std::cout << "]";
    }

    void visit_struct_decl(StructDecl *struct_decl)
    {
        std::cout << "struct " << struct_decl->identifier.lexeme;
        std::cout << "{";
        for (auto it = struct_decl->fields.begin(); it != struct_decl->fields.end(); it++)
        {
            std::cout << it->first << ": " << it->second->get_token().lexeme << ", ";
        }
        std::cout << "}";
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        direct_member_access->accessable->accept(this);
        std::cout << "." << direct_member_access->identifier.lexeme;
    }

    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        std::cout << struct_initialization->identifier.lexeme << "{";
        for (auto it = struct_initialization->field_assignments.begin(); it != struct_initialization->field_assignments.end(); it++)
        {
            std::cout << it->first << ": ";
            it->second->accept(this);
            std::cout << ", ";
        }
        std::cout << "}";
    }

    void visit_member_assign(MemberAssign *member_assign)
    {
        member_assign->accessable->accept(this);
        std::cout << ".";
        this->visit_assign_expr(member_assign);
    }

    void print_parse_type(std::shared_ptr<ParseType::Type> type)
    {
        if (type->tag == ParseType::ARRAY)
        {
            auto array = safe_dynamic_pointer_cast<ParseType::Array, ParseType::Type>(type);
            this->print_parse_type(array->child);
            std::cout << "[]";

            return;
        }
        else if (type->tag == ParseType::USER_DEFINED)
        {
            auto user_defined = safe_dynamic_pointer_cast<ParseType::UserDefined, ParseType::Type>(type);

            std::cout << user_defined->type.lexeme;
            return;
        }
        else if (type->tag == ParseType::PRIMITIVE)
        {
            auto primitive = safe_dynamic_pointer_cast<ParseType::Primitive, ParseType::Type>(type);

            std::cout << primitive->type.lexeme;
            return;
        }

        throw BirdException("unknown parse type");
    }

    void visit_as_cast(AsCast *as_cast)
    {
        as_cast->expr->accept(this);
        std::cout << " as ";
        print_parse_type(as_cast->type);
    }

    void visit_array_init(ArrayInit *array_init)
    {
        std::cout << "[";
        for (int i = 0; i < array_init->elements.size(); i++)
        {
            array_init->elements[i]->accept(this);
            if (i < array_init->elements.size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "]";
    }

    void visit_index_assign(IndexAssign *index_assign)
    {
        index_assign->lhs->accept(this);
        std::cout << " " << index_assign->op.lexeme << " ";
        index_assign->rhs->accept(this);
    }
};
