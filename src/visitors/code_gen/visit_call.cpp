#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

TaggedExpression
CodeGen::convert_string_to_array(BinaryenExpressionRef str_ptr) {
  auto arr_ptr =
      BinaryenCall(mod, "str_to_array", &str_ptr, 1, BinaryenTypeInt32());

  auto array_type = std::make_shared<ArrayType>(std::make_shared<CharType>());
  return TaggedExpression(arr_ptr, array_type);
}

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

  if (func_name == "iter") {
    auto arg_type = arg_types[0];

    if (arg_type->get_tag() == TypeTag::STRING) {
      auto arr = convert_string_to_array(args[0]);
      auto iter_type = std::make_shared<IteratorType>(arr.type);
      auto call = BinaryenCall(mod, "iter", &arr.value, 1, BinaryenTypeInt32());
      this->stack.push(TaggedExpression(call, iter_type));
      return;
    }

    if (arg_type->get_tag() == TypeTag::ARRAY) {
      auto iter_type = std::make_shared<IteratorType>(arg_type);
      auto call =
          BinaryenCall(mod, "iter", args.data(), 1, BinaryenTypeInt32());
      this->stack.push(TaggedExpression(call, iter_type));
      return;
    }
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

  return TaggedExpression(BinaryenCall(this->mod, func_name.c_str(),
                                       args.data(), args.size(),
                                       return_type.value),
                          return_type.type);
}