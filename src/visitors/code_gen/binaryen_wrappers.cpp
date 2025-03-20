#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

TaggedExpression CodeGen::binaryen_set(std::string identifier,
                                       BinaryenExpressionRef value) {
  // std::cout << "set: " << identifier << std::endl;
  TaggedIndex tagged_index = this->environment.get(identifier);
  if (this->environment.get_depth(identifier) != 0 &&
      this->current_function_name != "main") {
    // std::cout << "setting local" << std::endl;
    return BinaryenLocalSet(this->mod, tagged_index.value, value);
  } else {
    // std::cout << "setting global" << std::endl;
    return BinaryenGlobalSet(this->mod,
                             std::to_string(tagged_index.value).c_str(), value);
  }
}

TaggedExpression CodeGen::binaryen_get(std::string identifier) {
  TaggedIndex tagged_index = this->environment.get(identifier);
  // std::cout << "get: " << identifier << std::endl;
  if (this->environment.get_depth(identifier) != 0 &&
      this->current_function_name != "main") {
    // std::cout << "getting local" << std::endl;
    return TaggedExpression(
        BinaryenLocalGet(this->mod, tagged_index.value,
                         bird_type_to_binaryen_type(tagged_index.type)),
        tagged_index.type);
  } else {
    // std::cout << "getting global" << std::endl;
    return TaggedExpression(
        BinaryenGlobalGet(this->mod, std::to_string(tagged_index.value).c_str(),
                          bird_type_to_binaryen_type(tagged_index.type)),
        tagged_index.type);
  }
}
