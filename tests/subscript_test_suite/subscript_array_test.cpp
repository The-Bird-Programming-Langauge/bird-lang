#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(SubscriptTest, SubscriptArrayValid) {
  BirdTest::TestOptions options;
  options.code = "const foo: int[] = [1,2,3];"
                 "const result = foo[0];"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(SubscriptTest, SubscriptArrayInvalid) {
  BirdTest::TestOptions options;
  options.code = "const foo: int[] = [1,2,3];"
                 "const result = foo[10];"
                 "print result;";

  ASSERT_THROW(BirdTest::compile(options), BirdException);

  options.interpret = false;
  options.after_compile = [&](std::string output, CodeGen &code_gen) {
    // an exception was thrown during runtime
    ASSERT_EQ(output, "\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
