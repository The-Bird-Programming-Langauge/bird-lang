#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_import_stmt(ImportStmt *import_stmt)
{
  // add import item to global namespace

  // figure out how to import codegen import items
  
  this->stack.push(BinaryenBlock(this->mod, "EXIT", nullptr, 0, BinaryenTypeNone())); // temp
};