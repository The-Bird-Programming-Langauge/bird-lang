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

  virtual bool operator==(BirdType &other) const = 0;
  virtual bool operator!=(BirdType &other) const = 0;
};

std::string bird_type_to_string(std::shared_ptr<BirdType> type);
std::string bird_type_type_to_string(BirdTypeType type);

struct IntType : BirdType {
  IntType() : BirdType(BirdTypeType::INT) {}
  ~IntType() {};
  bool operator==(BirdType &other) const {
    return other.type == BirdTypeType::INT;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct FloatType : BirdType {
  FloatType() : BirdType(BirdTypeType::FLOAT) {}
  ~FloatType() {};
  bool operator==(BirdType &other) const {
    return other.type == BirdTypeType::FLOAT;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct StringType : BirdType {
  // TODO: figure out a way to remove this
  bool dynamic = false; // used in code gen
  StringType() : BirdType(BirdTypeType::STRING) {}
  StringType(bool dynamic) : BirdType(BirdTypeType::STRING), dynamic(dynamic) {}
  ~StringType() {};
  bool operator==(BirdType &other) const {
    return other.type == BirdTypeType::STRING;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct BoolType : BirdType {
  BoolType() : BirdType(BirdTypeType::BOOL) {}
  ~BoolType() {};
  bool operator==(BirdType &other) const {
    return other.type == BirdTypeType::BOOL;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct VoidType : BirdType {
  VoidType() : BirdType(BirdTypeType::VOID) {}
  ~VoidType() {};
  bool operator==(BirdType &other) const {
    return other.type == BirdTypeType::VOID;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct ErrorType : BirdType {
  ErrorType() : BirdType(BirdTypeType::ERROR) {}
  ~ErrorType() {};
  bool operator==(BirdType &other) const {
    return other.type == BirdTypeType::ERROR;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct StructType : BirdType {
  std::string name;
  std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields;
  StructType(
      std::string name,
      std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields)
      : BirdType(BirdTypeType::STRUCT), name(name), fields(std::move(fields)) {}
  ~StructType() {};
  bool operator==(BirdType &other) const {
    if (other.type == BirdTypeType::STRUCT) {
      return dynamic_cast<StructType *>(&other)->name == this->name;
    }
    return false;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct ArrayType : BirdType {
  std::shared_ptr<BirdType> element_type;
  ArrayType(std::shared_ptr<BirdType> element_type)
      : BirdType(BirdTypeType::ARRAY), element_type(std::move(element_type)) {}
  ~ArrayType() {};
  bool operator==(BirdType &other) const {
    if (other.type != BirdTypeType::ARRAY) {
      return false;
    }

    return *dynamic_cast<ArrayType *>(&other)->element_type ==
           *this->element_type;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

struct BirdFunction : BirdType {
  std::vector<std::shared_ptr<BirdType>> params;
  std::shared_ptr<BirdType> ret;

  BirdFunction(std::vector<std::shared_ptr<BirdType>> params,
               std::shared_ptr<BirdType> ret)
      : BirdType(BirdTypeType::FUNCTION), params(std::move(params)),
        ret(std::move(ret)) {}
  ~BirdFunction() {};
  bool operator==(BirdType &other) const {
    if (other.type != BirdTypeType::FUNCTION) {
      return false;
    }
    auto fn = dynamic_cast<BirdFunction *>(&other);

    if (fn->ret != this->ret) {
      return false;
    }

    if (fn->params.size() != this->params.size()) {
      return false;
    }

    for (int i = 0; i < this->params.size(); i++) {
      if (this->params[i] != fn->params[i]) {
        return false;
      }
    }

    return true;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

std::shared_ptr<BirdType> bird_type_type_to_bird_type(BirdTypeType type);
std::shared_ptr<BirdType> token_to_bird_type(Token token);

struct PlaceholderType : BirdType {
  std::string name;
  PlaceholderType(std::string name)
      : BirdType(BirdTypeType::PLACEHOLDER), name(name) {}
  ~PlaceholderType() {};
  bool operator==(BirdType &other) const {
    if (other.type == BirdTypeType::PLACEHOLDER) {
      auto placeholder = dynamic_cast<PlaceholderType *>(&other);
      return placeholder->name == this->name;
    }

    if (other.type == BirdTypeType::STRUCT) {
      auto struct_type = dynamic_cast<StructType *>(&other);
      return struct_type->name == this->name;
    }

    return false;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
};

template <typename T, typename U = BirdType>
std::shared_ptr<T> safe_dynamic_pointer_cast(std::shared_ptr<U> type) {
  std::shared_ptr<T> result = std::dynamic_pointer_cast<T>(type);
  if (result.get() == nullptr) {
    throw BirdException("invalid cast");
  }

  return result;
}
