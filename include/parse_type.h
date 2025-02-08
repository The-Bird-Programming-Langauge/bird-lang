#pragma once
#include "token.h"
#include <memory>

namespace ParseType
{
    enum Tag
    {
        PRIMITIVE,
        USER_DEFINED,
        ARRAY
    };

    struct Type
    {
        Tag tag;
        Type(Tag tag) : tag(tag) {}
        virtual ~Type() {}
    };

    struct Primitive : Type
    {
        Token type;
        Primitive(Token type) : Type(Tag::PRIMITIVE), type(type) {}
    };

    struct UserDefined : Type
    {
        Token type;
        UserDefined(Token type) : Type(Tag::USER_DEFINED), type(type) {}
    };

    struct Array : Type
    {
        std::shared_ptr<Type> child;
        Array(std::shared_ptr<Type> child) : Type(Tag::ARRAY), child(child) {}
    };
}