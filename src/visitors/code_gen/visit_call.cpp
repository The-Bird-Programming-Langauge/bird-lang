#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_call(Call *call) {
  // auto func_name = call->identifier.lexeme;

  // std::vector<BinaryenExpressionRef> args;

  // for (auto &arg : call->args) {
  //   arg->accept(this);
  //   args.push_back(this->stack.pop().value);
  // }

  // this->stack.push(this->create_call_with(func_name, args));
}

TaggedExpression
CodeGen::create_call_with(std::string func_name,
                          std::vector<BinaryenExpressionRef> args) {
  auto return_type = this->function_return_types[func_name];

  // auto children = std::vector<BinaryenExpressionRef>();

  this->must_garbage_collect = true;
  return TaggedExpression(BinaryenCall(this->mod, func_name.c_str(),
                                       args.data(), args.size(),
                                       return_type.value),
                          return_type.type);
}
