#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_direct_member_access(
    DirectMemberAccess *direct_member_access) {
  direct_member_access->accessable->accept(this);
  auto accessable = this->stack.pop();

  std::shared_ptr<StructType> struct_type;
  if (accessable.type->type == BirdTypeType::PLACEHOLDER) {
    std::shared_ptr<PlaceholderType> placeholder =
        safe_dynamic_pointer_cast<PlaceholderType>(accessable.type);
    if (this->struct_names.find(placeholder->name) ==
        this->struct_names.end()) {
      throw BirdException("struct not found");
    }

    struct_type = safe_dynamic_pointer_cast<StructType>(
        this->type_table.get(placeholder->name));
  } else {
    struct_type = safe_dynamic_pointer_cast<StructType>(accessable.type);
  }

  auto offset = 0;
  std::shared_ptr<BirdType> member_type;
  for (auto &field : struct_type->fields) {

    if (field.first == direct_member_access->identifier.lexeme) {
      member_type = field.second;
      break;
    }

    offset += bird_type_byte_size(field.second);
  }

  BinaryenExpressionRef args[2] = {
      accessable.value, BinaryenConst(this->mod, BinaryenLiteralInt32(offset))};

  auto func_name =
      member_type->type == BirdTypeType::FLOAT ? "mem_get_64" : "mem_get_32";

  this->stack.push(
      TaggedExpression(BinaryenCall(this->mod, func_name, args, 2,
                                    bird_type_to_binaryen_type(member_type)),
                       member_type));
}
