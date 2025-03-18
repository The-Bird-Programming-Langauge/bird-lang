#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <string>

void CodeGen::visit_lambda(Lambda *lambda) {
  auto size =
      BinaryenConst(this->mod, BinaryenLiteralInt32(this->lambda_count));

  auto function_name = std::to_string(this->lambda_count++) + "lambda";
  this->add_func(function_name, lambda->param_list, lambda->block,
                 lambda->return_type);

  const char *function_name_c_str = function_name.c_str();
  BinaryenAddActiveElementSegment(this->mod, "lambdas", function_name.c_str(),
                                  &function_name_c_str, 1, size);

  auto ret_type = lambda->return_type.has_value()
                      ? type_converter.convert(lambda->return_type.value())
                      : std::make_shared<VoidType>();

  std::vector<std::shared_ptr<BirdType>> params{};
  std::transform(lambda->param_list.begin(), lambda->param_list.end(),
                 std::back_inserter(params),
                 [&](std::pair<Token, std::shared_ptr<ParseType::Type>> param) {
                   return type_converter.convert(param.second);
                 });

  this->stack.push(TaggedExpression(
      size, std::make_shared<LambdaFunction>(params, ret_type)));
}