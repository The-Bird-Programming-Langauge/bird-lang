#include "../../../include/visitors/code_gen.h"
#include <algorithm>

void CodeGen::visit_struct_decl(StructDecl *struct_decl) {
  using bird_pair = std::pair<std::string, std::shared_ptr<BirdType>>;
  using parse_type_pair =
      std::pair<std::string, std::shared_ptr<ParseType::Type>>;

  std::vector<bird_pair> struct_fields;
  std::transform(struct_decl->fields.begin(), struct_decl->fields.end(),
                 std::back_inserter(struct_fields), [&](parse_type_pair field) {
                   return std::make_pair(
                       field.first, this->type_converter.convert(field.second));
                 });

  auto count = 0;
  std::sort(struct_fields.begin(), struct_fields.end(),
            [&](const bird_pair first, const bird_pair second) {
              if (type_is_on_heap(first.second->get_tag())) {
                count += 1;
                return true;
              }
              return false;
            });

  this->struct_name_to_num_pointers[struct_decl->identifier.lexeme] = count;
  type_table.declare(struct_decl->identifier.lexeme,
                     std::make_shared<StructType>(
                         struct_decl->identifier.lexeme, struct_fields));
}