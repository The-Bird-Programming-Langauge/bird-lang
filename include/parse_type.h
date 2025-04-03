#pragma once
#include "token.h"
#include <memory>

namespace ParseType {
enum Tag { PRIMITIVE, USER_DEFINED, ARRAY };

struct Type {
  Tag tag;
  Type(Tag tag) : tag(tag) {}
  virtual ~Type() {}
  virtual Token get_token() = 0;
  virtual void set_token(Token token) = 0;
};

struct Primitive : Type {
  Token type;
  Primitive(Token type) : Type(Tag::PRIMITIVE), type(type) {}
  Token get_token() override { return type; }
  virtual void set_token(Token token) override { this->type = token; }
};

struct UserDefined : Type {
  Token type;
  UserDefined(Token type) : Type(Tag::USER_DEFINED), type(type) {}
  Token get_token() override { return type; }
  virtual void set_token(Token token) override { this->type = token; }
};

struct Array : Type {
  std::shared_ptr<Type> child;
  Array(std::shared_ptr<Type> child) : Type(Tag::ARRAY), child(child) {}
  Token get_token() override { return child->get_token(); }
  virtual void set_token(Token token) override {
    auto temp_child = this->child;
    while (temp_child->tag == Tag::ARRAY) {
      auto temp_array = std::dynamic_pointer_cast<ParseType::Array>(temp_child);
      temp_child = temp_array->child;
    }

    temp_child->set_token(token);
  };
};
} // namespace ParseType