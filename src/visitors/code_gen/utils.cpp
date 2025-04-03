#include "../../../include/visitors/code_gen.h"

bool type_is_on_heap(const TypeTag type) {
  return type == TypeTag::STRUCT || type == TypeTag::ARRAY ||
         type == TypeTag::PLACEHOLDER;
}

const char *get_mem_set_for_type(const TypeTag type) {
  switch (type) {
  case TypeTag::FLOAT:
    return "mem_set_64";
    break;
  case TypeTag::STRUCT:
  case TypeTag::ARRAY:
  case TypeTag::PLACEHOLDER:
    return "mem_set_ptr";
  default:
    return "mem_set_32";
  }
}

const char *get_mem_get_for_type(const TypeTag type) {
  // something in the array member access breaks if we use the
  // same sizes for mem_get as we do for mem_set

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
  case TypeTag::UINT:
    return 4;
  case TypeTag::FLOAT:
    return 8;
  case TypeTag::BOOL:
    return 4;
  case TypeTag::VOID:
    return 0;
  case TypeTag::STRING:
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
  switch (bird_type->get_tag()) {
  case TypeTag::BOOL:
    return BinaryenTypeInt32();
  case TypeTag::INT:
    return BinaryenTypeInt32();
  case TypeTag::UINT:
    return BinaryenTypeInt32();
  case TypeTag::FLOAT:
    return BinaryenTypeFloat64();
  case TypeTag::VOID:
    return BinaryenTypeNone();
  case TypeTag::STRING:
    return BinaryenTypeInt32();
  case TypeTag::STRUCT:
    return BinaryenTypeInt32(); // ptr
  case TypeTag::PLACEHOLDER:
    return BinaryenTypeInt32();
  case TypeTag::ARRAY:
    return BinaryenTypeInt32();
  case TypeTag::FUNCTION:
    return BinaryenTypeInt32();
  case TypeTag::ERROR:
    throw BirdException("found error type");
  default:
    throw BirdException("invalid type");
  }
}