#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_call(Call *call) {
  call->callable->accept(this);
  auto function = stack.pop();

  std::vector<BinaryenExpressionRef> args;

  for (auto &arg : call->args) {
    arg->accept(this);
    args.push_back(this->stack.pop().value);
  }

  this->stack.push(this->create_call_with(
      BinaryenFunctionGetName((BinaryenFunctionRef)function.value), args));
}

TaggedExpression
CodeGen::create_call_with(std::string func_name,
                          std::vector<BinaryenExpressionRef> args) {
  auto return_type = this->function_return_types[func_name];

  this->must_garbage_collect = true;
  return TaggedExpression(BinaryenCall(this->mod, func_name.c_str(),
                                       args.data(), args.size(),
                                       return_type.value),
                          return_type.type);
}
