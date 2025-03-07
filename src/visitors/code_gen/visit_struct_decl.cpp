#include "../../../include/visitors/code_gen.h"
#include "bird_type.h"
#include <algorithm>
#include <binaryen-c.h>

void CodeGen::visit_struct_decl(StructDecl *struct_decl) {
  using bird_pair = std::pair<std::string, std::shared_ptr<BirdType>>;

  std::vector<bird_pair> struct_fields;
  std::transform(
      struct_decl->fields.begin(), struct_decl->fields.end(),
      std::back_inserter(struct_fields),
      [&](std::pair<Token, std::shared_ptr<ParseType::Type>> &field) {
        return std::make_pair(field.first.lexeme,
                              this->type_converter.convert(field.second));
      });

  auto count =
      std::count_if(struct_fields.begin(), struct_fields.end(),
                    [&](auto el) { return type_is_on_heap(el.second->type); });

  std::sort(struct_fields.begin(), struct_fields.end(),
            [&](const bird_pair first, const bird_pair second) {
              return type_is_on_heap(first.second->type) >
                     type_is_on_heap(second.second->type);
            });

  this->struct_name_to_num_pointers[struct_decl->identifier.lexeme] = count;
  type_table.declare(struct_decl->identifier.lexeme,
                     std::make_shared<StructType>(
                         struct_decl->identifier.lexeme, struct_fields));

  for (auto &method : struct_decl->fns) {
    method->accept(this);
  }
}