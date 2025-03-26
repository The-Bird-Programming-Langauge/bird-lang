#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_call(Call *call) {
  std::cout << "visit_call cg: " << call->call_token.lexeme << std::endl;
  std::cout << 0 << std::endl;
  call->callable->accept(this);
  std::cout << 1 << std::endl;
  auto function = stack.pop();
  std::cout << 2 << std::endl;

  std::vector<BinaryenExpressionRef> args;

  for (auto &arg : call->args) {
    arg->accept(this);
    args.push_back(this->stack.pop().value);
  }
  std::cout << 3 << std::endl;

  if (function.type->get_tag() == TypeTag::LAMBDA) {
    auto bird_function = std::dynamic_pointer_cast<BirdFunction>(function.type);

    std::vector<BinaryenType> params;
    for (auto param : bird_function->params) {
      params.push_back(bird_type_to_binaryen_type(param));
    }

    auto call = BinaryenCallIndirect(
        this->mod, "lambdas", function.value, args.data(), args.size(),
        BinaryenTypeCreate(params.data(), params.size()),
        bird_type_to_binaryen_type(bird_function->ret));

    std::cout << 4 << std::endl;
    this->stack.push(TaggedExpression(call, bird_function->ret));
    return;
  }

  std::cout << "done vc codegen" << std::endl;
  this->stack.push(this->create_call_with(
      BinaryenFunctionGetName((BinaryenFunctionRef)function.value), args));
}

TaggedExpression
CodeGen::create_call_with(std::string func_name,
                          std::vector<BinaryenExpressionRef> args) {
  std::cout << "create_call_with name: " << func_name << std::endl;
  auto return_type = this->function_return_types[func_name];

  this->must_garbage_collect = true;
  return TaggedExpression(BinaryenCall(this->mod, func_name.c_str(),
                                       args.data(), args.size(),
                                       return_type.value),
                          return_type.type);
}
