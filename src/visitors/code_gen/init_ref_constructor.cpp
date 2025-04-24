#include "../../../include/visitors/code_gen.h"
#include <memory>

void CodeGen::init_ref_constructor() {
  const auto struct_type = std::make_shared<StructType>(
      "0ref",
      std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>{
          {"data",
           // data can be any struct
           std::make_shared<StructType>(
               "", std::vector<
                       std::pair<std::string, std::shared_ptr<BirdType>>>{})},
      });

  this->struct_name_to_num_pointers["0ref"] = 1;
  this->create_struct_constructor(struct_type);
}