#include "visitors/code_gen.h"

void CodeGen::visit_expr_stmt(ExprStmt *expr_stmt)
{
    expr_stmt->expr->accept(this);
    // pop and push it back to the stack
}