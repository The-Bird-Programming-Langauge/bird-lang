#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_decl_stmt(DeclStmt *decl_stmt) {
  if (decl_stmt->get_captured()) {
    std::cout << "generating captured code" << std::endl;
    // this->stack.push(BinaryenConst(this->mod, BinaryenLiteralInt32(0)));

    auto env = BinaryenLocalGet(
        this->mod, this->function_env_location[this->current_function_name],
        BinaryenTypeInt32());

    decl_stmt->value->accept(this);
    std::vector<BinaryenExpressionRef> operands = {
        env, BinaryenConst(this->mod, BinaryenLiteralInt32(0)),
        stack.pop().value};

    const auto call = BinaryenCall(this->mod, "mem_set_32", operands.data(),
                                   operands.size(), BinaryenTypeNone());
    BinaryenExpressionPrint(call);

    this->stack.push(call);

    return;
  }
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

  this->stack.push(TaggedExpression(set_local.value, type));
}