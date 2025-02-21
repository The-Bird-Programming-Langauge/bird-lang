#include "visitors/code_gen.h"

void CodeGen::visit_continue_stmt(ContinueStmt *continue_stmt)
{
    this->stack.push(
        BinaryenBreak(
            this->mod,
            "BODY",
            nullptr,
            nullptr));
}