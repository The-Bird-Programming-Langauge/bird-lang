#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

/*
 * Block statement AST Node that represents blocks and new scopes
 * ex:
 * var x = 3;
 * {
 *     var x = 4;
 * }
 *
 */
class Block : public Stmt {
public:
  std::vector<std::unique_ptr<Stmt>> stmts;

  Block(std::vector<std::unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}

  void accept(Visitor *visitor) { visitor->visit_block(this); }
};
