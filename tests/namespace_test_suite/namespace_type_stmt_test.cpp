#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, NamespaceTypeStmtTest) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "    namespace B {"
                 "        type number = int;"
                 "    }"
                 "    var x: B::number = 0;"
                 "}"
                 "print A::x;";

  options.interpret = false;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "0\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}