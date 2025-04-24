#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_const_stmt(ConstStmt *const_stmt) {
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

  environment.declare(const_stmt->identifier.lexeme, TaggedIndex(index, type));

  TaggedExpression set_local =
      this->binaryen_set(const_stmt->identifier.lexeme, initializer.value);
  if (type_is_on_heap(initializer.type->get_tag())) {
    auto block = BinaryenBlock(this->mod, nullptr, &set_local.value, 1,
                               BinaryenTypeNone());
    auto get_ref = this->binaryen_get(const_stmt->identifier.lexeme);
    auto register_root = BinaryenCall(this->mod, "register_root",
                                      &get_ref.value, 1, BinaryenTypeNone());
    BinaryenBlockInsertChildAt(block, 1, register_root);
    this->stack.push(TaggedExpression(block, type));
    return;
  }

  this->stack.push(TaggedExpression(set_local.value, type));
}
