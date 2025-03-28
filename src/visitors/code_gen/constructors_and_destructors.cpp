#include "../../../include/visitors/code_gen.h"

CodeGen::~CodeGen() { BinaryenModuleDispose(this->mod); }

CodeGen::CodeGen(

    std::unordered_map<std::string, unsigned int> &function_capture_size)
    : function_capture_size(function_capture_size),
      type_converter(this->type_table, this->struct_names),
      mod(BinaryenModuleCreate()) {
  this->environment.push_env();
  this->type_table.push_env();
}