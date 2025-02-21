#include "visitors/code_gen.h"

void CodeGen::visit_while_stmt(WhileStmt *while_stmt)
{
    std::vector<BinaryenExpressionRef> children;

    while_stmt->stmt->accept(this);
    TaggedExpression body = this->stack.pop();

    children.push_back(body.value);

    while_stmt->condition->accept(this);
    TaggedExpression condition = this->stack.pop();

    auto outer_body =
        BinaryenBlock(
            this->mod,
            "BODY",
            children.data(),
            children.size(),
            BinaryenTypeNone());

    std::vector<BinaryenExpressionRef> while_body_children;
    while_body_children.push_back(outer_body);
    while_body_children.push_back(BinaryenBreak(
        this->mod,
        "LOOP",
        condition.value,
        nullptr));

    auto while_body =
        BinaryenBlock(
            this->mod,
            "while_block",
            while_body_children.data(),
            while_body_children.size(),
            BinaryenTypeNone());

    auto if_cond =
        BinaryenIf(
            this->mod,
            condition.value,
            BinaryenLoop(
                this->mod,
                "LOOP",
                while_body),
            nullptr);

    this->stack.push(
        BinaryenBlock(
            this->mod,
            "EXIT",
            &if_cond,
            1,
            BinaryenTypeNone()));
}
