#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_decl_stmt(DeclStmt *decl_stmt) {
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

  environment.declare(decl_stmt->identifier.lexeme, TaggedIndex(index, type));

  TaggedExpression set_local =
      this->binaryen_set(decl_stmt->identifier.lexeme, initializer_value.value);

  if (type_is_on_heap(initializer_value.type->get_tag())) {
    auto block = BinaryenBlock(this->mod, nullptr, &set_local.value, 1,
                               BinaryenTypeNone());
    auto get_ref = this->binaryen_get(decl_stmt->identifier.lexeme);
    auto register_root = BinaryenCall(this->mod, "register_root",
                                      &get_ref.value, 1, BinaryenTypeNone());
    BinaryenBlockInsertChildAt(block, 1, register_root);
    this->stack.push(TaggedExpression(block, type));
    return;
  }

  this->stack.push(TaggedExpression(set_local.value, type));
}