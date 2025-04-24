#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, ReturnFromOuterType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct X {"
                 "    val: int;"
                 "    fn double() -> int { return self.val * 2; }"
                 "  };"
                 "}"
                 "namespace B {"
                 "  var x: A::X = A::X { val = 21 };"
                 "}"
                 "print B::x.double();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "42\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}