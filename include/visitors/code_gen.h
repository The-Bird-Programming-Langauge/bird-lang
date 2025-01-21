#pragma once
#include "binaryen-c.h"
#include "bird_type.h"
#include <fstream>
#include <ios>

static std::string bird_type_to_string(std::shared_ptr<BirdType> type)
{
    switch (type->type)
    {
    case BirdTypeType::INT:
        return "int";
    case BirdTypeType::FLOAT:
        return "float";
    case BirdTypeType::BOOL:
        return "bool";
    case BirdTypeType::VOID:
        return "void";
    case BirdTypeType::STRING:
        return "string";
    case BirdTypeType::STRUCT:
        return "struct";
    default:
        return "unknown";
    }
}

template <typename T>
struct Tagged
{
    T value;
    std::shared_ptr<BirdType> type = std::shared_ptr<BirdType>(new VoidType());

    Tagged() : value(T()) {}
    Tagged(T v) : value(v) {}
    Tagged(T v, std::shared_ptr<BirdType> t) : value(v), type(t) {}
};

using TaggedExpression = Tagged<BinaryenExpressionRef>;
using TaggedIndex = Tagged<BinaryenIndex>;
using TaggedType = Tagged<BinaryenType>;

/*
 * used to avoid multiple BinaryenMemorySet calls
 * which somehow retain the information but overwrite
 * the page bounds
 */
struct MemorySegment
{
    const char *data;
    BinaryenIndex size;
    BinaryenExpressionRef offset;
};

class CodeGen : public Visitor
{
public:
    Environment<TaggedIndex> environment; // tracks the index of local variables
    Environment<std::shared_ptr<BirdType>> type_table;
    Stack<TaggedExpression> stack; // for returning values
    std::map<std::string, std::string> std_lib;

    // we need the function return types when calling functions
    std::unordered_map<std::string, TaggedType> function_return_types;
    // allows us to track the local variables of a function
    std::unordered_map<std::string, std::vector<BinaryenType>> function_locals;
    std::string current_function_name;          // for indexing into maps
    std::vector<MemorySegment> memory_segments; // store memory segments to add at once

    uint32_t current_offset = 1024; // current offset is set high to allow
                                    // js to identify a string by a higher
                                    // offset, will fix tomorrow
    BinaryenModuleRef mod;

    ~CodeGen()
    {
        BinaryenModuleDispose(this->mod);
    }

    CodeGen() : mod(BinaryenModuleCreate())
    {
        this->environment.push_env();
        this->type_table.push_env();
    }

    void init_std_lib()
    {
        BinaryenAddFunctionImport(
            this->mod,
            "print_i32",
            "env",
            "print_i32",
            BinaryenTypeInt32(),
            BinaryenTypeNone());

        BinaryenAddFunctionImport(
            this->mod,
            "print_f64",
            "env",
            "print_f64",
            BinaryenTypeFloat64(),
            BinaryenTypeNone());

        BinaryenAddFunctionImport(
            this->mod,
            "print_str",
            "env",
            "print_str",
            BinaryenTypeInt32(),
            BinaryenTypeNone());

        BinaryenType args[2] = {BinaryenTypeInt32(), BinaryenTypeInt32()}; // pointer and index
        BinaryenType args_type = BinaryenTypeCreate(args, 2);
        BinaryenAddFunctionImport(
            this->mod,
            "mem_get",
            "env",
            "mem_get",
            args_type,
            BinaryenTypeInt32());
    }

    void add_memory_segment(BinaryenModuleRef mod, const std::string &str, uint32_t &str_offset)
    {
        str_offset = current_offset;
        this->current_offset += str.size() + 1;

        MemorySegment segment = {
            str.c_str(),
            static_cast<BinaryenIndex>(str.size() + 1), // + 1 for '\0'
            BinaryenConst(mod, BinaryenLiteralInt32(str_offset))};

        this->memory_segments.push_back(segment);
    }

