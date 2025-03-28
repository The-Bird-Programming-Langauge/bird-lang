#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::init_lambda_constructor() {
  const auto struct_type = std::make_shared<StructType>(
      "0lambdaConstructor",
      std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>{
          {"fn_ptr", std::make_shared<IntType>()},        // index into table
          {"environment", std::make_shared<IntType>()}}); // unknown at runtime

  this->struct_name_to_num_pointers["0lambdaConstructor"] = 2;

  this->create_struct_constructor(struct_type);
}