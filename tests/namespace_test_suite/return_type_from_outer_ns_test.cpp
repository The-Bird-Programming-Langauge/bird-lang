#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, ReturnTypeFromOuterNamespace) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct Point { x: int; y: int; };"
                 "}"
                 "namespace B {"
                 "  fn origin() -> A::Point {"
                 "    return A::Point { x = 0, y = 0 };"
                 "  }"
                 "  var p = origin();"
                 "}"
                 "print B::p.x;"
                 "print B::p.y;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "0\n0\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
