#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <string>

// every lambda function should have its own env passed in as the first
// parameter we need more type information for the first parameter, we can
// probably just call it a struct

// the lambda is the environment in a struct,
// the pointer to the function

// std::shared_ptr<BirdType> create_lambda(std::shared_ptr<BirdType> lambda) {
//   // this representation is only used in codegen
//   return std::make_shared<StructType>(
//       "lambda",
//       std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>{
//           {"function", lambda},
//           {"environment",
// }

// visit lambda registers the lambda function and allows the program to pass the
// environment to the value this should create the environment struct as well
void CodeGen::visit_lambda(Lambda *lambda) {
  auto fn_ptr =
      BinaryenConst(this->mod, BinaryenLiteralInt32(this->lambda_count));

  auto func_name = std::to_string(this->lambda_count++) + "lambda";
  this->add_lambda(func_name, lambda);
  const char *function_name_c_str = func_name.c_str();
  BinaryenAddActiveElementSegment(this->mod, "lambdas", function_name_c_str,
                                  &function_name_c_str, 1, fn_ptr);

  auto ret_type = lambda->return_type.has_value()
                      ? type_converter.convert(lambda->return_type.value())
                      : std::make_shared<VoidType>();

  std::vector<std::shared_ptr<BirdType>> params{};
  std::transform(lambda->param_list.begin(), lambda->param_list.end(),
                 std::back_inserter(params),
                 [&](std::pair<Token, std::shared_ptr<ParseType::Type>> param) {
                   return type_converter.convert(param.second);
                 });

  // store the function pointer and a blank environment pointer
  auto &locals = this->function_locals[this->current_function_name];
  locals.push_back(BinaryenTypeInt32());

  auto identifier = std::to_string(locals.size()) + "temp";
  // type should never be used
  this->environment.declare(
      identifier, TaggedIndex(locals.size(), std::make_shared<VoidType>()));

  std::vector<BinaryenExpressionRef> operands = {
      fn_ptr,
      BinaryenLocalGet(this->mod,
                       this->function_env_location[this->current_function_name],
                       BinaryenTypeInt32())};

  auto lambda_ptr = BinaryenCall(
      this->mod, this->struct_constructors["0lambdaConstructor"].c_str(),
      operands.data(), operands.size(), BinaryenTypeInt32());

  this->stack.push(TaggedExpression(
      lambda_ptr, std::make_shared<LambdaFunction>(params, ret_type)));
}

void CodeGen::add_lambda(std::string name, Lambda *lambda) {
  std::cout << "adding lambda" << std::endl;
  auto tagged_type = this->register_function_return(name, lambda->return_type);
  auto result_type = tagged_type.value;

  auto old_function_name = this->current_function_name;

  this->current_function_name = name;
  auto current_function_body = std::vector<BinaryenExpressionRef>();
  this->function_locals[name] = std::vector<BinaryenType>();

  std::vector<BinaryenType> param_types;
  std::cout << param_types.size() << std::endl;
  auto int_type = BinaryenTypeInt32();
  param_types.push_back(int_type);
  this->function_locals[name].push_back(int_type);

  std::cout << param_types.size() << std::endl;

  for (auto &param : lambda->param_list) {
    auto param_type = this->type_converter.convert(param.second);
    param_types.push_back(bird_type_to_binaryen_type(param_type));
    this->function_locals[name].push_back(
        bird_type_to_binaryen_type(param_type));
  }

  std::cout << "param types size: " << param_types.size() << std::endl;

  BinaryenType params =
      BinaryenTypeCreate(param_types.data(), param_types.size());

  std::cout << "creating env" << std::endl;
  this->environment.push_env();

  auto index = 0;
  for (auto &param : lambda->param_list) {
    this->environment.declare(
        param.first.lexeme,
        TaggedIndex(index++, this->type_converter.convert(param.second)));
  }

  if (function_capture_size[name] > 0) {
    auto environment_code =
        this->create_environment(function_capture_size[name]);
    current_function_body.push_back(environment_code);
  }

  for (auto &stmt : dynamic_cast<Block *>(lambda->block.get())->stmts) {
    stmt->accept(this);
    auto result = this->stack.pop();

    if (result.type->get_tag() != TypeTag::VOID) {
      current_function_body.push_back(BinaryenDrop(this->mod, result.value));
    } else {
      current_function_body.push_back(result.value);
    }

    if (this->must_garbage_collect) {
      this->garbage_collect();
      current_function_body.push_back(this->stack.pop().value);
      this->must_garbage_collect = false;
    }
  }

  this->environment.pop_env();
  BinaryenExpressionRef body =
      BinaryenBlock(this->mod, nullptr, current_function_body.data(),
                    current_function_body.size(), BinaryenTypeNone());

  std::vector<BinaryenType> vars = std::vector<BinaryenType>(
      this->function_locals[name].begin() + param_types.size(),
      this->function_locals[name].end());

  BinaryenAddFunction(this->mod, name.c_str(), params, result_type, vars.data(),
                      vars.size(), body);

  BinaryenAddFunctionExport(this->mod, name.c_str(), name.c_str());

  this->current_function_name = old_function_name;
}