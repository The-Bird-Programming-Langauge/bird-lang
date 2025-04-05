#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(ImportTest, ImportFunctionWithFrom) {
  BirdTest::TestOptions options;
  options.code =
  "import sin from Math::Trig"
  "print Math::Trig::sin(2.0,1.0);";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}