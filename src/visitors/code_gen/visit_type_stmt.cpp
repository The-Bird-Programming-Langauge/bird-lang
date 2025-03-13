#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_type_stmt(TypeStmt *type_stmt) {
  this->type_table.declare(
      type_stmt->identifier.lexeme,
      this->type_converter.convert(this->type_table, type_stmt->type_token));
}