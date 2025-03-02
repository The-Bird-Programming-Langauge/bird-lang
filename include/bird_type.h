#pragma once

#include "exceptions/bird_exception.h"
#include "token.h"
#include <iostream>
#include <memory>
#include <vector>

/*
 * Enum for the types of the language, used for type checking
 */
enum class BirdTypeType {
  INT,
  FLOAT,
  STRING,
  BOOL,
  VOID,
  ERROR,
  STRUCT,
  ARRAY,
  FUNCTION,
  PLACEHOLDER
};

// TODO: figure out how to do first class functions
struct BirdType {
  BirdTypeType type;
  virtual ~BirdType() {};
  BirdType(BirdTypeType type) : type(type) {}

  bool operator==(const BirdType &other) const {
    return this->type == other.type;
  }

  bool operator!=(const BirdType &other) const {
    return this->type != other.type;
  }
};

struct IntType : BirdType {
  IntType() : BirdType(BirdTypeType::INT) {}
  ~IntType() {};
};

struct FloatType : BirdType {
  FloatType() : BirdType(BirdTypeType::FLOAT) {}
  ~FloatType() {};
};

struct StringType : BirdType {
  // TODO: figure out a way to remove this
  bool dynamic = false; // used in code gen
  StringType() : BirdType(BirdTypeType::STRING) {}
  StringType(bool dynamic) : BirdType(BirdTypeType::STRING), dynamic(dynamic) {}
  ~StringType() {};
};

struct BoolType : BirdType {
  BoolType() : BirdType(BirdTypeType::BOOL) {}
  ~BoolType() {};
};

struct VoidType : BirdType {
  VoidType() : BirdType(BirdTypeType::VOID) {}
  ~VoidType() {};
};

struct ErrorType : BirdType {
  ErrorType() : BirdType(BirdTypeType::ERROR) {}
  ~ErrorType() {};
};

struct StructType : BirdType {
  std::string name;
  std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields;
  StructType(
      std::string name,
      std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields)
      : BirdType(BirdTypeType::STRUCT), name(name), fields(std::move(fields)) {}
  ~StructType() {};
};

struct ArrayType : BirdType {
  std::shared_ptr<BirdType> element_type;
  ArrayType(std::shared_ptr<BirdType> element_type)
      : BirdType(BirdTypeType::ARRAY), element_type(std::move(element_type)) {}
  ~ArrayType() {};
};

struct BirdFunction : BirdType {
  std::vector<std::shared_ptr<BirdType>> params;
  std::shared_ptr<BirdType> ret;

  BirdFunction(std::vector<std::shared_ptr<BirdType>> params,
               std::shared_ptr<BirdType> ret)
      : BirdType(BirdTypeType::FUNCTION), params(std::move(params)),
        ret(std::move(ret)) {}
  ~BirdFunction() {};
};

std::shared_ptr<BirdType> bird_type_type_to_bird_type(BirdTypeType type);
std::shared_ptr<BirdType> token_to_bird_type(Token token);

struct PlaceholderType : BirdType {
  std::string name;
  PlaceholderType(std::string name)
      : BirdType(BirdTypeType::PLACEHOLDER), name(name) {}
  ~PlaceholderType() {};
};

std::string bird_type_to_string(std::shared_ptr<BirdType> type);

template <typename T, typename U = BirdType>
std::shared_ptr<T> safe_dynamic_pointer_cast(std::shared_ptr<U> type) {
  std::shared_ptr<T> result = std::dynamic_pointer_cast<T>(type);
  if (result.get() == nullptr) {
    throw BirdException("invalid cast");
  }

  return result;
}
