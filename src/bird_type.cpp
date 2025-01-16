#include "bird_type.h"
#include "token.h"

std::shared_ptr<BirdType> bird_type_type_to_bird_type(BirdTypeType type)
{
    switch (type)
    {
    case BirdTypeType::INT:
        return std::make_shared<IntType>();
    case BirdTypeType::FLOAT:
        return std::make_shared<FloatType>();
    case BirdTypeType::STRING:
        return std::make_shared<StringType>();
    case BirdTypeType::BOOL:
        return std::make_shared<BoolType>();
    case BirdTypeType::VOID:
        return std::make_shared<VoidType>();
    case BirdTypeType::ERROR:
        return std::make_shared<ErrorType>();
    default:
        // cannot create a struct type, function type, or alias type
        return std::make_shared<ErrorType>();
    }
}

std::shared_ptr<BirdType> token_to_bird_type(Token token)
{
    std::string type_name = token.lexeme;
    if (type_name == "int")
    {
        return std::make_shared<IntType>();
    }
    else if (type_name == "float")
    {
        return std::make_shared<FloatType>();
    }
    else if (type_name == "bool")
    {
        return std::make_shared<BoolType>();
    }
    else if (type_name == "str")
    {
        return std::make_shared<StringType>();
    }
    else if (type_name == "void")
    {
        return std::make_shared<VoidType>();
    }
    else
    {
        // type_name is not primitive
        return std::make_shared<ErrorType>();
    }
}