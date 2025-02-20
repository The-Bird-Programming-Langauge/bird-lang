#include "visitors/code_gen.h"

void CodeGen::visit_array_init(ArrayInit *array_init)
{
    std::vector<BinaryenExpressionRef> children;
    std::vector<BinaryenExpressionRef> vals;

    unsigned int size = 0;
    std::shared_ptr<BirdType> type;
    for (auto &element : array_init->elements)
    {
        element->accept(this);
        auto val = this->stack.pop();
        type = val.type;

        vals.push_back(val.value);
        size += bird_type_byte_size(val.type);
    }

    auto locals = this->function_locals[this->current_function_name];
    this->function_locals[this->current_function_name].push_back(BinaryenTypeInt32());

    auto identifier = std::to_string(locals.size()) + "temp";
    this->environment.declare(identifier, TaggedIndex(locals.size(),
                                                      type));

    auto size_literal = BinaryenConst(this->mod, BinaryenLiteralInt32(size));
    BinaryenExpressionRef local_set = this->binaryen_set(identifier,
                                                         BinaryenCall(this->mod,
                                                                      "mem_alloc",
                                                                      &size_literal,
                                                                      1,
                                                                      BinaryenTypeInt32()));

    children.push_back(local_set);

    unsigned int offset = 0;
    for (auto val : vals)
    {
        BinaryenExpressionRef args[3] = {
            this->binaryen_get(identifier),
            BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
            val};

        children.push_back(
            BinaryenCall(this->mod,
                         type->type == BirdTypeType::FLOAT ? "mem_set_64"
                                                           : "mem_set_32",
                         args,
                         3,
                         BinaryenTypeNone()));

        offset += bird_type_byte_size(type);
    }

    children.push_back(this->binaryen_get(identifier));

    auto block = BinaryenBlock(this->mod, nullptr, children.data(), children.size(), BinaryenTypeInt32());

    this->stack.push(TaggedExpression(block, std::make_shared<ArrayType>(type)));
}