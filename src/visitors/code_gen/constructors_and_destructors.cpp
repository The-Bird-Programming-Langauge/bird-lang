#include "visitors/code_gen.h"

CodeGen::~CodeGen()
{
    BinaryenModuleDispose(this->mod);
}

CodeGen::CodeGen() : type_converter(this->type_table, this->struct_names), mod(BinaryenModuleCreate())
{
    this->environment.push_env();
    this->type_table.push_env();
}