#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::visit_namespace(NamespaceStmt *_namespace) {
  std::vector<BinaryenExpressionRef> children;
  for (auto &member : _namespace->members) {
    member->accept(this);
    if (!this->stack.empty()) {
      children.push_back(this->stack.pop().value);
    }
  }

  this->stack.push(
      BinaryenBlock(this->mod, _namespace->identifier.lexeme.c_str(),
                    children.data(), children.size(), BinaryenTypeNone()));
}