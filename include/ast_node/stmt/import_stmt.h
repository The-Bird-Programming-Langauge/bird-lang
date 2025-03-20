#pragma once

#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"
#include <memory>
#include <optional>
#include <vector>

/*
 * import statement AST Node
 *
 * import Math
 * import Math::sin, Math::cos, NumPy
 * import sin, cos from Math
 */
class ImportStmt : public Stmt {
public:
  Token import_token;
  std::vector<std::vector<Token>> import_paths;

  ImportStmt(
    Token import_token,
    std::vector<std::vector<Token>> import_paths)
    : import_token(import_token),
      import_paths(import_paths) {}

  void accept(Visitor *visitor) { visitor->visit_import_stmt(this); }
};
