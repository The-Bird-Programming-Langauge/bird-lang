#include "visitors/code_gen.h"

/*
 * Binaryen doesnt support integer negation for some reason,
 * only float32 and 64, so i guess the work around would be
 * to subtract it from zero.
 */
void CodeGen::visit_unary(Unary *unary)
{
    unary->expr->accept(this);
    auto expr = this->stack.pop();

    BinaryenType expr_type = BinaryenExpressionGetType(expr.value);

    switch (unary->op.token_type)
    {
    case Token::Type::QUESTION:
    {
        this->stack.push(
            TaggedExpression(
                BinaryenSelect(
                    this->mod,
                    expr.value,
                    BinaryenConst(mod, BinaryenLiteralInt32(1)),
                    BinaryenConst(mod, BinaryenLiteralInt32(0)),
                    BinaryenTypeInt32()),
                std::shared_ptr<BirdType>(new BoolType())));
        break;
    }
    case Token::Type::MINUS:
    {
        if (expr_type == BinaryenTypeFloat64())
        {
            this->stack.push(
                TaggedExpression(
                    BinaryenUnary(
                        mod,
                        BinaryenNegFloat64(),
                        expr.value),
                    std::shared_ptr<BirdType>(new BoolType())));
        }
        else if (expr_type == BinaryenTypeInt32())
        {
            BinaryenExpressionRef zero = BinaryenConst(mod, BinaryenLiteralInt32(0));

            this->stack.push(
                TaggedExpression(
                    BinaryenBinary(
                        mod,
                        BinaryenSubInt32(),
                        zero,
                        expr.value),
                    std::shared_ptr<BirdType>(new IntType())));
        }
        break;
    }
    case Token::Type::NOT:
    {
        this->stack.push(
            TaggedExpression(
                BinaryenSelect(
                    this->mod,
                    BinaryenUnary(this->mod, BinaryenEqZInt32(), expr.value),
                    BinaryenConst(mod, BinaryenLiteralInt32(1)),
                    BinaryenConst(mod, BinaryenLiteralInt32(0)),
                    BinaryenTypeInt32()),
                std::shared_ptr<BirdType>(new BoolType)));
        break;
    }
    default:
    {
        throw BirdException("undefined unary operator for code gen");
    }
    }
}