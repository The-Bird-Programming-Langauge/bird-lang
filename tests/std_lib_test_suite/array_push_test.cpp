#include "../helpers/compile_helper.hpp"
#include <cassert>
#include <gtest/gtest.h>

TEST(ArrayPushTest, ArrayPushInt) {
  BirdTest::TestOptions options;
  options.code = "const arr: int[] = [1];"
                 "push(arr, 2);"
                 "push(arr, 3);"
                 "push(arr, 4);"
                 "push(arr, 5);"
                 "var len = length(arr);"
                 "print(len);"
                 "for var i = 0; i < length(arr); i += 1 { print arr[i]; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("len"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("len")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("len")), 5);
    ASSERT_TRUE(interpreter.env.contains("arr"));
    auto value = interpreter.env.get("arr");
    ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(value));
    auto arr = as_type<std::shared_ptr<std::vector<Value>>>(value);
    for (int i = 0; i < 5; i += 1) {
      auto el = (*arr)[i];
      ASSERT_TRUE(is_type<int>(el));
      ASSERT_EQ(as_type<int>(el), i + 1);
    }
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n1\n2\n3\n4\n5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayPushTest, ArrayPushFloat) {
  BirdTest::TestOptions options;
  options.code = "const arr: float[] = [1.0];"
                 "push(arr, 2.0);"
                 "push(arr, 3.0);"
                 "push(arr, 4.0);"
                 "push(arr, 5.0);"
                 "var len = length(arr);"
                 "print(len);"
                 "for var i = 0; i < length(arr); i += 1 { print arr[i]; }";
  ;

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("len"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("len")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("len")), 5);
    ASSERT_TRUE(interpreter.env.contains("arr"));
    auto value = interpreter.env.get("arr");
    ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(value));
    auto arr = as_type<std::shared_ptr<std::vector<Value>>>(value);
    for (int i = 0; i < 5; i += 1) {
      auto el = (*arr)[i];
      ASSERT_TRUE(is_type<double>(el));
      ASSERT_EQ(as_type<double>(el), i + 1);
    }
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n1\n2\n3\n4\n5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayPushTest, ArrayPushStruct) {
  BirdTest::TestOptions options;
  options.code = "struct Data { val: int; };"
                 "const arr: Data[] = [Data {val = 1}];"
                 "push(arr, Data {val = 2});"
                 "push(arr, Data {val = 3});"
                 "push(arr, Data {val = 4});"
                 "push(arr, Data {val = 5});"
                 "var len = length(arr);"
                 "print(len);"
                 "for var i = 0; i < length(arr); i += 1 { print arr[i].val; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("len"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("len")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("len")), 5);
    ASSERT_TRUE(interpreter.env.contains("arr"));
    auto value = interpreter.env.get("arr");
    ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(value));
    auto arr = as_type<std::shared_ptr<std::vector<Value>>>(value);
    for (int i = 0; i < 5; i += 1) {
      auto el = (*arr)[i];
      ASSERT_TRUE(is_type<Struct>(el));
      ASSERT_EQ(as_type<int>((*as_type<Struct>(el).fields)["val"]), i + 1);
    }
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n1\n2\n3\n4\n5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayPushTest, ArrayPushMismtach) {
  BirdTest::TestOptions options;
  options.code = "const arr: int[] = [1];"
                 "push(arr, true);";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ArrayPushTest, ArrayPushNoArray) {
  BirdTest::TestOptions options;
  options.code = "const arr: bool = true;"
                 "push(arr, true);";

  ASSERT_FALSE(BirdTest::compile(options));
}