#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_member_assign(MemberAssign *member_assign) {
  member_assign->value->accept(this);
  auto value = this->stack.pop();

  member_assign->accessable->accept(this);
  auto accessable = this->stack.pop();

  std::shared_ptr<StructType> struct_type =
      safe_dynamic_pointer_cast<StructType>(accessable.type);

  auto offset = 0;
  for (auto &field : struct_type->fields) {
    if (field.first == member_assign->identifier.lexeme) {
      break;
    }

    offset += bird_type_byte_size(field.second);
  }

  BinaryenExpressionRef args[3] = {
      accessable.value, BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
      value.value};

  this->stack.push(TaggedExpression(
      BinaryenCall(this->mod, get_mem_set_for_type(value.type->type), args, 3,
                   BinaryenTypeNone()),
      std::shared_ptr<BirdType>(new VoidType())));
}
