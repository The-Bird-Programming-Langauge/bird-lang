#pragma once
#include "binaryen-c.h"

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
    Environment<BinaryenIndex> environment; // tracks the index of local variables
    Environment<Type> type_table;

    Stack<BinaryenExpressionRef> stack; // for returning values

    std::map<std::string, std::string> std_lib;

    // we need the function return types when calling functions
    std::unordered_map<std::string, BinaryenType> function_return_types;
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
                main_function_body.push_back(result);
            }

            if (auto print_stmt = dynamic_cast<PrintStmt *>(stmt.get()))
            {
                print_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto if_stmt = dynamic_cast<IfStmt *>(stmt.get()))
            {
                if_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto block = dynamic_cast<Block *>(stmt.get()))
            {
                block->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt.get()))
            {
                expr_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto ternary_stmt = dynamic_cast<Ternary *>(stmt.get()))
            {
                ternary_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto while_stmt = dynamic_cast<WhileStmt *>(stmt.get()))
            {
                while_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto for_stmt = dynamic_cast<ForStmt *>(stmt.get()))
            {
                for_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get()))
            {
                return_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get()))
            {
                break_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
            }

            if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get()))
            {
                continue_stmt->accept(this);
                auto result = this->stack.pop();
                main_function_body.push_back(result);
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

    void visit_block(Block *block)
    {
        std::vector<BinaryenExpressionRef> children;
        this->environment.push_env();

        for (auto &stmt : block->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();

            if (result)
            {
                children.push_back(result);
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
        BinaryenExpressionRef initializer_value = this->stack.pop();

        BinaryenType type;
        if (decl_stmt->type_token.has_value())
        {
            if (!is_bird_type(decl_stmt->type_token.value()))
            {
                type = from_bird_type(this->type_table.get(decl_stmt->type_token.value().lexeme).type);
            }
            else
            {
                type = from_bird_type(decl_stmt->type_token.value());
            }
        }
        else
        {
            type = BinaryenExpressionGetType(initializer_value);
        }

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(type);

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value);
        environment.declare(decl_stmt->identifier.lexeme, index);

        this->stack.push(set_local);
    }

    void visit_assign_expr(AssignExpr *assign_expr)
    {
        BinaryenIndex index = this->environment.get(assign_expr->identifier.lexeme);

        auto lhs_val = BinaryenLocalGet(
            this->mod,
            index,
            this->function_locals[this->current_function_name][index]);

        assign_expr->value->accept(this);
        auto rhs_val = this->stack.pop();

        bool float_flag = (BinaryenExpressionGetType(lhs_val) == BinaryenTypeFloat64() ||
                           BinaryenExpressionGetType(rhs_val) == BinaryenTypeFloat64());

        if (float_flag && BinaryenExpressionGetType(lhs_val) == BinaryenTypeInt32())
        {
            rhs_val =
                BinaryenUnary(
                    mod,
                    BinaryenTruncSatSFloat64ToInt32(),
                    rhs_val);
        }
        else if (float_flag && BinaryenExpressionGetType(rhs_val) == BinaryenTypeInt32())
        {
            rhs_val =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    rhs_val);
        }

        BinaryenExpressionRef result;
        switch (assign_expr->assign_operator.token_type)
        {
        case Token::Type::EQUAL:
        {
            result = rhs_val;
            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            // TODO: figure out string conatenation
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenAddFloat64(), lhs_val, rhs_val)
                         : BinaryenBinary(this->mod, BinaryenAddInt32(), lhs_val, rhs_val);

            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenSubFloat64(), lhs_val, rhs_val)
                         : BinaryenBinary(this->mod, BinaryenSubInt32(), lhs_val, rhs_val);

            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenMulFloat64(), lhs_val, rhs_val)
                         : BinaryenBinary(this->mod, BinaryenMulInt32(), lhs_val, rhs_val);

            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenDivFloat64(), lhs_val, rhs_val)
                         : BinaryenBinary(this->mod, BinaryenDivSInt32(), lhs_val, rhs_val);

            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            result = (float_flag)
                         ? throw BirdException("Modular operation requires integer values")
                         : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val, rhs_val);

            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + assign_expr->assign_operator.lexeme);
            break;
        }

        BinaryenExpressionRef assign_stmt = BinaryenLocalSet(
            this->mod,
            index,
            result);

        this->stack.push(assign_stmt);
    }

    void visit_print_stmt(PrintStmt *print_stmt)
    {
        for (auto &arg : print_stmt->args)
        {
            arg->accept(this);
            auto result = this->stack.pop();

            // TODO: print strings
            if (BinaryenExpressionGetType(result) == BinaryenTypeInt32())
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_i32",
                        &result,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else if (BinaryenExpressionGetType(result) == BinaryenTypeFloat64())
            {
                BinaryenExpressionRef consoleLogCall =
                    BinaryenCall(
                        this->mod,
                        "print_f64",
                        &result,
                        1,
                        BinaryenTypeNone());

                this->stack.push(consoleLogCall);
            }
            else
            {
                throw BirdException("usupported print datatype");
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
        BinaryenExpressionRef body = this->stack.pop();

        children.push_back(body);

        while_stmt->condition->accept(this);
        BinaryenExpressionRef condition = this->stack.pop();

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
            condition,
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

        BinaryenExpressionRef initializer;
        if (for_stmt->initializer.has_value())
        {
            for_stmt->initializer.value()->accept(this);
            initializer = this->stack.pop();
        }

        BinaryenExpressionRef condition;
        if (for_stmt->condition.has_value())
        {
            for_stmt->condition.value()->accept(this);
            condition = this->stack.pop();
        }

        for_stmt->body->accept(this);
        BinaryenExpressionRef body = this->stack.pop();

        children.push_back(body);

        BinaryenExpressionRef increment;
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
        body_and_increment_children.push_back(increment);
        body_and_increment_children.push_back(
            BinaryenBreak(
                this->mod,
                "LOOP",
                condition,
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
        if (initializer)
        {
            initializer_and_loop.push_back(initializer);
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

        bool float_flag = (BinaryenExpressionGetType(left) == BinaryenTypeFloat64() ||
                           BinaryenExpressionGetType(right) == BinaryenTypeFloat64());

        if (float_flag && BinaryenExpressionGetType(left) == BinaryenTypeInt32())
        {
            left =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    left);
        }
        else if (float_flag && BinaryenExpressionGetType(right) == BinaryenTypeInt32())
        {
            right =
                BinaryenUnary(
                    mod,
                    BinaryenConvertSInt32ToFloat64(),
                    right);
        }

        switch (binary->op.token_type)
        {
        case Token::Type::PLUS:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenAddFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenAddInt32(), left, right));

            break;
        }
        case Token::Type::MINUS:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenSubFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenSubInt32(), left, right));

            break;
        }
        case Token::Type::SLASH:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenDivFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenDivSInt32(), left, right));

            break;
        }
        case Token::Type::STAR:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenMulFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenMulInt32(), left, right));

            break;
        }
        case Token::Type::GREATER:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenGtFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenGtSInt32(), left, right));

            break;
        }
        case Token::Type::PERCENT:
        {
            (float_flag)
                ? throw BirdException("Modular operation requires integer values")
                : this->stack.push(BinaryenBinary(mod, BinaryenRemSInt32(), left, right));

            break;
        }
        case Token::Type::GREATER_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenGeFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenGeSInt32(), left, right));

            break;
        }
        case Token::Type::LESS:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenLtFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenLtSInt32(), left, right));

            break;
        }
        case Token::Type::LESS_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenLeFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenLeSInt32(), left, right));

            break;
        }
        case Token::Type::EQUAL_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenEqFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenEqInt32(), left, right));

            break;
        }
        case Token::Type::BANG_EQUAL:
        {
            (float_flag)
                ? this->stack.push(BinaryenBinary(mod, BinaryenNeFloat64(), left, right))
                : this->stack.push(BinaryenBinary(mod, BinaryenNeInt32(), left, right));

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

        BinaryenType expr_type = BinaryenExpressionGetType(expr);

        if (expr_type == BinaryenTypeFloat64())
        {
            this->stack.push(BinaryenUnary(mod, BinaryenNegFloat64(), expr));
        }
        else if (expr_type == BinaryenTypeInt32())
        {
            BinaryenExpressionRef zero = BinaryenConst(mod, BinaryenLiteralInt32(0));

            this->stack.push(
                BinaryenBinary(
                    mod,
                    BinaryenSubInt32(),
                    zero,
                    expr));
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
            this->stack.push(int_literal);
            break;
        }

        case Token::Type::FLOAT_LITERAL:
        {
            double value = std::stod(primary->value.lexeme);
            BinaryenExpressionRef float_literal = BinaryenConst(this->mod, BinaryenLiteralFloat64(value));
            this->stack.push(float_literal);
            break;
        }

        case Token::Type::BOOL_LITERAL:
        {
            BinaryenExpressionRef bool_literal = BinaryenConst(
                this->mod,
                primary->value.lexeme == "true"
                    ? BinaryenLiteralInt32(1)
                    : BinaryenLiteralInt32(0));

            this->stack.push(bool_literal);
            break;
        }

        case Token::Type::STR_LITERAL:
        {
            const std::string &str_value = primary->value.lexeme;
            uint32_t str_ptr;

            add_memory_segment(mod, str_value, str_ptr);

            this->stack.push(BinaryenConst(this->mod, BinaryenLiteralInt32(str_ptr)));
            break;
        }

        case Token::Type::IDENTIFIER:
        {
            BinaryenIndex index = this->environment.get(primary->value.lexeme);

            this->stack.push(
                BinaryenLocalGet(
                    this->mod,
                    index,
                    this->function_locals[this->current_function_name][index]));
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
                condition,
                true_expr,
                false_expr,
                BinaryenExpressionGetType(true_expr)));
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
        BinaryenExpressionRef initializer_value = this->stack.pop();

        BinaryenType type;
        if (const_stmt->type_token.has_value())
        {
            if (!is_bird_type(const_stmt->type_token.value()))
            {
                type = from_bird_type(this->type_table.get(const_stmt->type_token.value().lexeme).type);
            }
            else
            {
                type = from_bird_type(const_stmt->type_token.value());
            }
        }
        else
        {
            type = BinaryenExpressionGetType(initializer_value);
        }

        BinaryenIndex index = this->function_locals[this->current_function_name].size();
        this->function_locals[this->current_function_name].push_back(type);

        BinaryenExpressionRef set_local = BinaryenLocalSet(this->mod, index, initializer_value);
        environment.declare(const_stmt->identifier.lexeme, index);

        this->stack.push(set_local);
    }

    void visit_func(Func *func)
    {
        auto func_name = func->identifier.lexeme;

        if (func->return_type.has_value())
        {
            this->function_return_types[func_name] = from_bird_type(func->return_type.value());
        }
        else
        {
            this->function_return_types[func_name] = BinaryenTypeNone();
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
            this->environment.declare(param.first.lexeme, index++);

            /*
             * this leaves values on the wasm stack if they arent used
             * in the function body, causing compilation errors. visit
             * primary will instead get the local variable and push to
             * stack if it is used in the function body.
             *
             *      current_function_body.push_back(
             *          BinaryenLocalGet(
             *              this->mod,
             *              index++,
             *              from_bird_type(param.second)));
             */
        }

        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();
            current_function_body.push_back(result);
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
                    condition,
                    then_branch,
                    else_branch));
        }
        else
        {
            this->stack.push(
                BinaryenIf(
                    this->mod,
                    condition,
                    then_branch,
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
            args.push_back(this->stack.pop());
        }

        this->stack.push(BinaryenCall(
            this->mod,
            func_name.c_str(),
            args.data(),
            args.size(),
            this->function_return_types[func_name]));
    }

    void visit_return_stmt(ReturnStmt *return_stmt)
    {
        if (return_stmt->expr.has_value())
        {
            return_stmt->expr.value()->accept(this);
            this->stack.push(
                BinaryenReturn(
                    this->mod,
                    this->stack.pop()));
        }
        else
        {
            this->stack.push(
                BinaryenReturn(
                    this->mod,
                    nullptr));
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
