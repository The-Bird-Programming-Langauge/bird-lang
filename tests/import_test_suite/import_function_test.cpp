#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(ImportTest, ImportFunction) {
  BirdTest::TestOptions options;
  options.code =
  "import Math::Trig::sin\n"
  "print Math::Trig::sin(2.0,1.0);";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}