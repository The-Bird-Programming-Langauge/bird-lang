#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::init_array_constructor() {
  const auto struct_type = std::make_shared<StructType>(
      "0array",
      std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>{
          {"data", std::make_shared<ArrayType>(std::make_shared<VoidType>())},
          {"length", std::make_shared<IntType>()}});

  this->struct_name_to_num_pointers["0array"] = 1;

  this->create_struct_constructor(struct_type);
}