    void init_static_memory(BinaryenModuleRef mod)
    {
        std::vector<const char *> segments;
        std::vector<BinaryenIndex> sizes;
        bool *passive = new bool[memory_segments.size()];
        std::vector<BinaryenExpressionRef> offsets;

        // add all memory segment information to
        // vectors to set at once
        for (const auto &segment : memory_segments)
        {
            segments.push_back(segment.data);
            sizes.push_back(segment.size);
            passive[segments.size() - 1] = false;
            offsets.push_back(segment.offset);
        }
        // since static memory is added at once we can
        // calculate the exact memory in pages to allocate
        BinaryenIndex max_pages = (current_offset / 65536) + 1;

        // call to create memory with all segments
        BinaryenSetMemory(
            mod,
            1,         // initial pages
            max_pages, // maximum pages
            "memory",
            segments.data(),
            passive,
            offsets.data(),
            sizes.data(),
            segments.size(),
            0);

        delete[] passive;
    }

    void generate(std::vector<std::unique_ptr<Stmt>> *stmts)
    {
        this->init_std_lib();

        this->current_function_name = "main";
        auto main_function_body = std::vector<BinaryenExpressionRef>();
        this->function_locals[this->current_function_name] = std::vector<BinaryenType>();

        for (auto &stmt : *stmts)
        {
            if (auto func_stmt = dynamic_cast<Func *>(stmt.get()))
            {
                func_stmt->accept(this);
                // no stack push here, automatically added
            }

            if (auto decl_stmt = dynamic_cast<DeclStmt *>(stmt.get()))
            {
                decl_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto ternary_stmt = dynamic_cast<Ternary *>(stmt.get()))
            {
                ternary_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.value);
            }

            if (auto type_stmt = dynamic_cast<TypeStmt *>(stmt.get()))
            {
                type_stmt->accept(this);
                // no stack push here, only type table
            }

            if (auto struct_decl = dynamic_cast<StructDecl *>(stmt.get()))
            {
                struct_decl->accept(this);
                // no stack push here, only type table
            }
        }

        this->init_static_memory(this->mod);

        BinaryenType params = BinaryenTypeNone();
        BinaryenType results = BinaryenTypeNone();

        BinaryenExpressionRef body =
            BinaryenBlock(
                this->mod,
                nullptr,
                main_function_body.data(),
                main_function_body.size(),
                BinaryenTypeNone());

        BinaryenAddFunction(
            this->mod,
            "main",
            params,
            results,
            this->function_locals["main"].data(),
            this->function_locals["main"].size(),
            body);

        BinaryenAddFunctionExport(
            this->mod,
            "main",
            "main");

        BinaryenModulePrint(this->mod);

        BinaryenModuleAllocateAndWriteResult result =
            BinaryenModuleAllocateAndWrite(this->mod, nullptr);

        if (!result.binary || result.binaryBytes == 0)
        {
            std::cerr << "failed to serialize" << std::endl;
            return;
        }

