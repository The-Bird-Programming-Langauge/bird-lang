#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_namespace(NamespaceStmt *_namespace) {
  auto previous_mangler = this->name_mangler;
  this->name_mangler += (_namespace->identifier.lexeme + "::");
  this->type_converter.set_namespace(this->name_mangler);

  std::vector<BinaryenExpressionRef> children;
  for (auto &member : _namespace->members) {
    member->accept(this);
    if (!this->stack.empty()) {
      children.push_back(this->stack.pop().value);
    }
  }

  this->name_mangler = previous_mangler;
  this->type_converter.set_namespace(previous_mangler);

  this->stack.push(
      BinaryenBlock(this->mod, _namespace->identifier.lexeme.c_str(),
                    children.data(), children.size(), BinaryenTypeNone()));
}