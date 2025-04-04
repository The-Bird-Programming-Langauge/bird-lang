#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

Token::Type assign_expr_binary_equivalent(Token::Type token_type) {
  switch (token_type) {
  case Token::PLUS_EQUAL:
    return Token::PLUS;
  case Token::MINUS_EQUAL:
    return Token::MINUS;
  case Token::STAR_EQUAL:
    return Token::STAR;
  case Token::SLASH_EQUAL:
    return Token::SLASH;
  case Token::PERCENT_EQUAL:
    return Token::PERCENT;
  default:
    throw BirdException(
        "Invalid assign expr found when converting to binary equivalent");
  }
}

bool type_is_on_heap(const TypeTag type) {
  return type == TypeTag::STRUCT || type == TypeTag::ARRAY ||
         type == TypeTag::PLACEHOLDER || type == TypeTag::STRING;
}

const char *get_mem_set_for_type(const TypeTag type) {
  switch (type) {
  case TypeTag::FLOAT:
    return "mem_set_64";
  default:
    return "mem_set_32";
  }
}

const char *get_mem_get_for_type(const TypeTag type) {
  switch (type) {
  case TypeTag::FLOAT:
    return "mem_get_64";
  default:
    return "mem_get_32";
  }
}

unsigned int bird_type_byte_size(std::shared_ptr<BirdType> type) {
  switch (type->get_tag()) {
  case TypeTag::INT:
    return 4;
  case TypeTag::FLOAT:
    return 8;
  case TypeTag::BOOL:
    return 4;
  case TypeTag::VOID:
    return 0;
  case TypeTag::STRING:
    return 4;
  case TypeTag::CHAR:
    return 4;
  case TypeTag::STRUCT:
    return 4;
  case TypeTag::ARRAY:
    return 4;
  case TypeTag::PLACEHOLDER:
    return 4;
  default:
    return 0;
  }
}

BinaryenType bird_type_to_binaryen_type(std::shared_ptr<BirdType> bird_type) {
  if (bird_type->get_tag() == TypeTag::BOOL)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::INT)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::FLOAT)
    return BinaryenTypeFloat64();
  else if (bird_type->get_tag() == TypeTag::VOID)
    return BinaryenTypeNone();
  else if (bird_type->get_tag() == TypeTag::STRING)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::STRUCT)
    return BinaryenTypeInt32(); // ptr
  else if (bird_type->get_tag() == TypeTag::PLACEHOLDER)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::ARRAY)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::FUNCTION)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::CHAR)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::ITERATOR)
    return BinaryenTypeInt32();
  else if (bird_type->get_tag() == TypeTag::ERROR)
    throw BirdException("found error type");

  throw BirdException("invalid type");
}