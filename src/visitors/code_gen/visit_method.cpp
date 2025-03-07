#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_method(Method *method) { this->visit_func(method); }
