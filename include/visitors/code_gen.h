#pragma once
#include "binaryen-c.h"
#include "bird_type.h"
#include "hoist_visitor.h"
#include "static_visitor.h"
#include <fstream>
#include <algorithm>
#include <ios>
#include <map>
#include "type_converter.h"
#include "stack.h"
#include "sym_table.h"

static unsigned int bird_type_byte_size(std::shared_ptr<BirdType> type) // in i32s
{
    switch (type->type)
    {
    case BirdTypeType::INT:
        return 5;
    case BirdTypeType::FLOAT:
        return 9;
    case BirdTypeType::BOOL:
        return 5;
    case BirdTypeType::VOID:
        return 0;
    case BirdTypeType::STRING:
        return 5;
    case BirdTypeType::STRUCT:
        return 5;
    case BirdTypeType::ARRAY:
        return 5;
    case BirdTypeType::PLACEHOLDER:
        return 5;
    default:
        return 0;
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
using TaggedBinaryOpFn = Tagged<std::function<BinaryenOp(void)>>;

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
    // and and or are short circuiting operators, so we need to handle them differently
    const std::map<Token::Type, std::map<std::pair<BirdTypeType, BirdTypeType>, TaggedBinaryOpFn>> binary_operations = {
        {Token::Type::PLUS, {
                                {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenAddInt32, std::make_shared<IntType>())},
                                {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenAddFloat64, std::make_shared<FloatType>())},
                                {{BirdTypeType::STRING, BirdTypeType::STRING}, TaggedBinaryOpFn(BinaryenAddInt32, std::make_shared<StringType>())},
                            }},
        {Token::Type::MINUS, {
                                 {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenSubInt32, std::make_shared<IntType>())},
                                 {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenSubFloat64, std::make_shared<FloatType>())},
                             }},
        {Token::Type::STAR, {
                                {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenMulInt32, std::make_shared<IntType>())},
                                {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenMulFloat64, std::make_shared<FloatType>())},
                            }},
        {Token::Type::SLASH, {
                                 {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenDivSInt32, std::make_shared<IntType>())},
                                 {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenDivFloat64, std::make_shared<FloatType>())},
                             }},
        {Token::Type::EQUAL_EQUAL, {
                                       {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenEqInt32, std::make_shared<BoolType>())},
                                       {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenEqFloat64, std::make_shared<BoolType>())},
                                       {{BirdTypeType::STRING, BirdTypeType::STRING}, TaggedBinaryOpFn(BinaryenEqInt32, std::make_shared<BoolType>())},
                                       {{BirdTypeType::BOOL, BirdTypeType::BOOL}, TaggedBinaryOpFn(BinaryenEqInt32, std::make_shared<BoolType>())},
                                   }},
        {Token::Type::BANG_EQUAL, {
                                      {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenNeInt32, std::make_shared<BoolType>())},
                                      {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenNeFloat64, std::make_shared<BoolType>())},
                                      {{BirdTypeType::STRING, BirdTypeType::STRING}, TaggedBinaryOpFn(BinaryenNeInt32, std::make_shared<BoolType>())},
                                      {{BirdTypeType::BOOL, BirdTypeType::BOOL}, TaggedBinaryOpFn(BinaryenNeInt32, std::make_shared<BoolType>())},
                                  }},
        {Token::Type::GREATER, {
                                   {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenGtSInt32, std::make_shared<BoolType>())},
                                   {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenGtFloat64, std::make_shared<BoolType>())},
                               }},
        {Token::Type::GREATER_EQUAL, {
                                         {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenGeSInt32, std::make_shared<BoolType>())},
                                         {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenGeFloat64, std::make_shared<BoolType>())},
                                     }},
        {Token::Type::LESS, {
                                {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenLtSInt32, std::make_shared<BoolType>())},
                                {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenLtFloat64, std::make_shared<BoolType>())},
                            }},
        {Token::Type::LESS_EQUAL, {
                                      {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenLeSInt32, std::make_shared<BoolType>())},
                                      {{BirdTypeType::FLOAT, BirdTypeType::FLOAT}, TaggedBinaryOpFn(BinaryenLeFloat64, std::make_shared<BoolType>())},
                                  }},
        {Token::Type::XOR, {
                               {{BirdTypeType::BOOL, BirdTypeType::BOOL}, TaggedBinaryOpFn(BinaryenXorInt32, std::make_shared<BoolType>())},
                           }},
        {Token::Type::PERCENT, {
                                   {{BirdTypeType::INT, BirdTypeType::INT}, TaggedBinaryOpFn(BinaryenRemSInt32, std::make_shared<IntType>())},
                               }},
    };

