#pragma once
#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

class PropDecl : public Stmt {
public:
  Token identifier;
  std::shared_ptr<ParseType::Type> type;

  PropDecl(Token identifier, std::shared_ptr<ParseType::Type> type)
      : identifier(identifier), type(std::move(type)) {}

  void accept(Visitor *visitor) { visitor->visit_prop_decl(this); }
};
