#pragma once
#include "binaryen-c.h"
#include <fstream>
#include <ios>

enum CodeGenType
{
    CodeGenInt,
    CodeGenFloat,
    CodeGenBool,
    CodeGenVoid,
    CodeGenPtr
};

struct TaggedExpression
{
    BinaryenExpressionRef expression;
    std::optional<CodeGenType> type;

    TaggedExpression() : expression(nullptr), type(std::nullopt) {}
    TaggedExpression(BinaryenExpressionRef expr, std::optional<CodeGenType> t = std::nullopt)
        : expression(expr), type(t) {}
};

struct TaggedIndex
{
    BinaryenIndex index;
    CodeGenType type;

    TaggedIndex() : index(0), type(CodeGenVoid) {}
    TaggedIndex(BinaryenIndex i, CodeGenType t) : index(i), type(t) {}
};

struct TaggedType
{
    BinaryenType binaryen_type;
    std::optional<CodeGenType> type;

    TaggedType() : binaryen_type(BinaryenTypeInt32()), type(std::nullopt) {}
    TaggedType(BinaryenType binaryen_typeP, std::optional<CodeGenType> typeP = std::nullopt)
        : binaryen_type(binaryen_typeP), type(typeP) {}
};

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
    Environment<TaggedIndex> environment; // tracks the index of local variables
    Environment<Type> type_table;

    Stack<TaggedExpression> stack; // for returning values

    std::map<std::string, std::string> std_lib;

    // we need the function return types when calling functions
    std::unordered_map<std::string, TaggedType> function_return_types;
    // allows us to track the local variables of a function
    std::unordered_map<std::string, std::vector<BinaryenType>> function_locals;
    std::string current_function_name; // for indexing into maps

    std::vector<MemorySegment> memory_segments; // store memory segments to add at once

    uint32_t current_offset = 1024; // current offset is set high to allow
                                    // js to identify a string by a higher
                                    // offset, will fix tomorrow
    BinaryenModuleRef mod;

