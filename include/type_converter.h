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
  std::set<std::string> &struct_names;

public:
  TypeConverter(Environment<std::shared_ptr<BirdType>> &type_table,
                std::set<std::string> &struct_names)
      : struct_names(struct_names) {}

  TypeConverter() = delete;

  std::shared_ptr<BirdType>
  convert(Environment<std::shared_ptr<BirdType>> &type_table,
          std::shared_ptr<ParseType::Type> type) {
    if (type->tag == ParseType::Tag::PRIMITIVE) {
      Token token =
          safe_dynamic_pointer_cast<ParseType::Primitive, ParseType::Type>(type)
              ->type;
      auto type_name = token.lexeme;

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
      }
    } else if (type->tag == ParseType::Tag::USER_DEFINED) {
      Token token =
          safe_dynamic_pointer_cast<ParseType::UserDefined, ParseType::Type>(
              type)
              ->type;
      auto type_name = token.lexeme;

      if (type_table.contains(type_name)) {
        return type_table.get(type_name);
      }

      if (this->struct_names.find(type_name) != this->struct_names.end()) {
        return std::make_shared<PlaceholderType>(type_name);
      }
    } else if (type->tag == ParseType::Tag::ARRAY) {
      auto array_type =
          safe_dynamic_pointer_cast<ParseType::Array, ParseType::Type>(type);
      return std::make_shared<ArrayType>(
          this->convert(type_table, array_type->child));
    } else if (type->tag == ParseType::Tag::SCOPED_TYPE) {

      auto scoped_type =
          safe_dynamic_pointer_cast<ParseType::ScopedType, ParseType::Type>(
              type);

      auto struct_type =
          safe_dynamic_pointer_cast<ParseType::UserDefined, ParseType::Type>(
              scoped_type->type);

      Token token = struct_type->type;

      auto type_name = token.lexeme;

      if (type_table.contains(type_name)) {
        return type_table.get(type_name);
      }

      if (this->struct_names.find(type_name) != this->struct_names.end()) {
        return std::make_shared<PlaceholderType>(type_name);
      }
    }

    return std::make_shared<ErrorType>();
  }
};
