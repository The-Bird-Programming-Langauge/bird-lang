#pragma once
#include "bird_type.h"
#include "parse_type.h"
#include "sym_table.h"
#include <functional>
#include <set>
#include <string>

/*
 * A class that converts ParseType to BirdType
 */

class TypeConverter {
  Environment<std::shared_ptr<BirdType>> &type_table;
  std::set<std::string> &struct_names;

public:
  TypeConverter(Environment<std::shared_ptr<BirdType>> &type_table,
                std::set<std::string> &struct_names)
      : type_table(type_table), struct_names(struct_names) {}

  TypeConverter() = delete;

  std::shared_ptr<BirdType> convert(std::shared_ptr<ParseType::Type> type) {
    if (type->tag == ParseType::Tag::PRIMITIVE) {
      Token token =
          safe_dynamic_pointer_cast<ParseType::Primitive, ParseType::Type>(type)
              ->type;
      switch (token.token_type) {
      case Token::Type::INT:
        return std::make_shared<IntType>();
      case Token::Type::UINT:
        return std::make_shared<UintType>();
      case Token::Type::FLOAT:
        return std::make_shared<FloatType>();
      case Token::Type::BOOL:
        return std::make_shared<BoolType>();
      case Token::Type::STR:
        return std::make_shared<StringType>();
      case Token::Type::VOID:
        return std::make_shared<VoidType>();
      default:
        return std::make_shared<ErrorType>();
      }

    } else if (type->tag == ParseType::Tag::USER_DEFINED) {
      Token token =
          safe_dynamic_pointer_cast<ParseType::UserDefined, ParseType::Type>(
              type)
              ->type;
      auto type_name = token.lexeme;

      if (this->type_table.contains(type_name)) {
        return this->type_table.get(type_name);
      }

      if (this->struct_names.find(type_name) != this->struct_names.end()) {
        return std::make_shared<PlaceholderType>(type_name);
      }

    } else if (type->tag == ParseType::Tag::ARRAY) {
      auto array_type =
          safe_dynamic_pointer_cast<ParseType::Array, ParseType::Type>(type);
      return std::make_shared<ArrayType>(this->convert(array_type->child));
    }

    return std::make_shared<ErrorType>();
  }
};
