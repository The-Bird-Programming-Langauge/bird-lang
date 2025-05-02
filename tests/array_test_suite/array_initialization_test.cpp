#include "../helpers/compile_helper.hpp"
#define ARRAY_TYPE std::shared_ptr<std::vector<Value>>

TEST(ArrayInitializationTest, TrailingCommaTest) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [1, 2, 3,];"
                 "print x[0];"
                 "print x[1];"
                 "print x[2];";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
