#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::visit_func(Func *func) {
  auto func_name = func->identifier.lexeme;
  this->add_func_with_name(func, func_name);
}

void CodeGen::add_func(
    std::string func_name,
    std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>> param_list,
    std::shared_ptr<Stmt> block,
    std::optional<std::shared_ptr<ParseType::Type>> return_type) {
  std::cout << "add_func name: " << func_name << std::endl;
  if (return_type.has_value()) {
    auto bird_return_type = this->type_converter.convert(return_type.value());
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

  for (auto &param : param_list) {
    auto param_type = this->type_converter.convert(param.second);
    param_types.push_back(bird_type_to_binaryen_type(param_type));
    this->function_locals[func_name].push_back(
        bird_type_to_binaryen_type(param_type));
  }

  BinaryenType params =
      BinaryenTypeCreate(param_types.data(), param_types.size());

  BinaryenType result_type =
      return_type.has_value()
          ? bird_type_to_binaryen_type(
                this->type_converter.convert(return_type.value()))
          : BinaryenTypeNone();

  this->environment.push_env();

  auto index = 0;
  for (auto &param : param_list) {
    this->environment.declare(
        param.first.lexeme,
        TaggedIndex(index++, this->type_converter.convert(param.second)));
  }

  // to enable recursion
  BinaryenAddFunction(this->mod, func_name.c_str(), BinaryenTypeNone(),
                      BinaryenTypeNone(), nullptr, 0, nullptr);

  for (auto &stmt : dynamic_cast<Block *>(block.get())->stmts) {
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
      this->function_locals[func_name].begin() + param_types.size(),
      this->function_locals[func_name].end());

  // to enable recursion, look above at previous add function
  BinaryenRemoveFunction(this->mod, func_name.c_str());
  BinaryenAddFunction(this->mod, func_name.c_str(), params, result_type,
                      vars.data(), vars.size(), body);

  BinaryenAddFunctionExport(this->mod, func_name.c_str(), func_name.c_str());

  this->current_function_name = old_function_name;

  // no stack push here, automatically added
}

void CodeGen::add_func_with_name(Func *func, std::string func_name) {
  add_func(func_name, func->param_list, func->block, func->return_type);
}