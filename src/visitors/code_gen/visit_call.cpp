#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_call(Call *call) {
  // auto func_name = call->identifier.lexeme;

  // std::vector<BinaryenExpressionRef> args;

  // for (auto &arg : call->args) {
  //   arg->accept(this);
  //   args.push_back(this->stack.pop().value);
  // }

  // auto return_type = this->function_return_types[func_name];

  // auto children = std::vector<BinaryenExpressionRef>();

  // this->stack.push(
  //     TaggedExpression(BinaryenCall(this->mod, func_name.c_str(),
  //     args.data(),
  //                                   args.size(), return_type.value),
  //                      return_type.type));

  // this->must_garbage_collect = true;
}
