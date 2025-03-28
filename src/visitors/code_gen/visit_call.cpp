#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_call(Call *call) {
  std::cout << "visitng call" << std::endl;
  call->callable->accept(this);
  auto function = stack.pop();

  std::vector<BinaryenExpressionRef> args;

  for (auto &arg : call->args) {
    arg->accept(this);
    args.push_back(this->stack.pop().value);
  }

  if (function.type->get_tag() == TypeTag::LAMBDA) {
    // create linear memory thing
    auto bird_function = std::dynamic_pointer_cast<BirdFunction>(function.type);

    std::vector<BinaryenType> params;
    for (auto param : bird_function->params) {
      params.push_back(bird_type_to_binaryen_type(param));
    }

    // extract lambda function pointer
    std::vector<BinaryenExpressionRef> fn_ptr_operands = {
        function.value, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};
    auto fn_ptr = BinaryenCall(this->mod, "mem_get_32", fn_ptr_operands.data(),
                               fn_ptr_operands.size(), BinaryenTypeInt32());

    std::vector<BinaryenExpressionRef> env_operands = {
        function.value,
        BinaryenConst(this->mod, BinaryenLiteralInt32(bird_type_byte_size(
                                     std::make_shared<IntType>())))};
    auto env = BinaryenCall(this->mod, "mem_get_32", env_operands.data(),
                            env_operands.size(), BinaryenTypeInt32());

    args.insert(args.begin(), env);
    params.insert(params.begin(), BinaryenTypeInt32());

    // pass it in as an argument(?)
    auto call = BinaryenCallIndirect(
        this->mod, "lambdas", fn_ptr, args.data(), args.size(),
        BinaryenTypeCreate(params.data(), params.size()),
        bird_type_to_binaryen_type(bird_function->ret));

    this->stack.push(TaggedExpression(call, bird_function->ret));
    return;
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
