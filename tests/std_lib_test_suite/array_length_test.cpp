#include "../helpers/compile_helper.hpp"

TEST(ArrayLengthTest, ArrayLengthInt) {
  BirdTest::TestOptions options;
  options.code = "const arr: int[] = [1,2,3,4,5];"
                 "var len = length(arr);"
                 "print(len);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("len"));
    ASSERT_EQ(as_type<int>(interpreter.env.get("len")), 5);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayLengthTest, ArrayLengthFloat) {
  BirdTest::TestOptions options;
  options.code = "const arr: float[] = [1.0,2.0,3.0,4.0,5.0];"
                 "var len = length(arr);"
                 "print(len);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("len"));
    ASSERT_EQ(as_type<int>(interpreter.env.get("len")), 5);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayLengthTest, ArrayLengthPush) {
  BirdTest::TestOptions options;
  options.code = "const arr: int[] = [1,2,3];"
                 "push(arr, 4);"
                 "push(arr, 5);"
                 "var len = length(arr);"
                 "print(len);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("len"));
    ASSERT_EQ(as_type<int>(interpreter.env.get("len")), 5);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}