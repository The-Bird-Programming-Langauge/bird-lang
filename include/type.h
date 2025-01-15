#pragma once

#include "./token.h"
#include <utility>

struct Type
{
    Token type;

    Type() = default;
    Type(Token typeP) : type(typeP) {}
};

struct StructType : Type
{
    std::string name;
    std::vector<std::pair<std::string, Token>> fields;

    StructType() = default;
    StructType(std::string name, std::vector<std::pair<std::string, Token>> fields) : name(name), fields(fields) {}
};

struct SemanticType
{
    SemanticType() = default;
};