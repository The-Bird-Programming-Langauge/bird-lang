#include "../../../include/visitors/code_gen.h"

static bool object_should_be_cleaned(const TaggedIndex value) {
  const auto type = value.type->type;
  if (type == BirdTypeType::STRUCT || type == BirdTypeType::ARRAY ||
      type == BirdTypeType::PLACEHOLDER)
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
  std::set<std::string> marked;
  for (const auto &scope : this->environment.envs) {
    for (const auto &[key, value] : scope) {
      if (marked.find(key) != marked.end())
        continue;

      if (object_should_be_cleaned(value)) {
        marked.insert(key);
        auto allocated_block_ptr = this->binaryen_get(key);
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