#include "../../../include/visitors/code_gen.h"

TaggedExpression CodeGen::binaryen_set(std::string identifier,
                                       BinaryenExpressionRef value) {
  TaggedIndex tagged_index = this->environment.get(identifier);
  if (this->environment.get_depth(identifier) != 0 &&
      this->current_function_name != "main") {
    return BinaryenLocalSet(this->mod, tagged_index.value, value);
  } else {
    return BinaryenGlobalSet(this->mod,
                             std::to_string(tagged_index.value).c_str(), value);
  }
}

TaggedExpression CodeGen::binaryen_get(std::string identifier) {
  TaggedIndex tagged_index = this->environment.get(identifier);
  if (this->environment.get_depth(identifier) != 0 &&
      this->current_function_name != "main") {
    return TaggedExpression(
        BinaryenLocalGet(this->mod, tagged_index.value,
                         bird_type_to_binaryen_type(tagged_index.type)),
        tagged_index.type);
  } else {
    return TaggedExpression(
        BinaryenGlobalGet(this->mod, std::to_string(tagged_index.value).c_str(),
                          bird_type_to_binaryen_type(tagged_index.type)),
        tagged_index.type);
  }
}
