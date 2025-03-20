#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_decl_stmt(DeclStmt *decl_stmt) {
  auto mangled_name = this->name_mangler + decl_stmt->identifier.lexeme;
  // std::cout << "visiting... " << decl_stmt->identifier.lexeme
  //           << " mangled name: " << mangled_name << std::endl;
  decl_stmt->value->accept(this);
  TaggedExpression initializer_value = this->stack.pop();

  std::shared_ptr<BirdType> type;
  if (decl_stmt->type.has_value()) // not inferred
  {
    type = this->type_converter.convert(decl_stmt->type.value());
  } else {
    if (initializer_value.type->get_tag() != TypeTag::VOID) {
      type = initializer_value.type;
    } else {
      BinaryenType binaryen_type =
          BinaryenExpressionGetType(initializer_value.value);
      type = (binaryen_type == BinaryenTypeFloat64())
                 ? std::shared_ptr<BirdType>(new FloatType())
                 : std::shared_ptr<BirdType>(new IntType());
    }
  }

  BinaryenIndex index =
      this->function_locals[this->current_function_name].size();
  this->function_locals[this->current_function_name].push_back(
      bird_type_to_binaryen_type(type));

  // std::cout << "declaring var: " << mangled_name
  //           << " in: " << this->current_function_name << " at index "
  //           << (this->function_locals[this->current_function_name].size())
  //           << std::endl;

  environment.declare(mangled_name, TaggedIndex(index, type));

  TaggedExpression set_local =
      this->binaryen_set(mangled_name, initializer_value.value);

  // std::cout << "var final setting: " << mangled_name << " -> ";
  // BinaryenExpressionPrint(set_local.value);
  // std::cout << std::endl;

  this->stack.push(TaggedExpression(set_local.value, type));
}