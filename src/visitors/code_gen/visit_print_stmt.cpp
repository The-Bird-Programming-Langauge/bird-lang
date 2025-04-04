#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_print_stmt(PrintStmt *print_stmt) {
  std::vector<BinaryenExpressionRef> calls;

  for (auto &arg : print_stmt->args) {
    arg->accept(this);
    auto result = this->stack.pop();

    if (result.type->get_tag() == TypeTag::VOID) {
      throw BirdException("unsupported print type");
    } else if (result.type->get_tag() == TypeTag::INT) {
      calls.push_back(BinaryenExpressionRef(BinaryenCall(
          this->mod, "print_i32", &result.value, 1, BinaryenTypeNone())));
    } else if (result.type->get_tag() == TypeTag::BOOL) {
      calls.push_back(BinaryenExpressionRef(BinaryenCall(
          this->mod, "print_bool", &result.value, 1, BinaryenTypeNone())));
    } else if (result.type->get_tag() == TypeTag::FLOAT) {
      calls.push_back(BinaryenExpressionRef(BinaryenCall(
          this->mod, "print_f64", &result.value, 1, BinaryenTypeNone())));
    } else if (result.type->get_tag() == TypeTag::STRING) {
      calls.push_back(BinaryenExpressionRef(BinaryenCall(
          this->mod, "print_str", &result.value, 1, BinaryenTypeNone())));
    } else if (result.type->get_tag() == TypeTag::CHAR) {
      calls.push_back(BinaryenExpressionRef(BinaryenCall(
          this->mod, "print_char", &result.value, 1, BinaryenTypeNone())));
    } else if (result.type->get_tag() == TypeTag::STRUCT) {
      throw BirdException("unsupported print type");
    } else {
      throw BirdException("Unsupported print datatype: " +
                          bird_type_to_string(result.type));
    }
  }

  // print an endline character
  calls.push_back(BinaryenExpressionRef(BinaryenCall(
      this->mod, "print_endline", nullptr, 0, BinaryenTypeNone())));

  // push all of the calls to the stack as 1 block
  this->stack.push(TaggedExpression(BinaryenBlock(
      this->mod, nullptr, calls.data(), calls.size(), BinaryenTypeNone())));
}
