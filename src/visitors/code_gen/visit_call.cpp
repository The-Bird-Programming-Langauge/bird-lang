#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_call(Call *call) {
  auto func_name = call->identifier.lexeme;
  std::vector<BinaryenExpressionRef> args;
  std::vector<std::shared_ptr<BirdType>> arg_types;

  for (auto &arg : call->args) {
    arg->accept(this);
    auto arg_expr = this->stack.pop();
    args.push_back(arg_expr.value);
    arg_types.push_back(arg_expr.type);
  }

  // TODO: figure out how to clean this up
  if (func_name == "push") {
    auto el_type = arg_types[1]->get_tag();
    if (el_type == TypeTag::FLOAT) {
      func_name = "push_64";
    } else if (type_is_on_heap(el_type)) {
      func_name = "push_ptr";
    } else {
      func_name = "push_32";
    }
  }

  this->stack.push(this->create_call_with(func_name, args));
}

TaggedExpression
CodeGen::create_call_with(std::string func_name,
                          std::vector<BinaryenExpressionRef> args) {
  auto return_type = this->function_return_types[func_name];

  auto children = std::vector<BinaryenExpressionRef>();

  this->must_garbage_collect = true;
  return TaggedExpression(BinaryenCall(this->mod, func_name.c_str(),
                                       args.data(), args.size(),
                                       return_type.value),
                          return_type.type);
}