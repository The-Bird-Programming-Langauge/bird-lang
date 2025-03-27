#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {
  // auto prev = this->name_mangler;
  // this->name_mangler += scope_resolution->_namespace.lexeme + "::";
  scope_resolution->identifier->accept(this);
  // this->name_mangler = prev;
}