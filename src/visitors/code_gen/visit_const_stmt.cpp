#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_const_stmt(ConstStmt *const_stmt) {
  auto mangled_name = this->name_mangler + const_stmt->identifier.lexeme;
  // std::cout << "visiting const... " << const_stmt->identifier.lexeme
  //           << " mangled name: " << mangled_name << std::endl;
  const_stmt->value->accept(this);
  TaggedExpression initializer = this->stack.pop();

  std::shared_ptr<BirdType> type;
  if (const_stmt->type.has_value()) {
    type = this->type_converter.convert(const_stmt->type.value());
  } else {
    if (initializer.type->get_tag() != TypeTag::VOID) {
      type = initializer.type;
    } else {
      BinaryenType binaryen_type = BinaryenExpressionGetType(initializer.value);
      type = (binaryen_type == BinaryenTypeFloat64())
                 ? std::shared_ptr<BirdType>(new FloatType())
                 : std::shared_ptr<BirdType>(new IntType());
    }
  }

  BinaryenIndex index =
      this->function_locals[this->current_function_name].size();
  this->function_locals[this->current_function_name].push_back(
      bird_type_to_binaryen_type(type));

  // std::cout << "declaring const: " << mangled_name
  //           << " in: " << this->current_function_name << " at index "
  //           << (this->function_locals[this->current_function_name].size())
  //           << std::endl;

  environment.declare(mangled_name, TaggedIndex(index, type));

  TaggedExpression set_local =
      this->binaryen_set(mangled_name, initializer.value);

  // std::cout << "const final setting: " << mangled_name << " -> ";
  // BinaryenExpressionPrint(set_local.value);
  // std::cout << std::endl;

  this->stack.push(TaggedExpression(set_local.value, type));
}
