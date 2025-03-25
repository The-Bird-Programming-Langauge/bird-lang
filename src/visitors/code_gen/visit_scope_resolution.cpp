#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {
  auto previous_mangler = this->name_mangler;
  this->name_mangler += scope_resolution->_namespace.lexeme + "::";
  this->type_converter.set_namespace(this->name_mangler);

  scope_resolution->identifier->accept(this);

  this->name_mangler = previous_mangler;
  this->type_converter.set_namespace(previous_mangler);
}