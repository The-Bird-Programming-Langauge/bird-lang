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

  auto func_name = value.type->type == BirdTypeType::FLOAT ? "mem_set_64"
                   : value.type->type == BirdTypeType::STRUCT ||
                           value.type->type == BirdTypeType::PLACEHOLDER
                       ? "mem_set_ptr"
                       : "mem_set_32";

  this->stack.push(TaggedExpression(
      BinaryenCall(this->mod, func_name, args, 3, BinaryenTypeNone()),
      std::shared_ptr<BirdType>(new VoidType())));
}
