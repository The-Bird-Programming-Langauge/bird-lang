#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../../exceptions/bird_exception.h"
#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

// forward declaration
class Expr;

/*
 * Type statement AST Node that represents type declarations
 * ex:
 * type x = int;
 */
class StructDecl : public Stmt {
  void initialize_methods_and_fields(
      std::vector<
          std::variant<std::shared_ptr<Stmt>,
                       std::pair<Token, std::shared_ptr<ParseType::Type>>>>
          fields_and_fns) {
    for (auto &stmt : fields_and_fns) {
      if (std::holds_alternative<std::shared_ptr<Stmt>>(stmt)) {
        auto func = std::dynamic_pointer_cast<Func>(
            std::get<std::shared_ptr<Stmt>>(stmt));
        fns.push_back(std::make_shared<Method>(identifier, func.get()));
      } else if (std::holds_alternative<
                     std::pair<Token, std::shared_ptr<ParseType::Type>>>(
                     stmt)) {
        fields.push_back(
            std::get<std::pair<Token, std::shared_ptr<ParseType::Type>>>(stmt));
      }
    }
  }

public:
  Token identifier;
  std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>> fields;
  std::vector<std::shared_ptr<Method>> fns;
  std::vector<Token> impls;

  StructDecl(
      Token identifier,
      std::vector<
          std::variant<std::shared_ptr<Stmt>,
                       std::pair<Token, std::shared_ptr<ParseType::Type>>>>
          fields_and_fns,
      std::vector<Token> impls)
      : identifier(identifier), impls(impls) {
    this->initialize_methods_and_fields(fields_and_fns);
  }

  void accept(Visitor *visitor) { visitor->visit_struct_decl(this); }
};