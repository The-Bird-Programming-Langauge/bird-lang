#include "../include/bird_type.h"
#include "../include/token.h"

std::shared_ptr<BirdType> bird_type_type_to_bird_type(TypeTag type) {
  switch (type) {
  case TypeTag::INT:
    return std::make_shared<IntType>();
  case TypeTag::FLOAT:
    return std::make_shared<FloatType>();
  case TypeTag::STRING:
    return std::make_shared<StringType>();
  case TypeTag::BOOL:
    return std::make_shared<BoolType>();
  case TypeTag::VOID:
    return std::make_shared<VoidType>();
  case TypeTag::ERROR:
    return std::make_shared<ErrorType>();
  default:
    // cannot create a struct type, function type, or alias type
    return std::make_shared<ErrorType>();
  }
}

std::shared_ptr<BirdType> token_to_bird_type(Token token) {
  std::string type_name = token.lexeme;
  if (type_name == "int") {
    return std::make_shared<IntType>();
  } else if (type_name == "float") {
    return std::make_shared<FloatType>();
  } else if (type_name == "bool") {
    return std::make_shared<BoolType>();
  } else if (type_name == "str") {
    return std::make_shared<StringType>();
  } else if (type_name == "void") {
    return std::make_shared<VoidType>();
  } else {
    // type_name is not primitive
    return std::make_shared<ErrorType>();
  }
}

std::string bird_type_type_to_string(TypeTag type) {
  switch (type) {
  case TypeTag::INT:
    return "int";
  case TypeTag::FLOAT:
    return "float";
  case TypeTag::BOOL:
    return "bool";
  case TypeTag::VOID:
    return "void";
  case TypeTag::STRING:
    return "string";
  case TypeTag::STRUCT:
    return "struct";
  case TypeTag::ARRAY:
    return "array";
  case TypeTag::PLACEHOLDER:
    return "placeholder";
  case TypeTag::FUNCTION:
    return "function";
  case TypeTag::ERROR:
    return "error";
  case TypeTag::CHAR:
    return "char";
  case TypeTag::GENERIC:
    return "generic";
  }
}

std::string bird_type_to_string(std::shared_ptr<BirdType> type) {
  return bird_type_type_to_string(type->get_tag());
}