public:
    Environment<TaggedIndex> environment; // tracks the index of local variables
    Environment<std::shared_ptr<BirdType>> type_table;
    Stack<TaggedExpression> stack; // for returning values
    std::map<std::string, std::string> std_lib;
    std::set<std::string> struct_names;

    std::unordered_map<std::string, uint32_t> str_offsets;

    // we need the function return types when calling functions
    std::unordered_map<std::string, TaggedType> function_return_types;
    // allows us to track the local variables of a function
    std::unordered_map<std::string, std::vector<BinaryenType>> function_locals;
    std::string current_function_name;          // for indexing into maps
    std::vector<MemorySegment> memory_segments; // store memory segments to add at once

    TypeConverter type_converter;

    uint32_t current_offset = 0;
    BinaryenModuleRef mod;

    ~CodeGen()
    {
        BinaryenModuleDispose(this->mod);
    }

    CodeGen() : type_converter(this->type_table, this->struct_names), mod(BinaryenModuleCreate())
    {
        this->environment.push_env();
        this->type_table.push_env();
    }

    void init_std_lib();
    void add_memory_segment(const std::string &str);
    void init_static_memory(std::vector<std::string> &strings);
    void generate(std::vector<std::unique_ptr<Stmt>> *stmts);

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
        else if (bird_type->type == BirdTypeType::PLACEHOLDER)
            return BinaryenTypeInt32();
        else if (bird_type->type == BirdTypeType::ARRAY)
            return BinaryenTypeInt32();

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
        else if (token->type == BirdTypeType::STRING || token->type == BirdTypeType::STRUCT || token->type == BirdTypeType::ARRAY)
            return BinaryenTypeInt32();
        else
        {

            throw BirdException("invalid type");
        }
    }

    // perform garbage collection on memory data by marking and sweeping dynamically allocated blocks
    void garbage_collect();
    void visit_block(Block *block);
    void visit_decl_stmt(DeclStmt *decl_stmt);
    void visit_assign_expr(AssignExpr *assign_expr);
    void visit_print_stmt(PrintStmt *print_stmt);
    void visit_expr_stmt(ExprStmt *expr_stmt);
    void visit_while_stmt(WhileStmt *while_stmt);
    void visit_for_stmt(ForStmt *for_stmt);
    void visit_binary(Binary *binary);
    void visit_binary_short_circuit(Binary *binary);
    void visit_binary_normal(Binary *binary);
    void visit_unary(Unary *unary);

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

            if (this->str_offsets.find(str_value) == this->str_offsets.end())
            {
                throw BirdException("string not found: " + str_value);
            }

            BinaryenExpressionRef str_ptr = BinaryenConst(this->mod, BinaryenLiteralInt32(this->str_offsets[str_value]));
            this->stack.push(TaggedExpression(str_ptr, std::shared_ptr<BirdType>(new StringType())));
            break;
        }

        case Token::Type::IDENTIFIER:
        {
            TaggedIndex tagged_index = this->environment.get(primary->value.lexeme);
            BinaryenExpressionRef local_get = this->binaryen_get(primary->value.lexeme);

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

        // May need to make this a tagged expression
        this->stack.push(
            BinaryenIf(
                this->mod,
                condition.value,
                true_expr.value,
                false_expr.value));
    }

    void visit_const_stmt(ConstStmt *const_stmt)
    {
        const_stmt->value->accept(this);
        TaggedExpression initializer = this->stack.pop();

        std::shared_ptr<BirdType> type;
        if (const_stmt->type.has_value())
        {
            type = this->type_converter.convert(const_stmt->type.value());
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

        BinaryenExpressionRef set_local = this->binaryen_set(const_stmt->identifier.lexeme, initializer.value);

        this->stack.push(TaggedExpression(set_local, type));
    }

    void visit_func(Func *func)
    {
        auto func_name = func->identifier.lexeme;

        if (func->return_type.has_value())
        {
            auto bird_return_type = this->type_converter.convert(func->return_type.value());
            auto binaryen_return_type = this->bird_type_to_binaryen_type(bird_return_type);

            this->function_return_types[func_name] = TaggedType(binaryen_return_type, bird_return_type);
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
            auto param_type = this->type_converter.convert(param.second);
            param_types.push_back(this->bird_type_to_binaryen_type(param_type));
            this->function_locals[func_name].push_back(this->bird_type_to_binaryen_type(param_type));
        }

        BinaryenType params = BinaryenTypeCreate(param_types.data(), param_types.size());

        BinaryenType result_type = func->return_type.has_value()
                                       ? this->bird_type_to_binaryen_type(this->type_converter.convert(func->return_type.value()))
                                       : BinaryenTypeNone();

        this->environment.push_env();

        auto index = 0;
        for (auto &param : func->param_list)
        {
            this->environment.declare(param.first.lexeme, TaggedIndex(index++, this->type_converter.convert(param.second)));
        }

        for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts)
        {
            stmt->accept(this);
            auto result = this->stack.pop();

            if (result.type->type != BirdTypeType::VOID)
            {
                current_function_body.push_back(
                    BinaryenDrop(
                        this->mod,
                        result.value));
            }
            else
            {
                current_function_body.push_back(result.value);
            }
        }

        // perform garbage collection at the end of a function by popping the javascript calls off the stack in a block and executing the block

        this->environment.pop_env();

        this->garbage_collect();
        auto calls_block = this->stack.pop();
        current_function_body.push_back(calls_block.value);

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

            if (*result.type != *func_return_type.type)
            {
                throw BirdException("return type mismatch");
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
        this->type_table.declare(
            type_stmt->identifier.lexeme,
            this->type_converter.convert(type_stmt->type_token));
    }

    void visit_subscript(Subscript *subscript)
    {
        subscript->subscriptable->accept(this);
        auto subscriptable = this->stack.pop();

        subscript->index->accept(this);
        auto index = this->stack.pop();

        std::shared_ptr<BirdType> type;
        if (subscriptable.type->type == BirdTypeType::ARRAY)
        {
            type = safe_dynamic_pointer_cast<ArrayType>(subscriptable.type)->element_type;
        }

        // TODO: make this better
        BinaryenExpressionRef args[2] = {subscriptable.value,
                                         subscriptable.type->type == BirdTypeType::ARRAY ? BinaryenBinary(
                                                                                               this->mod,
                                                                                               BinaryenMulInt32(),
                                                                                               index.value,
                                                                                               BinaryenConst(
                                                                                                   this->mod,
                                                                                                   BinaryenLiteralInt32(bird_type_byte_size(type))))
                                                                                         : index.value};

        this->stack.push(
            TaggedExpression(
                BinaryenCall(
                    this->mod,
                    type->type == BirdTypeType::FLOAT ? "mem_get_64"
                                                      : "mem_get_32",
                    args,
                    2,
                    BinaryenTypeInt32()),
                std::shared_ptr<BirdType>(type)));
    }

    /*
    This function is called when a struct declaration is encountered.
    The struct's shape is stored in the type table.
    */
    void visit_struct_decl(StructDecl *struct_decl)
    {
        std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> struct_fields;
        std::transform(struct_decl->fields.begin(), struct_decl->fields.end(), std::back_inserter(struct_fields), [&](std::pair<std::string, std::shared_ptr<ParseType::Type>> field)
                       { return std::make_pair(field.first, this->type_converter.convert(field.second)); });

        type_table.declare(struct_decl->identifier.lexeme, std::make_shared<StructType>(struct_decl->identifier.lexeme, struct_fields));
    }

    void visit_direct_member_access(DirectMemberAccess *direct_member_access)
    {
        direct_member_access->accessable->accept(this);
        auto accessable = this->stack.pop();

        std::shared_ptr<StructType> struct_type;
        if (accessable.type->type == BirdTypeType::PLACEHOLDER)
        {
            std::shared_ptr<PlaceholderType> placeholder = safe_dynamic_pointer_cast<PlaceholderType>(accessable.type);
            if (this->struct_names.find(placeholder->name) == this->struct_names.end())
            {
                throw BirdException("struct not found");
            }

            struct_type = safe_dynamic_pointer_cast<StructType>(this->type_table.get(placeholder->name));
        }
        else
        {
            struct_type = safe_dynamic_pointer_cast<StructType>(accessable.type);
        }

        auto offset = 0;
        std::shared_ptr<BirdType> member_type;
        for (auto &field : struct_type->fields)
        {

            if (field.first == direct_member_access->identifier.lexeme)
            {
                member_type = field.second;
                break;
            }

            offset += bird_type_byte_size(field.second);
        }

        BinaryenExpressionRef args[2] = {accessable.value, BinaryenConst(this->mod, BinaryenLiteralInt32(offset))};

        auto func_name =
            member_type->type == BirdTypeType::FLOAT
                ? "mem_get_64"
                : "mem_get_32";

        this->stack.push(
            TaggedExpression(
                BinaryenCall(
                    this->mod,
                    func_name,
                    args,
                    2,
                    bird_type_to_binaryen_type(member_type)),
                member_type));
    }

    // map from struct name to function name
    std::unordered_map<std::string, std::string> struct_constructors;
    void visit_struct_initialization(StructInitialization *struct_initialization)
    {
        auto type = this->type_table.get(struct_initialization->identifier.lexeme);

        std::shared_ptr<StructType> struct_type = safe_dynamic_pointer_cast<StructType>(type);
        if (struct_constructors.find(struct_initialization->identifier.lexeme) == struct_constructors.end())
        {
            // declare a function based on the struct fields
            // call the function with the struct fields OR with default values

            unsigned int size = 0;
            for (auto &field : struct_type->fields)
            {
                size += bird_type_byte_size(field.second);
            }

            std::vector<BinaryenExpressionRef> constructor_body;

            auto size_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(size));
            auto call = BinaryenCall(
                this->mod,
                "mem_alloc",
                &size_literal,
                1,
                BinaryenTypeInt32());

            std::vector<BinaryenType> param_types;
            for (auto &field : struct_type->fields)
            {
                param_types.push_back(bird_type_to_binaryen_type(field.second));
            }

            constructor_body.push_back(BinaryenLocalSet(this->mod, param_types.size(), call));

            int count = 0;
            for (auto &field : struct_type->fields)
            {
                auto type = field.second;

                auto offset = 0;
                for (auto &struct_field : struct_type->fields)
                {
                    if (struct_field.first == field.first)
                        break;

                    offset += bird_type_byte_size(struct_field.second);
                }

                BinaryenExpressionRef args[3] = {BinaryenLocalGet(this->mod, param_types.size(), BinaryenTypeInt32()),
                                                 BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
                                                 BinaryenLocalGet(this->mod, count++, bird_type_to_binaryen_type(type))};
                auto func_name =
                    type->type == BirdTypeType::FLOAT
                        ? "mem_set_64"
                    : type->type == BirdTypeType::STRUCT || type->type == BirdTypeType::PLACEHOLDER ? "mem_set_ptr"
                                                                                                    : "mem_set_32";

                constructor_body.push_back(
                    BinaryenCall(
                        this->mod,
                        func_name,
                        args,
                        3,
                        BinaryenTypeNone()));
            }

            constructor_body.push_back(BinaryenReturn(this->mod, BinaryenLocalGet(this->mod, param_types.size(), BinaryenTypeInt32())));

            auto constructor_var_types = BinaryenTypeInt32();
            BinaryenAddFunction(this->mod,
                                struct_initialization->identifier.lexeme.c_str(),
                                BinaryenTypeCreate(param_types.data(), param_types.size()),
                                BinaryenTypeInt32(),
                                &constructor_var_types,
                                1,
                                BinaryenBlock(this->mod, nullptr, constructor_body.data(), constructor_body.size(), BinaryenTypeNone()));

            struct_constructors[struct_initialization->identifier.lexeme] = struct_initialization->identifier.lexeme;
        }

        std::vector<BinaryenExpressionRef> args;

        for (auto &field : struct_type->fields)
        {
            auto found = false;
            for (auto &field_assignment : struct_initialization->field_assignments)
            {
                if (field.first == field_assignment.first)
                {
                    found = true;
                    field_assignment.second->accept(this);
                    auto field_value = this->stack.pop();
                    args.push_back(field_value.value);
                    break;
                }
            }

            if (!found)
            {
                auto default_value = field.second->type == BirdTypeType::FLOAT ? BinaryenConst(this->mod, BinaryenLiteralFloat64(0.0)) : BinaryenConst(this->mod, BinaryenLiteralInt32(0));
                args.push_back(default_value);
            }
        }

        this->stack.push(
            TaggedExpression(
                BinaryenCall(
                    this->mod,
                    struct_constructors[struct_initialization->identifier.lexeme].c_str(),
                    args.data(),
                    args.size(),
                    BinaryenTypeInt32()),
                struct_type));
    }

    void visit_member_assign(MemberAssign *member_assign)
    {
        member_assign->value->accept(this);
        auto value = this->stack.pop();

        member_assign->accessable->accept(this);
        auto accessable = this->stack.pop();

        std::shared_ptr<StructType> struct_type = safe_dynamic_pointer_cast<StructType>(accessable.type);

        auto offset = 0;
        for (auto &field : struct_type->fields)
        {
            if (field.first == member_assign->identifier.lexeme)
            {
                break;
            }

            offset += bird_type_byte_size(field.second);
        }

        BinaryenExpressionRef args[3] = {accessable.value, BinaryenConst(this->mod, BinaryenLiteralInt32(offset)), value.value};

        auto func_name =
            value.type->type == BirdTypeType::FLOAT
                ? "mem_set_64"
            : value.type->type == BirdTypeType::STRUCT || value.type->type == BirdTypeType::PLACEHOLDER ? "mem_set_ptr"
                                                                                                        : "mem_set_32";

        this->stack.push(
            TaggedExpression(
                BinaryenCall(
                    this->mod,
                    func_name,
                    args,
                    3,
                    BinaryenTypeNone()),
                std::shared_ptr<BirdType>(new VoidType())));
    }

    void visit_as_cast(AsCast *as_cast)
    {
        as_cast->expr->accept(this);
        auto expr = this->stack.pop();

        std::shared_ptr<BirdType> to_type = this->type_converter.convert(as_cast->type);

        if (to_type->type == BirdTypeType::INT && expr.type->type == BirdTypeType::FLOAT)
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenUnary(
                        this->mod,
                        BinaryenTruncSatSFloat64ToInt32(),
                        expr.value),
                    std::shared_ptr<BirdType>(new IntType())));
            return;
        }
        else if (to_type->type == BirdTypeType::FLOAT && expr.type->type == BirdTypeType::INT)
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenUnary(
                        this->mod,
                        BinaryenConvertSInt32ToFloat64(),
                        expr.value),
                    std::shared_ptr<BirdType>(new FloatType())));
            return;
        }

        this->stack.push(expr);
    }

    BinaryenExpressionRef binaryen_set(std::string identifier, BinaryenExpressionRef value)
    {
        // std::cout << identifier << ": " << this->environment.get_depth(identifier) << std::endl;

        TaggedIndex tagged_index = this->environment.get(identifier);
        if (this->environment.get_depth(identifier) != 0 && this->current_function_name != "main")
        {
            return BinaryenLocalSet(this->mod, tagged_index.value, value);
        }
        else
        {
            return BinaryenGlobalSet(this->mod, std::to_string(tagged_index.value).c_str(), value);
        }
    }

    BinaryenExpressionRef binaryen_get(std::string identifier)
    {
        TaggedIndex tagged_index = this->environment.get(identifier);
        if (this->environment.get_depth(identifier) != 0 && this->current_function_name != "main")
        {
            return BinaryenLocalGet(this->mod, tagged_index.value, from_bird_type(tagged_index.type));
        }
        else
        {
            return BinaryenGlobalGet(this->mod, std::to_string(tagged_index.value).c_str(), from_bird_type(tagged_index.type));
        }
    }

    void visit_array_init(ArrayInit *array_init)
    {
        std::vector<BinaryenExpressionRef> children;
        std::vector<BinaryenExpressionRef> vals;

        unsigned int size = 0;
        std::shared_ptr<BirdType> type;
        for (auto &element : array_init->elements)
        {
            element->accept(this);
            auto val = this->stack.pop();
            type = val.type;

            vals.push_back(val.value);
            size += bird_type_byte_size(val.type);
        }

        auto locals = this->function_locals[this->current_function_name];
        this->function_locals[this->current_function_name].push_back(BinaryenTypeInt32());

        auto identifier = std::to_string(locals.size()) + "temp";
        this->environment.declare(identifier, TaggedIndex(locals.size(),
                                                          type));

        auto size_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(size));
        BinaryenExpressionRef local_set = this->binaryen_set(identifier,
                                                             BinaryenCall(this->mod,
                                                                          "mem_alloc",
                                                                          &size_literal,
                                                                          1,
                                                                          BinaryenTypeInt32()));

        children.push_back(local_set);

        unsigned int offset = 0;
        for (auto val : vals)
        {
            BinaryenExpressionRef args[3] = {
                this->binaryen_get(identifier),
                BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
                val};

            children.push_back(
                BinaryenCall(this->mod,
                             type->type == BirdTypeType::FLOAT ? "mem_set_64"
                                                               : "mem_set_32",
                             args,
                             3,
                             BinaryenTypeNone()));

            offset += bird_type_byte_size(type);
        }

        children.push_back(this->binaryen_get(identifier));

        auto block = BinaryenBlock(this->mod, nullptr, children.data(), children.size(), BinaryenTypeInt32());

        this->stack.push(TaggedExpression(block, std::make_shared<ArrayType>(type)));
    }

    void visit_index_assign(IndexAssign *index_assign)
    {
        index_assign->lhs->subscriptable->accept(this);
        auto lhs = this->stack.pop();

        index_assign->lhs->accept(this);
        auto lhs_val = this->stack.pop();

        index_assign->lhs->index->accept(this);
        auto index = this->stack.pop();

        index_assign->rhs->accept(this);
        auto rhs_val = this->stack.pop();

        auto index_literal = BinaryenBinary(
            this->mod, BinaryenMulInt32(),
            index.value,
            BinaryenConst(this->mod, BinaryenLiteralInt32(bird_type_byte_size(lhs_val.type))));

        bool float_flag = (lhs_val.type->type == BirdTypeType::FLOAT && rhs_val.type->type == BirdTypeType::FLOAT);

        BinaryenExpressionRef result;
        switch (index_assign->op.token_type)
        {
        case Token::Type::EQUAL:
        {
            result = rhs_val.value;

            break;
        }
        case Token::Type::PLUS_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenAddFloat64(), lhs_val.value, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenAddInt32(), lhs_val.value, rhs_val.value);
            break;
        }
        case Token::Type::MINUS_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenSubFloat64(), lhs_val.value, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenSubInt32(), lhs_val.value, rhs_val.value);

            break;
        }
        case Token::Type::STAR_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenMulFloat64(), lhs_val.value, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenMulInt32(), lhs_val.value, rhs_val.value);

            break;
        }
        case Token::Type::SLASH_EQUAL:
        {
            result = (float_flag)
                         ? BinaryenBinary(this->mod, BinaryenDivFloat64(), lhs_val.value, rhs_val.value)
                         : BinaryenBinary(this->mod, BinaryenDivSInt32(), lhs_val.value, rhs_val.value);

            break;
        }
        case Token::Type::PERCENT_EQUAL:
        {
            result = (float_flag)
                         ? throw BirdException("Modular operation requires integer values")
                         : BinaryenBinary(this->mod, BinaryenRemSInt32(), lhs_val.value, rhs_val.value);

            break;
        }
        default:
            throw BirdException("Unidentified assignment operator " + index_assign->op.lexeme);
            break;
        }

        BinaryenExpressionRef args[3] = {lhs.value, index_literal, result};

        this->stack.push(
            BinaryenCall(
                this->mod,
                lhs_val.type->type == BirdTypeType::FLOAT ? "mem_set_64"
                                                          : "mem_set_32",
                args,
                3,
                BinaryenTypeNone()));
    }
};
