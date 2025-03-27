#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {
  scope_resolution->identifier->accept(this);
}