#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_import_stmt(ImportStmt *import_stmt) {
  this->stack.push(BinaryenBlock(this->mod, "EXIT", nullptr, 0, BinaryenTypeNone())); // temp
}