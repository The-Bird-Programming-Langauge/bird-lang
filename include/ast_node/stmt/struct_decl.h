#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../exceptions/bird_exception.h"
#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "func.h"
#include "prop_decl.h"
#include "stmt.h"

// forward declaration
class Expr;

/*
 * Type statement AST Node that represents type declarations
 * ex:
 * type x = int;
 */
class StructDecl : public Stmt {
public:
  Token identifier;
  std::vector<std::unique_ptr<PropDecl>> fields;
  std::vector<std::shared_ptr<Func>> fns;

  StructDecl(Token identifier,
             std::vector<std::unique_ptr<Stmt>> fields_and_fns)
      : identifier(identifier) {
    for (auto &stmt : fields_and_fns) {
      if (auto fn = dynamic_cast<Func *>(stmt.get())) {
        fns.push_back(std::make_shared<Func>(*fn));
      } else if (auto field = dynamic_cast<PropDecl *>(stmt.get())) {
        fields.push_back(std::make_unique<PropDecl>(*field));
      } else {
        throw new BirdException("invalid stmt passed to struct decl");
      }
    }
  }

  void accept(Visitor *visitor) { visitor->visit_struct_decl(this); }
};