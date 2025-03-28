#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::init_lambda_table() {
  // TODO: add a pass to figure out how many lambdas there are
  BinaryenAddTable(this->mod, "lambdas", 100, 100, BinaryenTypeFuncref());
  BinaryenAddTableExport(this->mod, "lambdas", "lambdas");
}