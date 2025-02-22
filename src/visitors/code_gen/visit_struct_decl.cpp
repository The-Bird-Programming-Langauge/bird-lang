#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_struct_decl(StructDecl *struct_decl) {
  std::vector<std::pair<std::string, std::shared_ptr<BirdType>>> struct_fields;
  std::transform(
      struct_decl->fields.begin(), struct_decl->fields.end(),
      std::back_inserter(struct_fields),
      [&](std::pair<std::string, std::shared_ptr<ParseType::Type>> field) {
        return std::make_pair(field.first,
                              this->type_converter.convert(field.second));
      });

  type_table.declare(struct_decl->identifier.lexeme,
                     std::make_shared<StructType>(
                         struct_decl->identifier.lexeme, struct_fields));
}