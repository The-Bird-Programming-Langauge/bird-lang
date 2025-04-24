#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, StructField) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  const string: str = \"A\";"
                 "  struct B { string: str; };"
                 "  var n: B = B { string = string };"
                 "}"
                 "print A::n.string;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "A\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}