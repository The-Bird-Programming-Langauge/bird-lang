#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

static bool object_should_be_cleaned(const TaggedIndex value) {
  const auto type = value.type->type;
  if (type_is_on_heap(type))
    return true;

  if (type == BirdTypeType::STRING) {
    return safe_dynamic_pointer_cast<StringType>(value.type)->dynamic;
  }

  return false;
}

void CodeGen::garbage_collect() {
  // list that stores all of the javascript calls to be pushed on the stack as 1
  // block
  std::vector<BinaryenExpressionRef> calls;

  // mark all dynamically allocated blocks by traversing the environment, locate
  // all pointers pointing to dynamically allocated blocks, and pass the
  // pointers to the mark function
  for (int i = this->environment.envs.size() - 1; i >= 0; i--) {
    const auto scope = this->environment.envs[i];
    for (const auto &[key, value] : scope) {
      if (object_should_be_cleaned(value)) {
        auto allocated_block_ptr =
            i == 0 ? BinaryenGlobalGet(this->mod,
                                       std::to_string(value.value).c_str(),
                                       bird_type_to_binaryen_type(value.type))
                   : BinaryenLocalGet(this->mod, value.value,
                                      bird_type_to_binaryen_type(value.type));
        calls.push_back(BinaryenCall(this->mod, "mark", &allocated_block_ptr, 1,
                                     BinaryenTypeNone()));
      }
    }
  }

  // sweep all unmarked dynamically allocated blocks
  calls.push_back(
      BinaryenCall(this->mod, "sweep", nullptr, 0, BinaryenTypeNone()));

  // push all of the calls to the stack as 1 block
  this->stack.push(TaggedExpression(BinaryenBlock(
      this->mod, nullptr, calls.data(), calls.size(), BinaryenTypeNone())));
}