#include "../../../include/visitors/code_gen.h"
#include "exceptions/bird_exception.h"
#include <memory>

void CodeGen::visit_member_assign(MemberAssign *member_assign) {
  member_assign->value->accept(this);
  auto value = this->stack.pop();

  member_assign->accessable->accept(this);
  auto accessable = this->stack.pop();

  std::shared_ptr<StructType> struct_type =
      safe_dynamic_pointer_cast<StructType>(accessable.type);

  std::shared_ptr<BirdType> original_value_type;

  auto offset = 0;
  for (auto &field : struct_type->fields) {
    if (field.first == member_assign->identifier.lexeme) {
      original_value_type = field.second;
      break;
    }

    offset += bird_type_byte_size(field.second);
  }

  if (!original_value_type) {
    throw BirdException("field not found in member assign");
  }

  BinaryenExpressionRef get_args[2] = {
      accessable.value, BinaryenConst(this->mod, BinaryenLiteralInt32(offset))};

  auto original_value = BinaryenCall(
      this->mod, get_mem_get_for_type(original_value_type->get_tag()), get_args,
      2, bird_type_to_binaryen_type(original_value_type));

  if (member_assign->assign_operator.token_type != Token::EQUAL) {
    auto binary_op = this->binary_operations.at(assign_expr_binary_equivalent(
        member_assign->assign_operator.token_type));
    auto binary_op_fn =
        binary_op.at({original_value_type->get_tag(), value.type->get_tag()});

    value = TaggedExpression(BinaryenBinary(this->mod, binary_op_fn.value(),
                                            original_value, value.value),
                             binary_op_fn.type);
  }

  BinaryenExpressionRef args[3] = {
      accessable.value, BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
      value.value};

  this->stack.push(TaggedExpression(
      BinaryenCall(this->mod, get_mem_set_for_type(value.type->get_tag()), args,
                   3, BinaryenTypeNone()),
      std::shared_ptr<BirdType>(new VoidType())));
}
