#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include "token.h"
#include "exceptions/bird_exception.h"

/*
 * Enum for the types of the language, used for type checking
 */
enum class BirdTypeType
{
    INT,
    FLOAT,
    STRING,
    BOOL,
    VOID,
    ERROR,
    STRUCT,
    FUNCTION,
    PLACEHOLDER
};

// TODO: figure out how to do first class functions
struct BirdType
{
    BirdTypeType type;
    virtual ~BirdType() {};
    BirdType(BirdTypeType type) : type(type) {}

    bool operator==(const BirdType &other) const
    {
        return this->type == other.type;
    }

    bool operator!=(const BirdType &other) const
    {
        return this->type != other.type;
    }
};

struct IntType : BirdType
{
    IntType() : BirdType(BirdTypeType::INT) {}
    ~IntType() {};
};

struct FloatType : BirdType
{
    FloatType() : BirdType(BirdTypeType::FLOAT) {}
    ~FloatType() {};
};

struct StringType : BirdType
{
    StringType() : BirdType(BirdTypeType::STRING) {}
    ~StringType() {};
};

struct BoolType : BirdType
{
    BoolType() : BirdType(BirdTypeType::BOOL) {}
    ~BoolType() {};
};

struct VoidType : BirdType
{
    VoidType() : BirdType(BirdTypeType::VOID) {}
    ~VoidType() {};
};

struct ErrorType : BirdType
{
    ErrorType() : BirdType(BirdTypeType::ERROR) {}
    ~ErrorType() {};
};

struct AliasType : BirdType
{
    std::string name;
    std::shared_ptr<BirdType> alias;
    AliasType(std::string name, std::shared_ptr<BirdType> type)
        : BirdType(type->type), name(name), alias(std::move(type)) {}
    ~AliasType() {};
};

struct StructType : BirdType
{
    std::string name;
    std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields;
    StructType(std::string name, std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields)
        : BirdType(BirdTypeType::STRUCT), name(name), fields(std::move(fields)) {}
    ~StructType() {};
};

struct BirdFunction : BirdType
{
    std::vector<std::shared_ptr<BirdType>> params;
    std::shared_ptr<BirdType> ret;

    BirdFunction(std::vector<std::shared_ptr<BirdType>> params, std::shared_ptr<BirdType> ret)
        : BirdType(BirdTypeType::FUNCTION), params(std::move(params)), ret(std::move(ret)) {}
    ~BirdFunction() {};
};

std::shared_ptr<BirdType> bird_type_type_to_bird_type(BirdTypeType type);
std::shared_ptr<BirdType> token_to_bird_type(Token token);

struct PlaceholderType : BirdType
{
    std::string name;
    PlaceholderType(std::string name) : BirdType(BirdTypeType::PLACEHOLDER), name(name) {}
    ~PlaceholderType() {};
};

std::string bird_type_to_string(std::shared_ptr<BirdType> type);

template <typename T>
std::shared_ptr<T> safe_dynamic_pointer_cast(std::shared_ptr<BirdType> type)
{
    std::shared_ptr<T> result = std::dynamic_pointer_cast<T>(type);
    if (result.get() == nullptr)
    {
        throw BirdException("invalid cast, found " + bird_type_to_string(type));
    }

    return result;
}
