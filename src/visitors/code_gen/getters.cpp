#include "visitors/code_gen.h"

Environment<TaggedIndex> &CodeGen::get_environment()
{
    return this->environment;
}

Environment<std::shared_ptr<BirdType>> &CodeGen::get_type_table()
{
    return this->type_table;
}