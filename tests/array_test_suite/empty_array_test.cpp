#include "../helpers/compile_helper.hpp"
#define ARRAY_TYPE std::shared_ptr<std::vector<Value>>

TEST(EmptyArrayTest, EmptyArrayToInt) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [];"
                 "push(x, 1);"
                 "var result: int = x[0];"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<int>(result));
    ASSERT_EQ(as_type<int>(result), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, EmptyArrayToFloat) {
  BirdTest::TestOptions options;
  options.code = "var x: float[] = [];"
                 "push(x, 3.9);"
                 "var result: float = x[0];"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<double>(result));
    ASSERT_EQ(as_type<double>(result), 3.9);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "3.9\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, EmptyArrayToStruct) {
  BirdTest::TestOptions options;
  options.code = "struct Data { val: int; };"
                 "var x: Data[] = [];"
                 "push(x, Data {val = 3});"
                 "var result: Data = x[0];"
                 "print result.val;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<Struct>(result));
    ASSERT_EQ(as_type<int>((*as_type<Struct>(result).fields)["val"]), 3);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}