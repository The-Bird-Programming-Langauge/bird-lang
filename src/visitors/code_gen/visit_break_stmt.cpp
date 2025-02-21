#include "visitors/code_gen.h"

void CodeGen::visit_break_stmt(BreakStmt *break_stmt)
{
    this->stack.push(
        BinaryenBreak(
            this->mod,
            "EXIT",
            nullptr,
            nullptr));
}