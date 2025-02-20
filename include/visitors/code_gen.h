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
    void visit_primary(Primary *primary);
    void visit_ternary(Ternary *ternary);
    void visit_const_stmt(ConstStmt *const_stmt);
    void visit_func(Func *func);
    void visit_if_stmt(IfStmt *if_stmt);
    void visit_call(Call *call);
    void visit_return_stmt(ReturnStmt *return_stmt);
    void visit_break_stmt(BreakStmt *break_stmt);
    void visit_continue_stmt(ContinueStmt *continue_stmt);
    void visit_type_stmt(TypeStmt *type_stmt);
    void visit_subscript(Subscript *subscript);

    /*
    This function is called when a struct declaration is encountered.
    The struct's shape is stored in the type table.
    */
    void visit_struct_decl(StructDecl *struct_decl);
    void visit_direct_member_access(DirectMemberAccess *direct_member_access);

    // map from struct name to function name
    std::unordered_map<std::string, std::string> struct_constructors;
    void visit_struct_initialization(StructInitialization *struct_initialization);
    void visit_member_assign(MemberAssign *member_assign);
    void visit_as_cast(AsCast *as_cast);

    BinaryenExpressionRef binaryen_set(std::string identifier, BinaryenExpressionRef value);
    BinaryenExpressionRef binaryen_get(std::string identifier);

    void visit_array_init(ArrayInit *array_init);
    void visit_index_assign(IndexAssign *index_assign);
};
