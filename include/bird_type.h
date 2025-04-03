#pragma once

#include "exceptions/bird_exception.h"
#include "token.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

/*
 * Enum for the types of the language, used for type checking
 */
enum class TypeTag {
  INT,
  FLOAT,
  STRING,
  BOOL,
  VOID,
  ERROR,
  STRUCT,
  ARRAY,
  FUNCTION,
  PLACEHOLDER,
  CHAR,
  GENERIC // TODO: figure out a way to remove this
};

struct BirdType {
  virtual ~BirdType() {}
  virtual bool operator==(BirdType &other) const = 0;
  virtual bool operator!=(BirdType &other) const = 0;
  virtual TypeTag get_tag() const = 0;
  virtual std::string to_string() const = 0;
};

std::string bird_type_to_string(std::shared_ptr<BirdType> type);
std::string bird_type_type_to_string(TypeTag type);

struct IntType : BirdType {
  IntType() {}
  ~IntType() {};
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::INT;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::INT; }
  std::string to_string() const { return "int"; }
};

struct CharType : BirdType {
  CharType() {}
  ~CharType() {}
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::CHAR;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::CHAR; }
  std::string to_string() const { return "char"; }
};

struct FloatType : BirdType {
  FloatType() {}
  ~FloatType() {};
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::FLOAT;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::FLOAT; }
  std::string to_string() const { return "float"; }
};

struct StringType : BirdType {
  // TODO: figure out a way to remove this
  bool dynamic = false; // used in code gen
  StringType() {}
  StringType(bool dynamic) : dynamic(dynamic) {}
  ~StringType() {};
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::STRING;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::STRING; }
  std::string to_string() const { return "string"; }
};

struct BoolType : BirdType {
  BoolType() {}
  ~BoolType() {};
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::BOOL;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::BOOL; }
  std::string to_string() const { return "bool"; }
};

struct VoidType : BirdType {
  VoidType() {}
  ~VoidType() {};
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::VOID;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::VOID; }
  std::string to_string() const { return "void"; }
};

struct ErrorType : BirdType {
  ErrorType() {}
  ~ErrorType() {};
  bool operator==(BirdType &other) const {
    return other.get_tag() == TypeTag::ERROR;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::ERROR; }
  std::string to_string() const { return "error"; }
};

struct StructType : BirdType {
  std::string name;
  std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields;
  StructType(
      std::string name,
      std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> fields)
      : name(name), fields(std::move(fields)) {}
  ~StructType() {};
  bool operator==(BirdType &other) const {
    if (other.get_tag() == TypeTag::STRUCT) {
      return dynamic_cast<StructType *>(&other)->name == this->name;
    }
    return false;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::STRUCT; }
  std::string to_string() const { return "struct " + name; }
};

// used for internal library functions, is not exposed to the user
struct Generic : BirdType {
  bool operator==(BirdType &other) const { return true; }
  bool operator!=(BirdType &other) const { return false; }
  TypeTag get_tag() const { return TypeTag::GENERIC; }
  std::string to_string() const { return "generic"; }
};

struct ArrayType : BirdType {
  std::shared_ptr<BirdType> element_type;
  ArrayType(std::shared_ptr<BirdType> element_type)
      : element_type(std::move(element_type)) {}
  ~ArrayType() {};
  bool operator==(BirdType &other) const {
    if (other.get_tag() != TypeTag::ARRAY) {
      return false;
    }

    return *dynamic_cast<ArrayType *>(&other)->element_type ==
           *this->element_type;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::ARRAY; }
  std::string to_string() const { return element_type->to_string() + "[]"; }
};

struct BirdFunction : BirdType {
  std::vector<std::shared_ptr<BirdType>> params;
  std::shared_ptr<BirdType> ret;

  BirdFunction(std::vector<std::shared_ptr<BirdType>> params,
               std::shared_ptr<BirdType> ret)
      : params(std::move(params)), ret(std::move(ret)) {}
  ~BirdFunction() {};
  bool operator==(BirdType &other) const {
    if (other.get_tag() != TypeTag::FUNCTION) {
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
  TypeTag get_tag() const { return TypeTag::FUNCTION; }
  std::string to_string() const {
    std::string result = ret->to_string() + "(";
    std::for_each_n(params.begin(), params.size() - 1, [&](const auto &param) {
      result += param->to_string() + ", ";
    });

    result += params.back()->to_string();
    result += ")";
    return result;
  }
};

std::shared_ptr<BirdType> bird_type_type_to_bird_type(TypeTag type);
std::shared_ptr<BirdType> token_to_bird_type(Token token);

struct PlaceholderType : BirdType {
  std::string name;
  PlaceholderType(std::string name) : name(name) {}
  ~PlaceholderType() {};
  bool operator==(BirdType &other) const {
    if (other.get_tag() == TypeTag::PLACEHOLDER) {
      auto placeholder = dynamic_cast<PlaceholderType *>(&other);
      return placeholder->name == this->name;
    }

    if (other.get_tag() == TypeTag::STRUCT) {
      auto struct_type = dynamic_cast<StructType *>(&other);
      return struct_type->name == this->name;
    }

    return false;
  }
  bool operator!=(BirdType &other) const { return !(*this == other); }
  TypeTag get_tag() const { return TypeTag::PLACEHOLDER; }
  std::string to_string() const { return "placeholder " + name; }
};

template <typename T, typename U = BirdType>
std::shared_ptr<T> safe_dynamic_pointer_cast(std::shared_ptr<U> type) {
  std::shared_ptr<T> result = std::dynamic_pointer_cast<T>(type);
  if (result.get() == nullptr) {
    throw BirdException("invalid cast");
  }

  return result;
}
