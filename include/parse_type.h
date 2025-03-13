#pragma once
#include "token.h"
#include <memory>
#include <vector>

namespace ParseType {
enum Tag { PRIMITIVE, USER_DEFINED, ARRAY, SCOPED_TYPE };

struct Type {
  Tag tag;
  Type(Tag tag) : tag(tag) {}
  virtual ~Type() {}
  virtual Token get_token() = 0;
};

struct Primitive : Type {
  Token type;
  Primitive(Token type) : Type(Tag::PRIMITIVE), type(type) {}
  Token get_token() override { return type; }
};

struct UserDefined : Type {
  Token type;
  UserDefined(Token type) : Type(Tag::USER_DEFINED), type(type) {}
  Token get_token() override { return type; }
};

struct Array : Type {
  std::shared_ptr<Type> child;
  Array(std::shared_ptr<Type> child) : Type(Tag::ARRAY), child(child) {}
  Token get_token() override { return child->get_token(); }
};

struct ScopedType : Type {
  std::vector<Token> path;
  std::shared_ptr<Type> type;

  ScopedType(std::vector<Token> path, std::shared_ptr<Type> type)
      : Type(Tag::SCOPED_TYPE), path(path), type(std::move(type)) {}
  Token get_token() override { return type->get_token(); }
};
} // namespace ParseType