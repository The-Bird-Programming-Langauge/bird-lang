
#pragma once

#include <memory>
#include <vector>
#include <map>
#include "stmt.h"
#include "expr.h"

#include "binary.h"
#include "unary.h"
#include "primary.h"

#include "decl_stmt.h"
#include "expr_stmt.h"

#include "bird_exception.h"
#include "sym_table.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> Builder(TheContext);
static llvm::Module *TheModule = new llvm::Module("my_module", TheContext);

class CodeGen : public Visitor
{
    std::vector<llvm::Value *> stack;
    SymbolTable<llvm::Value *> environment;

public:
    void generate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        // Declare the printf function (external function)
        llvm::FunctionType *printfType = llvm::FunctionType::get(Builder.getInt32Ty(), Builder.getPtrTy(), true);
        llvm::FunctionCallee printfFunc = TheModule->getOrInsertFunction("printf", printfType);

        // main function
        llvm::FunctionType *funcType = llvm::FunctionType::get(Builder.getVoidTy(), false);
        llvm::Function *mainFunction = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", TheModule);
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(TheContext, "entry", mainFunction);

        Builder.SetInsertPoint(entry);

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
        }

        auto result = this->stack[this->stack.size() - 1];

        llvm::Value *formatStr = Builder.CreateGlobalStringPtr("%d\n");
        Builder.CreateCall(printfFunc, {formatStr, result});

        Builder.CreateRetVoid();

        TheModule->print(llvm::outs(), nullptr);
        this->stack.clear();

        std::error_code EC;
        llvm::raw_fd_ostream dest("output.ll", EC);

        if (EC)
        {
            llvm::errs() << "Could not open file: " << EC.message();
            exit(1);
        }

        // Print the IR to the file
        TheModule->print(dest, nullptr);

        dest.close(); // Close the output file
    }

    void visitDeclStmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);

        llvm::Value *result = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        this->environment.insert(decl_stmt->identifier.lexeme, result);
    }

    void visitExprStmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
    }

    void visitBinary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto left = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto right = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        switch (binary->op.token_type)
        {
        case TokenType::PLUS:
        {
            auto value = Builder.CreateAdd(left, right, "addtmp");
            this->stack.push_back(value);
            break;
        }
        case TokenType::MINUS:
        {
            auto value = Builder.CreateSub(left, right, "subtmp");
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined binary operator for code gen");
        }
        }
    }

    void visitUnary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack[this->stack.size() - 1];
        this->stack.pop_back();

        auto llvm_value = Builder.CreateNeg(expr);
        this->stack.push_back(
            llvm_value);
    }

    void visitPrimary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case TokenType::I32_LITERAL:
        {
            float value = std::stoi(primary->value.lexeme);

            auto llvm_value = llvm::ConstantInt::get(TheContext, llvm::APInt(32, value));
            this->stack.push_back(llvm_value);
            break;
        }
        case TokenType::IDENTIFIER:
        {
            auto value = this->environment.get(primary->value.lexeme);
            if (value == nullptr)
            {
                throw BirdException("undefined identifier");
            }
            this->stack.push_back(value);
            break;
        }
        default:
        {
            throw BirdException("undefined primary value");
        }
        }
    }
};