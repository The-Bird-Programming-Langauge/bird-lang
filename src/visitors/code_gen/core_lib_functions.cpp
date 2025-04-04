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