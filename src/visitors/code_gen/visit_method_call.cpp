#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_method_call(MethodCall *method_call) {
  method_call->accessable->accept(this);
  auto struct_expr = stack.pop();
  auto struct_type = std::dynamic_pointer_cast<StructType>(struct_expr.type);
  std::vector<BinaryenExpressionRef> args = {struct_expr.value};

  for (auto arg : method_call->args) {
    arg->accept(this);
    args.push_back(stack.pop().value);
  }

  this->stack.push(create_call_with(
      "0" + struct_type->name + "." + method_call->identifier.lexeme, args));
}