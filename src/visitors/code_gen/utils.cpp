#include "../../../include/visitors/code_gen.h"

const char *get_mem_set_for_type(const BirdTypeType type) {
  switch (type) {
  case BirdTypeType::FLOAT:
    return "mem_set_64";
    break;
  case BirdTypeType::STRUCT:
  case BirdTypeType::ARRAY:
  case BirdTypeType::PLACEHOLDER:
    return "mem_set_ptr";
  default:
    return "mem_set_32";
  }
}

unsigned int bird_type_byte_size(std::shared_ptr<BirdType> type) {
  switch (type->type) {
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

BinaryenType bird_type_to_binaryen_type(std::shared_ptr<BirdType> bird_type) {
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
  else if (bird_type->type == BirdTypeType::FUNCTION)
    return BinaryenTypeInt32();
  else if (bird_type->type == BirdTypeType::ERROR)
    throw BirdException("found error type");

  throw BirdException("invalid type");
}