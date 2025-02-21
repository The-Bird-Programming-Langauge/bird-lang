#include "visitors/code_gen.h"

unsigned int bird_type_byte_size(std::shared_ptr<BirdType> type)
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