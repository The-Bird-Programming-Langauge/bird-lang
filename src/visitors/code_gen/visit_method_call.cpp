#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_method_call(MethodCall *method_call) {
  method_call->instance->accept(this);
  std::vector<BinaryenExpressionRef> args = {stack.pop().value};

  for (auto arg : method_call->args) {
    arg->accept(this);
    args.push_back(stack.pop().value);
  }

  this->stack.push(create_call_with(method_call->identifier.lexeme, args));
}