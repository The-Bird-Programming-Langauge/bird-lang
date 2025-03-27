#pragma once
#include "ast_node/expr/lambda.h"
#include "bird_type.h"
#include "parse_type.h"
#include "sym_table.h"
#include <functional>
#include <memory>
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

  std::shared_ptr<BirdType> convert(std::shared_ptr<ParseType::Type> type,
                                    const std::string &name_mangler = "") {
    if (type->tag == ParseType::Tag::PRIMITIVE) {
      Token token = type->get_token();
      std::cout << "convert: " << token.lexeme << std::endl;
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
      Token token = type->get_token();
      std::cout << "USER DEFINED: " << token.lexeme << std::endl;
      auto type_name = token.lexeme;

      if (this->type_table.contains(type_name)) {
        return this->type_table.get(type_name);
      }

      if (this->struct_names.find(type_name) != this->struct_names.end()) {
        return std::make_shared<PlaceholderType>(type_name);
      }
    } else if (type->tag == ParseType::Tag::ARRAY) {
      auto array_type = std::dynamic_pointer_cast<ParseType::Array>(type);
      return std::make_shared<ArrayType>(this->convert(array_type->child));
    } else if (type->tag == ParseType::Tag::FUNCTION) {
      auto function_type = std::dynamic_pointer_cast<ParseType::Function>(type);

      std::vector<std::shared_ptr<BirdType>> params;
      for (auto param : function_type->params) {
        params.push_back(this->convert(param));
      }

      auto ret = this->convert(function_type->ret);
      return std::make_shared<LambdaFunction>(params, ret);
    }

    return std::make_shared<ErrorType>();
  }
};
