#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(ImportTest, ImportStruct) {
  BirdTest::TestOptions options;
  options.code =
  "import Math::Trig::Triangle"
  "var x = Math::Trig::Triangle{a=1.0,b=2.0,c=3.0};"
  "print x.a;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}