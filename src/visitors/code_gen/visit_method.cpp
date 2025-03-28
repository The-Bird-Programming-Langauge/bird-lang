#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_method(Method *method) {
  this->add_func_with_name(method, "0" + method->class_identifier.lexeme + "." +
                                       method->identifier.lexeme);
}
