#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>
#include <memory>

void CodeGen::generate_array_length_fn() {

  std::string func_name = "length";
  auto bird_return_type = std::make_shared<IntType>();
  auto binaryen_return_type = bird_type_to_binaryen_type(bird_return_type);

  this->function_return_types[func_name] =
      TaggedType(binaryen_return_type, bird_return_type);

  std::vector<BinaryenType> param_types{bird_type_to_binaryen_type(
      std::make_shared<ArrayType>(std::make_shared<Generic>()))};

  BinaryenType params =
      BinaryenTypeCreate(param_types.data(), param_types.size());

  std::vector<BinaryenExpressionRef> function_body;

  auto ref = BinaryenLocalGet(this->mod, 0, BinaryenTypeInt32());
  BinaryenExpressionRef mem_get_array_args[2]{
      ref, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  auto array = BinaryenCall(this->mod, "mem_get_32", mem_get_array_args, 2,
                            BinaryenTypeInt32());

  BinaryenExpressionRef mem_get_args[2]{
      array, BinaryenConst(this->mod, BinaryenLiteralInt32(bird_type_byte_size(
                                          std::make_shared<IntType>())))};

  function_body.push_back(BinaryenCall(this->mod, "mem_get_32", mem_get_args, 2,
                                       BinaryenTypeInt32()));

  BinaryenExpressionRef body =
      BinaryenBlock(this->mod, nullptr, function_body.data(),
                    function_body.size(), BinaryenTypeInt32());

  BinaryenAddFunction(this->mod, func_name.c_str(), params,
                      binaryen_return_type, nullptr, 0, body);
}
void CodeGen::generate_iter_fn() {
  std::string func_name = "iter";
  auto return_type =
      std::make_shared<IteratorType>(std::make_shared<Generic>());
  BinaryenType binaryen_return_type = bird_type_to_binaryen_type(return_type);

  this->function_return_types[func_name] =
      TaggedType(binaryen_return_type, return_type);

  std::vector<BinaryenType> param_types{bird_type_to_binaryen_type(
      std::make_shared<ArrayType>(std::make_shared<Generic>()))};

  BinaryenType params =
      BinaryenTypeCreate(param_types.data(), param_types.size());

  std::vector<BinaryenExpressionRef> function_body;

  auto ref = BinaryenLocalGet(this->mod, 0, BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> mem_get_array_args{
      ref, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  auto array_ptr =
      BinaryenCall(this->mod, "mem_get_32", mem_get_array_args.data(),
                   mem_get_array_args.size(), BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> mem_get_args{
      array_ptr, BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  auto array = BinaryenCall(this->mod, "mem_get_32", mem_get_args.data(),
                            mem_get_args.size(), BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> length_args{
      array_ptr, BinaryenConst(this->mod, BinaryenLiteralInt32(4))};

  auto length = BinaryenCall(this->mod, "mem_get_32", length_args.data(),
                             length_args.size(), BinaryenTypeInt32());

  std::vector<BinaryenExpressionRef> mem_alloc_args{
      BinaryenConst(this->mod, BinaryenLiteralInt32(12)),
      BinaryenConst(this->mod, BinaryenLiteralInt32(1))};

  auto iter = BinaryenLocalSet(
      this->mod, 1,
      BinaryenCall(this->mod, "mem_alloc", mem_alloc_args.data(),
                   mem_alloc_args.size(), BinaryenTypeInt32()));

  auto get_iter_ptr = BinaryenLocalGet(this->mod, 1, BinaryenTypeInt32());

  function_body.push_back(iter);

  std::vector<BinaryenExpressionRef> mem_set_ptr{
      get_iter_ptr, BinaryenConst(this->mod, BinaryenLiteralInt32(0)), array};

  function_body.push_back(BinaryenCall(this->mod, "mem_set_32",
                                       mem_set_ptr.data(), mem_set_ptr.size(),
                                       BinaryenTypeNone()));

  std::vector<BinaryenExpressionRef> mem_set_len{
      get_iter_ptr, BinaryenConst(this->mod, BinaryenLiteralInt32(4)), length};

  function_body.push_back(BinaryenCall(this->mod, "mem_set_32",
                                       mem_set_len.data(), mem_set_len.size(),
                                       BinaryenTypeNone()));

  std::vector<BinaryenExpressionRef> mem_set_idx_args{
      get_iter_ptr, BinaryenConst(this->mod, BinaryenLiteralInt32(8)),
      BinaryenConst(this->mod, BinaryenLiteralInt32(0))};

  function_body.push_back(
      BinaryenCall(this->mod, "mem_set_32", mem_set_idx_args.data(),
                   mem_set_idx_args.size(), BinaryenTypeNone()));

  function_body.push_back(get_iter_ptr);

  std::vector<BinaryenType> var_types{BinaryenTypeInt32()};

  BinaryenAddFunction(this->mod, func_name.c_str(), params,
                      binaryen_return_type, var_types.data(), var_types.size(),
                      BinaryenBlock(this->mod, nullptr, function_body.data(),
                                    function_body.size(), BinaryenTypeInt32()));
}
