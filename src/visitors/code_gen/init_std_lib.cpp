#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::init_std_lib() {
  BinaryenAddFunctionImport(this->mod, "print_i32", "env", "print_i32", BinaryenTypeInt32(), BinaryenTypeNone());
  BinaryenAddFunctionImport(this->mod, "print_f64", "env", "print_f64", BinaryenTypeFloat64(), BinaryenTypeNone());
  BinaryenAddFunctionImport(this->mod, "print_bool", "env", "print_bool", BinaryenTypeInt32(), BinaryenTypeNone());
  BinaryenAddFunctionImport(this->mod, "print_str", "env", "print_str", BinaryenTypeInt32(), BinaryenTypeNone());
  BinaryenAddFunctionImport(this->mod, "print_endline", "env", "print_endline", BinaryenTypeNone(), BinaryenTypeNone());

  BinaryenType args[2] = {BinaryenTypeInt32(),
                          BinaryenTypeInt32()}; // pointer and index
  BinaryenType args_type = BinaryenTypeCreate(args, 2);
  BinaryenAddFunctionImport(this->mod, "mem_get_32", "env", "mem_get_32",
                            args_type, BinaryenTypeInt32());

  BinaryenAddFunctionImport(this->mod, "mem_get_64", "env", "mem_get_64",
                            args_type, BinaryenTypeFloat64());

  // mem_set(pointer, index, value)
  BinaryenType args_set[3] = {BinaryenTypeInt32(), BinaryenTypeInt32(),
                              BinaryenTypeInt32()};
  BinaryenType args_set_type = BinaryenTypeCreate(args_set, 3);
  BinaryenAddFunctionImport(this->mod, "mem_set_32", "env", "mem_set_32",
                            args_set_type, BinaryenTypeNone());

  BinaryenType args_set_64[3] = {BinaryenTypeInt32(), BinaryenTypeInt32(),
                                 BinaryenTypeFloat64()};
  BinaryenType args_set_64_type = BinaryenTypeCreate(args_set_64, 3);
  BinaryenAddFunctionImport(this->mod, "mem_set_64", "env", "mem_set_64",
                            args_set_64_type, BinaryenTypeNone());

  BinaryenAddFunctionImport(this->mod, "mem_alloc", "env", "mem_alloc",
                            args_type, BinaryenTypeInt32());

  BinaryenAddFunctionImport(this->mod, "register_root", "env", "register_root",
                            BinaryenTypeInt32(), BinaryenTypeNone());

  BinaryenAddFunctionImport(this->mod, "unregister_root", "env",
                            "unregister_root", BinaryenTypeInt32(),
                            BinaryenTypeNone());

  BinaryenAddFunctionImport(this->mod, "gc", "env", "gc", BinaryenTypeNone(),
                            BinaryenTypeNone());

  generate_array_length_fn();
  BinaryenAddFunctionImport(this->mod, "strcmp", "env", "strcmp", args_type,
                            BinaryenTypeInt32());

  BinaryenAddFunctionImport(this->mod, "strcat", "env", "strcat", args_type,
                            BinaryenTypeInt32());

  BinaryenAddFunctionImport(this->mod, "push_32", "env", "push_32", args_type,
                            BinaryenTypeNone());
  BinaryenAddFunctionImport(this->mod, "push_ptr", "env", "push_ptr", args_type,
                            BinaryenTypeNone());

  BinaryenType args_int_float[2] = {BinaryenTypeInt32(),
                                    BinaryenTypeFloat64()}; // pointer and index
  BinaryenType args_int_float_type = BinaryenTypeCreate(args_int_float, 2);

  BinaryenAddFunctionImport(this->mod, "push_64", "env", "push_64",
                            args_int_float_type, BinaryenTypeNone());
}