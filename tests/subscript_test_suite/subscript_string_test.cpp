#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(SubscriptTest, SubscriptStringValid) {
  BirdTest::TestOptions options;
  options.code = "const string: str = \"hello\";"
                 "const result = string[0];"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string &output, CodeGen &codegen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(SubscriptTest, SubscriptStringInvalid) {
  BirdTest::TestOptions options;
  options.code = "const string: str = \"hello\";"
                 "const result = string[10];"
                 "print result;";

  ASSERT_THROW(BirdTest::compile(options), BirdException);
  options.interpret = false;
  ASSERT_FALSE(BirdTest::compile(options));
}
