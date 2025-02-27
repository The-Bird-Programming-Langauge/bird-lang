#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::visit_func(Func *func) {
  auto func_name = func->identifier.lexeme;

  if (func->return_type.has_value()) {
    auto bird_return_type =
        this->type_converter.convert(func->return_type.value());
    auto binaryen_return_type = bird_type_to_binaryen_type(bird_return_type);

    this->function_return_types[func_name] =
        TaggedType(binaryen_return_type, bird_return_type);
  } else {
    this->function_return_types[func_name] = TaggedType(
        BinaryenTypeNone(), std::shared_ptr<BirdType>(new VoidType()));
  }

  auto old_function_name = this->current_function_name;

  this->current_function_name = func_name;
  auto current_function_body = std::vector<BinaryenExpressionRef>();
  this->function_locals[func_name] = std::vector<BinaryenType>();

  std::vector<BinaryenType> param_types;

  for (auto &param : func->param_list) {
    auto param_type = this->type_converter.convert(param.second);
    param_types.push_back(bird_type_to_binaryen_type(param_type));
    this->function_locals[func_name].push_back(
        bird_type_to_binaryen_type(param_type));
  }

  BinaryenType params =
      BinaryenTypeCreate(param_types.data(), param_types.size());

  BinaryenType result_type =
      func->return_type.has_value()
          ? bird_type_to_binaryen_type(
                this->type_converter.convert(func->return_type.value()))
          : BinaryenTypeNone();

  this->environment.push_env();

  auto index = 0;
  for (auto &param : func->param_list) {
    this->environment.declare(
        param.first.lexeme,
        TaggedIndex(index++, this->type_converter.convert(param.second)));
  }

  for (auto &stmt : dynamic_cast<Block *>(func->block.get())->stmts) {
    stmt->accept(this);
    auto result = this->stack.pop();

    if (result.type->type != BirdTypeType::VOID) {
      current_function_body.push_back(BinaryenDrop(this->mod, result.value));
    } else {
      current_function_body.push_back(result.value);
    }
  }

  // perform garbage collection at the end of a function by popping the
  // javascript calls off the stack in a block and executing the block

  this->environment.pop_env();

  this->garbage_collect();
  auto calls_block = this->stack.pop();
  current_function_body.push_back(calls_block.value);

  BinaryenExpressionRef body =
      BinaryenBlock(this->mod, nullptr, current_function_body.data(),
                    current_function_body.size(), BinaryenTypeNone());

  std::vector<BinaryenType> vars = std::vector<BinaryenType>(
      this->function_locals[func_name].begin() + param_types.size(),
      this->function_locals[func_name].end());

  BinaryenAddFunction(this->mod, func_name.c_str(), params, result_type,
                      vars.data(), vars.size(), body);

  BinaryenAddFunctionExport(this->mod, func_name.c_str(), func_name.c_str());

  this->current_function_name = old_function_name;
  this->function_locals.erase(func_name);

  // no stack push here, automatically added
}