public:
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
        std::vector<int8_t> passive;
        std::vector<BinaryenExpressionRef> offsets;

        // add all memory segment information to
        // vectors to set at once
        for (const auto &segment : memory_segments)
        {
            segments.push_back(segment.data);
            sizes.push_back(segment.size);
            passive.push_back(0);
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
            passive.data(),
            offsets.data(),
            sizes.data(),
            segments.size(),
            0);
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
        std::vector<int8_t> passive;
        std::vector<BinaryenExpressionRef> offsets;

        // add all memory segment information to
        // vectors to set at once
        for (const auto &segment : memory_segments)
        {
            segments.push_back(segment.data);
            sizes.push_back(segment.size);
            passive.push_back(0);
            offsets.push_back(segment.offset);
        }

        // call to create memory with all segments
        BinaryenSetMemory(
            mod,
            1, // initial pages
            1, // maximum pages
            "memory",
            segments.data(),
            passive.data(),
            offsets.data(),
            sizes.data(),
            segments.size(),
            0);
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
                main_function_body.push_back(result.expression);
            }

            if (auto const_stmt = dynamic_cast<ConstStmt *>(stmt.get()))
            {
                const_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto ternary_stmt = dynamic_cast<Ternary *>(stmt.get()))
            {
                ternary_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result.expression);
            }

            if (auto type_stmt = dynamic_cast<TypeStmt *>(stmt.get()))
            {
                type_stmt->accept(this);
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

        BinaryenFunctionRef mainFunction =
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

        this->environment.pop_env();
    }

    bool is_bird_type(Token token)
    {
        return token.lexeme == "bool" || token.lexeme == "int" || token.lexeme == "float" || token.lexeme == "void" || token.lexeme == "str";
    }

    BinaryenType from_bird_type(Token token)
    {
        if (token.lexeme == "bool")
            return BinaryenTypeInt32(); // bool is represented as i32
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
                return from_bird_type(this->type_table.get(token.lexeme).type);
            }

            throw BirdException("invalid type");
        }
    }

    CodeGenType to_code_gen_type(Token token)
    {
        if (token.lexeme == "bool")
            return CodeGenBool; // bool is represented as i32
        else if (token.lexeme == "int")
            return CodeGenInt;
        else if (token.lexeme == "float")
            return CodeGenFloat;
        else if (token.lexeme == "void")
            return CodeGenVoid;
        else if (token.lexeme == "str")
            return CodeGenPtr;
        else
        {
            if (this->type_table.contains(token.lexeme))
            {
                return to_code_gen_type(this->type_table.get(token.lexeme).type);
            }

            throw BirdException("invalid type");
        }
    }

    BinaryenType from_codegen_type(CodeGenType token)
    {
        if (token == CodeGenBool)
            return BinaryenTypeInt32();
        else if (token == CodeGenInt)
            return BinaryenTypeInt32();
        else if (token == CodeGenFloat)
            return BinaryenTypeFloat64();
        else if (token == CodeGenVoid)
            return BinaryenTypeNone();
        else if (token == CodeGenPtr)
            return BinaryenTypeInt32();
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

            if (result.expression)
            {
                children.push_back(result.expression);
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

        CodeGenType type;
        if (decl_stmt->type_token.has_value())
        {
            if (!is_bird_type(decl_stmt->type_token.value()))
            {
                type = to_code_gen_type(this->type_table.get(decl_stmt->type_token.value().lexeme).type);
            }
            else
            {
                type = to_code_gen_type(decl_stmt->type_token.value());
            }

            if (type == BinaryenTypeInt32() && BinaryenExpressionGetType(initializer_value.expression) == BinaryenTypeFloat64())
            {
                initializer_value =
                    BinaryenUnary(
                        mod,
                        BinaryenTruncSatSFloat64ToInt32(),
                        initializer_value.expression);
            }
            else if (type == BinaryenTypeFloat64() && BinaryenExpressionGetType(initializer_value.expression) == BinaryenTypeInt32())
            {
                initializer_value =
                    BinaryenUnary(
                        mod,
                        BinaryenConvertSInt32ToFloat64(),
                        initializer_value.expression);
            }
        }
        else
        {
            if (initializer_value.type.has_value())
            {
                type = initializer_value.type.value();
            }
            else
            {
                BinaryenType binaryen_type = BinaryenExpressionGetType(initializer_value.expression);
                type = (binaryen_type == BinaryenTypeFloat64())
                           ? CodeGenFloat
                           : CodeGenInt;
            }
        }

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(from_codegen_type(type));

        environment.declare(decl_stmt->identifier.lexeme, TaggedIndex(index, type));

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value.expression);

        this->stack.push(TaggedExpression(set_local, type));
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        TaggedIndex index = this->environment.get(assign_expr->identifier.lexeme);

        auto lhs_val = BinaryenLocalGet(
            this->mod,
            index.index,
            from_codegen_type(index.type));

        assign_expr->value->accept(this);
        TaggedExpression rhs_val = this->stack.pop();

        bool float_flag = (index.type == CodeGenFloat || (rhs_val.type.has_value() && rhs_val.type.value() == CodeGenFloat));
        if (float_flag && index.type == CodeGenInt)
        {
            rhs_val =
                BinaryenUnary(
                    mod,
                    BinaryenTruncSatSFloat64ToInt32(),
                    rhs_val.expression);
        }
        else if (float_flag && rhs_val.type == CodeGenInt)
        {
            rhs_val =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    rhs_val.expression);
        }

        BinaryenExpressionRef result;
        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            result = rhs_val.expression;
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            // TODO: figure out string conatenation
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenAddFloat64(), lhs_val, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenAddInt32(), lhs_val, rhs_val.expression);

            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenSubFloat64(), lhs_val, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenSubInt32(), lhs_val, rhs_val.expression);

            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenMulFloat64(), lhs_val, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenMulInt32(), lhs_val, rhs_val.expression);

            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenDivFloat64(), lhs_val, rhs_val.expression)
                         : BinaryenBinary(this->mod, BinaryenDivSInt32(), lhs_val, rhs_val.expression);

            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            result = (float_flag)
                         ? throw BirdException("Modular operation requires integer values")
                         : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val, rhs_val.expression);

            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
            break;
        }

        BinaryenExpressionRef assign_stmt = BinaryenLocalSet(
            this->mod,
            index.index,
            result);

        this->stack.push(TaggedExpression(assign_stmt));
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = this->stack.pop();

            CodeGenType codegen_type;
            if (result.type.has_value())
            {
                codegen_type = result.type.value();
            }
            else
            {
                // if type isnt give defaults to int
                BinaryenType binaryen_type = BinaryenExpressionGetType(result.expression);
                if (binaryen_type == BinaryenTypeInt32())
                    codegen_type = CodeGenInt;
                else if (binaryen_type == BinaryenTypeFloat64())
                    codegen_type = CodeGenFloat;
                else
                    throw BirdException("unsupported print type");
            }

            if (codegen_type == CodeGenInt)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_i32",
                        &result.expression,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (codegen_type == CodeGenFloat)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_f64",
                        &result.expression,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (codegen_type == CodeGenPtr)
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_str",
                        &result.expression,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else
            {
                throw BirdException("Unsupported print datatype");
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

        children.push_back(body.expression);

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
            condition.expression,
            nullptr));

        auto while_body =
            BinaryenBlock(
                this->mod,
                "while_block",
                while_body_children.data(),
                while_body_children.size(),
                BinaryenTypeNone());

        auto loop =
            BinaryenLoop(
                this->mod,
                "LOOP",
                while_body);

        this->stack.push(
            BinaryenBlock(
                this->mod,
                "EXIT",
                &loop,
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

        children.push_back(body.expression);

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
        body_and_increment_children.push_back(increment.expression);
        body_and_increment_children.push_back(
            BinaryenBreak(
                this->mod,
                "LOOP",
                condition.expression,
                nullptr));

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
        if (initializer.expression)
        {
            initializer_and_loop.push_back(initializer.expression);
        }

        initializer_and_loop.push_back(for_loop);

        this->stack.push(
            BinaryenBlock(
                this->mod,
                "EXIT",
                initializer_and_loop.data(),
                initializer_and_loop.size(),
                BinaryenTypeNone()));

        this->environment.pop_env();
    }

    void visit_binary(Binary *binary)
    {
        binary->left->accept(this);
        binary->right->accept(this);

        auto right = this->stack.pop();
        auto left = this->stack.pop();

        bool float_flag = (BinaryenExpressionGetType(left.expression) == BinaryenTypeFloat64() ||
                           BinaryenExpressionGetType(right.expression) == BinaryenTypeFloat64());

        if (float_flag && BinaryenExpressionGetType(left.expression) == BinaryenTypeInt32())
        {
            left =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    left.expression);
        }
        else if (float_flag && BinaryenExpressionGetType(right.expression) == BinaryenTypeInt32())
        {
            right =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    right.expression);
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
                              left.expression,
                              right.expression),
                          CodeGenFloat))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenAddInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenFloat))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenSubInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenFloat))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenDivSInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenFloat))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenMulInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenInt))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenGtSInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenInt))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenGeSInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

            break;
        }
        case Token::Type::LESS:
        {
            (float_flag)
                ? this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod, BinaryenLtFloat64(),
                              left.expression,
                              right.expression),
                          CodeGenInt))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenLtSInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenInt))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenLeSInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenInt))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenEqInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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
                              left.expression,
                              right.expression),
                          CodeGenInt))
                : this->stack.push(
                      TaggedExpression(
                          BinaryenBinary(
                              this->mod,
                              BinaryenNeInt32(),
                              left.expression,
                              right.expression),
                          CodeGenInt));

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

        BinaryenType expr_type = BinaryenExpressionGetType(expr.expression);

        if (expr_type == BinaryenTypeFloat64())
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenUnary(
                        mod,
                        BinaryenNegFloat64(),
                        expr.expression),
                    CodeGenFloat));
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
                        expr.expression),
                    CodeGenInt));
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
            this->stack.push(TaggedExpression(int_literal, CodeGenInt));
            break;
        }

        case Token::Type::FLOAT_LITERAL:
        {
            double value = std::stod(primary->value.lexeme);
            BinaryenExpressionRef float_literal = BinaryenConst(this->mod, BinaryenLiteralFloat64(value));
            this->stack.push(TaggedExpression(float_literal, CodeGenFloat));
            break;
        }

        case Token::Type::BOOL_LITERAL:
        {
            BinaryenExpressionRef bool_literal = BinaryenConst(
                this->mod,
                primary->value.lexeme == "true"
                    ? BinaryenLiteralInt32(1)
                    : BinaryenLiteralInt32(0));

            this->stack.push(TaggedExpression(bool_literal, CodeGenBool));
            break;
        }

        case Token::Type::STR_LITERAL:
        {
            const std::string &str_value = primary->value.lexeme;
            uint32_t str_ptr;

            add_memory_segment(mod, str_value, str_ptr);

            BinaryenExpressionRef str_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(str_ptr));

            this->stack.push(TaggedExpression(str_literal, CodeGenPtr));
        }

        case Token::Type::IDENTIFIER:
        {
            TaggedIndex tagged_index = this->environment.get(primary->value.lexeme);
            BinaryenExpressionRef local_get = BinaryenLocalGet(
                this->mod,
                tagged_index.index,
                from_codegen_type(tagged_index.type));

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
                condition.expression,
                true_expr.expression,
                false_expr.expression,
                BinaryenExpressionGetType(true_expr.expression)));
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
        TaggedExpression initializer = this->stack.pop();

        CodeGenType type;
        if (const_stmt->type_token.has_value())
        {
            if (!is_bird_type(const_stmt->type_token.value()))
            {
                type = to_code_gen_type(this->type_table.get(const_stmt->type_token.value().lexeme).type);
            }
            else
            {
                type = to_code_gen_type(const_stmt->type_token.value());
            }
        }
        else
        {
            BinaryenType binaryen_type = BinaryenExpressionGetType(initializer.expression);
            type = (binaryen_type == BinaryenTypeFloat64())
                       ? CodeGenFloat
                       : CodeGenInt;
        }

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(from_codegen_type(type));

        environment.declare(const_stmt->identifier.lexeme, TaggedIndex(index, type));

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer.expression);

        this->stack.push(TaggedExpression(set_local, type));
    }

    void visit_func(Func *func)
    {
        auto func_name = func->identifier.lexeme;

        if (func->return_type.has_value())
        {
            auto binaryen_return_type = from_bird_type(func->return_type.value());
            auto codegen_return_type = to_code_gen_type(func->return_type.value());
            this->function_return_types[func_name] = TaggedType(binaryen_return_type, codegen_return_type);
        }
        else
        {
            this->function_return_types[func_name] = TaggedType(BinaryenTypeNone(), CodeGenVoid);
        }

        auto old_function_name = this->current_function_name;

        this->current_function_name = func_name;
        auto current_function_body = std::vector<BinaryenExpressionRef>();
        this->function_locals[func_name] = std::vector<BinaryenType>();

        std::vector<BinaryenType> param_types;

        for (auto &param : func->param_list)
        {
            param_types.push_back(from_bird_type(param.second));
            this->function_locals[func_name].push_back(from_bird_type(param.second));
        }

        BinaryenType params = BinaryenTypeCreate(param_types.data(), param_types.size());

        BinaryenType result_type = func->return_type.has_value()
                                       ? from_bird_type(func->return_type.value())
                                       : BinaryenTypeNone();

        this->environment.push_env();

        auto index = 0;
        for (auto &param : func->param_list)
        {
            this->environment.declare(param.first.lexeme, TaggedIndex(index++, to_code_gen_type(param.second)));
        }

        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();

            if (result_type == BinaryenTypeNone())
            {
                result = BinaryenDrop(this->mod, result.expression);
            }

            current_function_body.push_back(result.expression);
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

        BinaryenFunctionRef func_type = BinaryenAddFunction(
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
                    condition.expression,
                    then_branch.expression,
                    else_branch.expression));
        }
        else
        {
            this->stack.push(
                BinaryenIf(
                    this->mod,
                    condition.expression,
                    then_branch.expression,
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
            args.push_back(this->stack.pop().expression);
        }

        auto return_type = this->function_return_types[func_name];
        this->stack.push(TaggedExpression(
            BinaryenCall(
                this->mod,
                func_name.c_str(),
                args.data(),
                args.size(),
                return_type.binaryen_type),
            return_type.type));
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        TaggedType func_return_type = this->function_return_types[this->current_function_name];
        BinaryenExpressionRef return_expr;

        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            auto result = this->stack.pop();

            // now allows for things like fn addF(x: float, y: float) -> int {...}
            if (result.type.has_value() && result.type != func_return_type.type)
            {
                if (func_return_type.type == CodeGenFloat && result.type == CodeGenInt)
                {
                    result = TaggedExpression(
                        BinaryenUnary(
                            this->mod,
                            BinaryenConvertSInt32ToFloat64(),
                            result.expression),
                        CodeGenFloat);
                }
                else if (func_return_type.type == CodeGenInt && result.type == CodeGenFloat)
                {
                    result = TaggedExpression(
                        BinaryenUnary(
                            this->mod,
                            BinaryenTruncSatSFloat64ToInt32(),
                            result.expression),
                        CodeGenInt);
                }
                else
                {
                    throw BirdException("Type mismatch in return statement");
                }
            }

            this->stack.push(
                TaggedExpression(
                    BinaryenReturn(
                        this->mod,
                        result.expression),
                    func_return_type.type));
        }
        else
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenReturn(
                        this->mod,
                        nullptr),
                    CodeGenVoid));
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
        if (type_stmt->type_is_literal)
        {
            this->type_table.declare(type_stmt->identifier.lexeme, Type(type_stmt->type_token));
        }
        else
        {
            this->type_table.declare(type_stmt->identifier.lexeme, Type(this->type_table.get(type_stmt->type_token.lexeme).type));
        }
    }
};