        std::string filename = "output.wasm";
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open())
        {
            file.write(static_cast<char *>(result.binary), result.binaryBytes);
            file.close();
            std::cout << "wasm module written to " << filename << std::endl;
        }
        else
        {
            std::cerr << "failed to open file" << filename << std::endl;
        }

        free(result.binary);

        // this->environment.pop_env();
    }

    bool is_bird_type(Token token)
    {
        return token.lexeme == "bool" || token.lexeme == "int" || token.lexeme == "float" || token.lexeme == "void" || token.lexeme == "str";
    }

    BinaryenType token_to_binaryen_type(Token token)
    {
        if (token.lexeme == "bool")
            return BinaryenTypeInt32();
        else if (token.lexeme == "int")
            return BinaryenTypeInt32();
        else if (token.lexeme == "float")
            return BinaryenTypeFloat64();
        else if (token.lexeme == "void")
            return BinaryenTypeNone();
        else if (token.lexeme == "str")
            return BinaryenTypeInt32();
        else
        {
            if (this->type_table.contains(token.lexeme))
            {
                return bird_type_to_binaryen_type(this->type_table.get(token.lexeme));
            }

            std::cout << "token to binaryen type" << std::endl;
            throw BirdException("invalid type");
        }
    }

    BinaryenType bird_type_to_binaryen_type(std::shared_ptr<BirdType> bird_type)
    {
        if (bird_type->type == BirdTypeType::BOOL)
            return BinaryenTypeInt32();
        else if (bird_type->type == BirdTypeType::INT)
            return BinaryenTypeInt32();
        else if (bird_type->type == BirdTypeType::FLOAT)
            return BinaryenTypeFloat64();
        else if (bird_type->type == BirdTypeType::VOID)
            return BinaryenTypeNone();
        else if (bird_type->type == BirdTypeType::STRING)
            return BinaryenTypeInt32();
        else if (bird_type->type == BirdTypeType::STRUCT)
            return BinaryenTypeInt32(); // ptr
        else if (bird_type->type == BirdTypeType::ALIAS)
        {
            std::shared_ptr<AliasType> alias = std::dynamic_pointer_cast<AliasType>(bird_type);
            return bird_type_to_binaryen_type(alias->alias);
        }
        std::cout << "bird type to binaryen type" << std::endl;
        throw BirdException("invalid type");
    }

    BinaryenType from_bird_type(std::shared_ptr<BirdType> token)
    {
        if (token->type == BirdTypeType::BOOL)
            return BinaryenTypeInt32();
        else if (token->type == BirdTypeType::INT)
            return BinaryenTypeInt32();
        else if (token->type == BirdTypeType::FLOAT)
            return BinaryenTypeFloat64();
        else if (token->type == BirdTypeType::VOID)
            return BinaryenTypeNone();
        else if (token->type == BirdTypeType::STRING || token->type == BirdTypeType::STRUCT)
            return BinaryenTypeInt32();
        else if (token->type == BirdTypeType::ALIAS)
        {
            std::shared_ptr<AliasType> alias = std::dynamic_pointer_cast<AliasType>(token);
            return from_bird_type(alias->alias);
        }
        else
        {
            throw BirdException("invaid type");
        }
    }

    void visit_block(Block *block)
    {
        std::vector<BinaryenExpressionRef> children;
        this->environment.push_env();

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();

            if (result.value)
            {
                children.push_back(result.value);
            }
        }

        this->environment.pop_env();

        BinaryenExpressionRef block_expr =
            BinaryenBlock(
                this->mod,
                nullptr,
                children.data(),
                children.size(),
                BinaryenTypeNone());

        this->stack.push(block_expr);
    }

    void visit_decl_stmt(DeclStmt *decl_stmt)
    {
        decl_stmt->value->accept(this);
        TaggedExpression initializer_value = this->stack.pop();

        std::shared_ptr<BirdType> type;
        if (decl_stmt->type_token.has_value()) // not inferred
        {
            if (decl_stmt->type_is_literal)
            {
                type = token_to_bird_type(decl_stmt->type_token.value());
            }
            else
            {
                type = this->type_table.get(decl_stmt->type_token.value().lexeme);
            }

            if (type->type == BirdTypeType::INT && initializer_value.type->type == BirdTypeType::FLOAT)
            {
                initializer_value =
                    TaggedExpression(
                        BinaryenUnary(
                            mod,
                            BinaryenTruncSatSFloat64ToInt32(),
                            initializer_value.value),
                        std::shared_ptr<BirdType>(new IntType()));
            }
            else if (type->type == BirdTypeType::FLOAT && initializer_value.type->type == BirdTypeType::INT)
            {
                initializer_value =
                    TaggedExpression(
                        BinaryenUnary(
                            mod,
                            BinaryenConvertSInt32ToFloat64(),
                            initializer_value.value),
                        std::shared_ptr<BirdType>(new FloatType()));
            }
        }
        else
        {
            if (initializer_value.type->type != BirdTypeType::VOID)
            {
                type = initializer_value.type;
            }
            else
            {
                BinaryenType binaryen_type = BinaryenExpressionGetType(initializer_value.value);
                type = (binaryen_type == BinaryenTypeFloat64())
                           ? std::shared_ptr<BirdType>(new FloatType())
                           : std::shared_ptr<BirdType>(new IntType());
            }
        }

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(from_bird_type(type));

        environment.declare(decl_stmt->identifier.lexeme, TaggedIndex(index, type));

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value.value);

        this->stack.push(TaggedExpression(set_local, type));
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        TaggedIndex index = this->environment.get(assign_expr->identifier.lexeme);

        auto lhs_val = BinaryenLocalGet(
            this->mod,
            index.value,
            from_bird_type(index.type));

        assign_expr->value->accept(this);
        TaggedExpression rhs_val = this->stack.pop();

        bool float_flag = (index.type->type == BirdTypeType::FLOAT || rhs_val.type->type == BirdTypeType::FLOAT);
        if (float_flag && index.type->type == BirdTypeType::INT)
        {
            rhs_val =
                BinaryenUnary(
                    mod,
                    BinaryenTruncSatSFloat64ToInt32(),
                    rhs_val.value);
        }
        else if (float_flag && rhs_val.type->type == BirdTypeType::INT)
        {
            rhs_val =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    rhs_val.value);
        }

        BinaryenExpressionRef result;
        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            result = rhs_val.value;
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            // TODO: figure out string conatenation
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenAddFloat64(), lhs_val, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenAddInt32(), lhs_val, rhs_val.value);

            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenSubFloat64(), lhs_val, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenSubInt32(), lhs_val, rhs_val.value);

            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenMulFloat64(), lhs_val, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenMulInt32(), lhs_val, rhs_val.value);

            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenDivFloat64(), lhs_val, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenDivSInt32(), lhs_val, rhs_val.value);

            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            result = (float_flag)
                         ? throw BirdException("Modular operation requires integer values")
                         : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val, rhs_val.value);

            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
            break;
        }

        BinaryenExpressionRef assign_stmt = BinaryenLocalSet(
            this->mod,
            index.value,
            result);

        this->stack.push(TaggedExpression(assign_stmt));
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = this->stack.pop();

            if (result.type->type == BirdTypeType::ALIAS)
            {
                std::shared_ptr<AliasType> alias = std::dynamic_pointer_cast<AliasType>(result.type);
                result.type = alias->alias;
            }

            if (result.type->type == BirdTypeType::VOID)
            {
                throw BirdException("unsupported print type");
            }

            if (result.type->type == BirdTypeType::INT)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_i32",
                        &result.value,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (result.type->type == BirdTypeType::BOOL)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_i32",
                        &result.value,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (result.type->type == BirdTypeType::FLOAT)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_f64",
                        &result.value,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (result.type->type == BirdTypeType::STRING)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_str",
                        &result.value,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (result.type->type == BirdTypeType::STRUCT)
            {
                throw BirdException("unsupported print type");
            }
            else
            {
                throw BirdException("Unsupported print datatype: " + bird_type_to_string(result.type));
            }
        }
    }

    void visit_expr_stmt(ExprStmt *expr_stmt)
    {
        expr_stmt->expr->accept(this);
        // pop and push it back to the stack
    }

    void visit_while_stmt(WhileStmt *while_stmt)
    {
        std::vector<BinaryenExpressionRef> children;

        while_stmt->stmt->accept(this);
        TaggedExpression body = this->stack.pop();

        children.push_back(body.value);

        while_stmt->condition->accept(this);
        TaggedExpression condition = this->stack.pop();

        auto outer_body =
            BinaryenBlock(
                this->mod,
                "BODY",
                children.data(),
                children.size(),
                BinaryenTypeNone());

        std::vector<BinaryenExpressionRef> while_body_children;
        while_body_children.push_back(outer_body);
        while_body_children.push_back(BinaryenBreak(
            this->mod,
            "LOOP",
            condition.value,
            nullptr));

        auto while_body =
            BinaryenBlock(
                this->mod,
                "while_block",
                while_body_children.data(),
                while_body_children.size(),
                BinaryenTypeNone());

        auto if_cond =
            BinaryenIf(
                this->mod,
                condition.value,
                BinaryenLoop(
                    this->mod,
                    "LOOP",
                    while_body),
                nullptr);

        this->stack.push(
            BinaryenBlock(
                this->mod,
                "EXIT",
                &if_cond,
                1,
                BinaryenTypeNone()));
    }

    void visit_for_stmt(ForStmt *for_stmt)
    {
        this->environment.push_env();
        std::vector<BinaryenExpressionRef> children;

        TaggedExpression initializer;
        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
            initializer = this->stack.pop();
        }

        TaggedExpression condition;
        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
            condition = this->stack.pop();
        }

        for_stmt->body->accept(this);
        TaggedExpression body = this->stack.pop();

        children.push_back(body.value);

        TaggedExpression increment;
        if (for_stmt->increment.has_value())
        {
            for_stmt->increment.value()->accept(this);
            increment = this->stack.pop();
        }

        auto body_and_condition = BinaryenBlock(
            this->mod,
            "BODY",
            children.data(),
            children.size(),
            BinaryenTypeNone());

        std::vector<BinaryenExpressionRef> body_and_increment_children;
        body_and_increment_children.push_back(body_and_condition);

        if (increment.value)
        {
            body_and_increment_children.push_back(increment.value);
        }

        if (condition.value)
        {
            body_and_increment_children.push_back(
                BinaryenBreak(
                    this->mod,
                    "LOOP",
                    condition.value,
                    nullptr));
        }
        else
        {
            body_and_increment_children.push_back(
                BinaryenBreak(
                    this->mod,
                    "LOOP",
                    nullptr,
                    nullptr));
        }

        auto body_and_increment = BinaryenBlock(
            this->mod,
            "for_body",
            body_and_increment_children.data(),
            body_and_increment_children.size(),
            BinaryenTypeNone());

        auto for_loop = BinaryenLoop(
            this->mod,
            "LOOP",
            body_and_increment);

        std::vector<BinaryenExpressionRef> initializer_and_loop;
        if (initializer.value)
        {
            initializer_and_loop.push_back(initializer.value);
        }

        initializer_and_loop.push_back(for_loop);

        auto block = BinaryenBlock(
            this->mod,
            "EXIT",
            initializer_and_loop.data(),
            initializer_and_loop.size(),
            BinaryenTypeNone());

        this->stack.push(
            condition.value ? BinaryenIf(
                                  this->mod,
                                  condition.value,
                                  block,
                                  nullptr)
                            : block);

        this->environment.pop_env();
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack.pop();
        auto left = this->stack.pop();

        bool float_flag = (BinaryenExpressionGetType(left.value) == BinaryenTypeFloat64() ||
                           BinaryenExpressionGetType(right.value) == BinaryenTypeFloat64());

        if (float_flag && BinaryenExpressionGetType(left.value) == BinaryenTypeInt32())
        {
            left =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    left.value);
        }
        else if (float_flag && BinaryenExpressionGetType(right.value) == BinaryenTypeInt32())
        {
            right =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    right.value);
        }

        // TODO: print bool for relational operators, currently pushes them as CodeGenInt
        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenAddFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new FloatType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenAddInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new IntType())));

            break;
        }
        case Token::Type::MINUS:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenSubFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new FloatType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenSubInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new IntType())));

            break;
        }
        case Token::Type::SLASH:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenDivFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new FloatType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenDivSInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new IntType())));

            break;
        }
        case Token::Type::STAR:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenMulFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new FloatType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenMulInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new IntType())));

            break;
        }
        case Token::Type::GREATER:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenGtFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenGtSInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())));

            break;
        }
        case Token::Type::PERCENT:
        {
            (float_flag)
                ? throw BirdException("modular operation requires integer values")
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenRemSInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new IntType())));

            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenGeFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenGeSInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())));

            break;
        }
        case Token::Type::LESS:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod, BinaryenLtFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenLtSInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())));

            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenLeFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenLeSInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())));

            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenEqFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenEqInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())));

            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenNeFloat64(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenNeInt32(),
                              left.value,
                              right.value),
                          std::shared_ptr<BirdType>(new BoolType())));

            break;
        }
        default:
        {
            throw BirdException("undefined binary operator for code gen");
        }
        }
    }

    /*
     * Binaryen doesnt support integer negation for some reason,
     * only float32 and 64, so i guess the work around would be
     * to subtract it from zero.
     */
    void visit_unary(Unary *unary)
    {
        unary->expr->accept(this);
        auto expr = this->stack.pop();

        BinaryenType expr_type = BinaryenExpressionGetType(expr.value);

        if (expr_type == BinaryenTypeFloat64())
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenUnary(
                        mod,
                        BinaryenNegFloat64(),
                        expr.value),
                    std::shared_ptr<BirdType>(new BoolType())));
        }
        else if (expr_type == BinaryenTypeInt32())
        {
            BinaryenExpressionRef zero = BinaryenConst(mod, BinaryenLiteralInt32(0));

            this->stack.push(
                TaggedExpression(
                    BinaryenBinary(
                        mod,
                        BinaryenSubInt32(),
                        zero,
                        expr.value),
                    std::shared_ptr<BirdType>(new IntType())));
        }
    }

    void visit_primary(Primary *primary)
    {
        switch (primary->value.token_type)
        {
        case Token::Type::INT_LITERAL:
        {
            int value = std::stoi(primary->value.lexeme);
            BinaryenExpressionRef int_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(value));
            this->stack.push(TaggedExpression(int_literal, std::shared_ptr<BirdType>(new IntType())));
            break;
        }

        case Token::Type::FLOAT_LITERAL:
        {
            double value = std::stod(primary->value.lexeme);
            BinaryenExpressionRef float_literal = BinaryenConst(this->mod, BinaryenLiteralFloat64(value));
            this->stack.push(TaggedExpression(float_literal, std::shared_ptr<BirdType>(new FloatType())));
            break;
        }

        case Token::Type::BOOL_LITERAL:
        {
            BinaryenExpressionRef bool_literal = BinaryenConst(
                this->mod,
                primary->value.lexeme == "true"
                    ? BinaryenLiteralInt32(1)
                    : BinaryenLiteralInt32(0));

            this->stack.push(TaggedExpression(bool_literal, std::shared_ptr<BirdType>(new BoolType())));
            break;
        }

        case Token::Type::STR_LITERAL:
        {
            const std::string &str_value = primary->value.lexeme;
            uint32_t str_ptr;

            add_memory_segment(mod, str_value, str_ptr);

            BinaryenExpressionRef str_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(str_ptr));

            this->stack.push(TaggedExpression(str_literal, std::shared_ptr<BirdType>(new StringType())));
            break;
        }

        case Token::Type::IDENTIFIER:
        {
            TaggedIndex tagged_index = this->environment.get(primary->value.lexeme);
            BinaryenExpressionRef local_get = BinaryenLocalGet(
                this->mod,
                tagged_index.value,
                from_bird_type(tagged_index.type));

            this->stack.push(TaggedExpression(local_get, tagged_index.type));
            break;
        }

        default:
            throw BirdException("undefined primary value: " + primary->value.lexeme);
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

        this->stack.push(
            BinaryenSelect(
                this->mod,
                condition.value,
                true_expr.value,
                false_expr.value,
                BinaryenExpressionGetType(true_expr.value)));
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
        TaggedExpression initializer = this->stack.pop();

        std::shared_ptr<BirdType> type;
        if (const_stmt->type_token.has_value())
        {
            if (!is_bird_type(const_stmt->type_token.value()))
            {
                type = this->type_table.get(const_stmt->type_token.value().lexeme);
            }
            else
            {
                type = token_to_bird_type(const_stmt->type_token.value());
            }

            if (type->type == BirdTypeType::INT && initializer.type->type == BirdTypeType::FLOAT)
            {
                initializer =
                    TaggedExpression(
                        BinaryenUnary(
                            mod,
                            BinaryenTruncSatSFloat64ToInt32(),
                            initializer.value),
                        std::shared_ptr<BirdType>(new IntType()));
            }
            else if (type->type == BirdTypeType::FLOAT && initializer.type->type == BirdTypeType::INT)
            {
                initializer =
                    TaggedExpression(
                        BinaryenUnary(
                            mod,
                            BinaryenConvertSInt32ToFloat64(),
                            initializer.value),
                        std::shared_ptr<BirdType>(new FloatType()));
            }
        }
        else
        {
            if (initializer.type->type != BirdTypeType::VOID)
            {
                type = initializer.type;
            }
            else
            {
                BinaryenType binaryen_type = BinaryenExpressionGetType(initializer.value);
                type = (binaryen_type == BinaryenTypeFloat64())
                           ? std::shared_ptr<BirdType>(new FloatType())
                           : std::shared_ptr<BirdType>(new IntType());
            }
        }

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(from_bird_type(type));

        environment.declare(const_stmt->identifier.lexeme, TaggedIndex(index, type));

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer.value);

        this->stack.push(TaggedExpression(set_local, type));
    }

    void visit_func(Func *func)
    {
        auto func_name = func->identifier.lexeme;

        if (func->return_type.has_value())
        {
            auto binaryen_return_type = token_to_binaryen_type(func->return_type.value());
            // auto codegen_return_type = token_to_bird_type(func->return_type.value());
            if (this->is_bird_type(func->return_type.value()))
            {
                this->function_return_types[func_name] = TaggedType(binaryen_return_type, token_to_bird_type(func->return_type.value()));
            }
            else
            {
                if (this->type_table.contains(func->return_type.value().lexeme))
                    this->function_return_types[func_name] = TaggedType(binaryen_return_type, this->type_table.get(func->return_type.value().lexeme));
                else
                    throw BirdException("invalid return type");
            }
        }
        else
        {
            this->function_return_types[func_name] = TaggedType(BinaryenTypeNone(), std::shared_ptr<BirdType>(new VoidType()));
        }

        auto old_function_name = this->current_function_name;

        this->current_function_name = func_name;
        auto current_function_body = std::vector<BinaryenExpressionRef>();
        this->function_locals[func_name] = std::vector<BinaryenType>();

        std::vector<BinaryenType> param_types;

        for (auto &param : func->param_list)
        {
            param_types.push_back(token_to_binaryen_type(param.second));
            this->function_locals[func_name].push_back(token_to_binaryen_type(param.second));
        }

        BinaryenType params = BinaryenTypeCreate(param_types.data(), param_types.size());

        BinaryenType result_type = func->return_type.has_value()
                                       ? token_to_binaryen_type(func->return_type.value())
                                       : BinaryenTypeNone();

        this->environment.push_env();

        auto index = 0;
        for (auto &param : func->param_list)
        {
            if (this->is_bird_type(param.second))
            {
                this->environment.declare(param.first.lexeme, TaggedIndex(index++, token_to_bird_type(param.second)));
            }
            else
            {
                if (this->type_table.contains(param.second.lexeme))
                    this->environment.declare(param.first.lexeme, TaggedIndex(index++, this->type_table.get(param.second.lexeme)));
                else
                    throw BirdException("invalid param type");
            }
        }

        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();

            if (result_type == BinaryenTypeNone())
            {
                result = BinaryenDrop(this->mod, result.value);
            }

            current_function_body.push_back(result.value);
        }

        this->environment.pop_env();

        BinaryenExpressionRef body = BinaryenBlock(
            this->mod,
            nullptr,
            current_function_body.data(),
            current_function_body.size(),
            BinaryenTypeNone());

        std::vector<BinaryenType> vars = std::vector<BinaryenType>(
            this->function_locals[func_name].begin() + param_types.size(),
            this->function_locals[func_name].end());

        BinaryenAddFunction(
            this->mod,
            func_name.c_str(),
            params,
            result_type,
            vars.data(),
            vars.size(),
            body);

        BinaryenAddFunctionExport(
            this->mod,
            func_name.c_str(),
            func_name.c_str());

        this->current_function_name = old_function_name;
        this->function_locals.erase(func_name);

        // no stack push here, automatically added
    }

    void visit_if_stmt(IfStmt *if_stmt)
    {
        if_stmt->condition->accept(this);
        auto condition = this->stack.pop();

        if_stmt->then_branch->accept(this);
        auto then_branch = this->stack.pop();

        if (if_stmt->else_branch.has_value())
        {
            if_stmt->else_branch.value()->accept(this);
            auto else_branch = this->stack.pop();

            this->stack.push(
                BinaryenIf(
                    this->mod,
                    condition.value,
                    then_branch.value,
                    else_branch.value));
        }
        else
        {
            this->stack.push(
                BinaryenIf(
                    this->mod,
                    condition.value,
                    then_branch.value,
                    nullptr));
        }
    }

    void visit_call(Call *call)
    {
        auto func_name = call->identifier.lexeme;

        std::vector<BinaryenExpressionRef> args;

        for (auto &arg : call->args)
        {
            arg->accept(this);
            args.push_back(this->stack.pop().value);
        }

        auto return_type = this->function_return_types[func_name];
        this->stack.push(TaggedExpression(
            BinaryenCall(
                this->mod,
                func_name.c_str(),
                args.data(),
                args.size(),
                return_type.value),
            return_type.type));
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        TaggedType func_return_type = this->function_return_types[this->current_function_name];

        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            auto result = this->stack.pop();

            // now allows for things like fn addF(x: float, y: float) -> int {...}
            if (result.type != func_return_type.type)
            {
                if (func_return_type.type->type == BirdTypeType::FLOAT && result.type->type == BirdTypeType::INT)
                {
                    result = TaggedExpression(
                        BinaryenUnary(
                            this->mod,
                            BinaryenConvertSInt32ToFloat64(),
                            result.value),
                        std::shared_ptr<BirdType>(new FloatType()));
                }
                else if (func_return_type.type->type == BirdTypeType::INT && result.type->type == BirdTypeType::FLOAT)
                {
                    result = TaggedExpression(
                        BinaryenUnary(
                            this->mod,
                            BinaryenTruncSatSFloat64ToInt32(),
                            result.value),
                        std::shared_ptr<BirdType>(new IntType()));
                }
            }

            this->stack.push(
                TaggedExpression(
                    BinaryenReturn(
                        this->mod,
                        result.value),
                    func_return_type.type));
        }
        else
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenReturn(
                        this->mod,
                        nullptr),
                    std::shared_ptr<VoidType>()));
        }
    }

    void visit_break_stmt(BreakStmt *break_stmt)
    {
        this->stack.push(
            BinaryenBreak(
                this->mod,
                "EXIT",
                nullptr,
                nullptr));
    }

    void visit_continue_stmt(ContinueStmt *continue_stmt)
    {
        this->stack.push(
            BinaryenBreak(
                this->mod,
                "BODY",
                nullptr,
                nullptr));
    }

    void visit_type_stmt(TypeStmt *type_stmt)
    {
        std::shared_ptr<AliasType> alias;
        if (type_stmt->type_is_literal)
        {
            alias = std::make_shared<AliasType>(
                type_stmt->identifier.lexeme,
                token_to_bird_type(type_stmt->type_token));
        }
        else
        {
            alias = std::make_shared<AliasType>(
                type_stmt->identifier.lexeme,
                this->type_table.get(type_stmt->type_token.lexeme));
        }

        this->type_table.declare(type_stmt->identifier.lexeme, alias);
    }

    void visit_subscript(Subscript *subscript)
    {
        subscript->subscriptable->accept(this);
        auto subscriptable = this->stack.pop();

        subscript->index->accept(this);
        auto index = this->stack.pop();

        BinaryenExpressionRef args[2] = {subscriptable.value, index.value};

        this->stack.push(
            TaggedExpression(
                BinaryenCall(
                    this->mod,
                    "mem_get",
                    args,
                    2,
                    BinaryenTypeInt32()),
                std::shared_ptr<BirdType>(new IntType())));
    }

    /*
    This function is called when a struct declaration is encountered.
    The struct's shape is stored in the type table.
    */
    void visit_struct_decl(StructDecl *struct_decl)
    {
        std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> struct_fields;
        std::transform(struct_decl->fields.begin(), struct_decl->fields.end(), std::back_inserter(struct_fields), [&](std::pair<std::string, Token> field)
                       { 
                        if (is_bird_type(field.second))
                        {
                            return std::make_pair(field.first, token_to_bird_type(field.second));
                        }

                        if (this->type_table.contains(field.second.lexeme))
                        {
                            return std::make_pair(field.first, this->type_table.get(field.second.lexeme));
                        } 

                        std::cout << "visit struct decl" << std::endl;
                        throw BirdException("invalid type"); });

        type_table.declare(struct_decl->identifier.lexeme, std::make_shared<StructType>(struct_decl->identifier.lexeme, struct_fields));
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        // assume that the struct is already declared
        // assume that the member is already declared

        // direct_member_access->accessable->accept(this);
        // auto instance = this->stack.pop();

        // BinaryenExpressionRef args[2] = {instance.value, index.value};
        // this->stack.push(
        //     TaggedExpression(
        //         BinaryenCall(
        //             this->mod,
        //             "mem_get",
        //             args,
        //             2,
        //             BinaryenTypeInt32()),
        //         CodeGenInt));
    }

    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        throw BirdException("Struct initialization not implemented");
    }
};
