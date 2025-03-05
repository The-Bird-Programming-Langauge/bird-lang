#include "../helpers/compile_helper.hpp"

TEST(FunctionParamsTest, TooManyParams) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int) -> int {return i;} "
                 "var result: int = function(4, 2);"
                 "print(result);";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionParamsTest, ParamMutability) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int) -> int {"
                 "i = 5;"
                 "return i;"
                 "}"
                 "var result: int = function(4);"
                 "print(result);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.call_table.contains("function"));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<int>(result));
    EXPECT_EQ(as_type<int>(result), 5);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FunctionParamsTest, StructReference) {
  BirdTest::TestOptions options;
  options.code = "struct Person {name: str;};"
                 "fn function(p: Person) {"
                 "  p.name = \"different\";"
                 "}"
                 "var input = Person { name = \"first\"};"
                 "function(input);"
                 "print(input.name);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.call_table.contains("function"));
    ASSERT_TRUE(interpreter.env.contains("input"));
    auto result = interpreter.env.get("input");
    auto type_is_map = is_type<Struct>(result);
    ASSERT_TRUE(type_is_map);
    EXPECT_EQ(as_type<std::string>(as_type<Struct>(result).fields->at("name")),
              "different");
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "different\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FunctionParamsTest, ArrayReferenceTest) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int[]) -> void {"
                 "i[0] = 5;"
                 "}"
                 "var input = [1,2,3];"
                 "function(input);"
                 "print(input[0]);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.call_table.contains("function"));
    ASSERT_TRUE(interpreter.env.contains("input"));
    auto result = interpreter.env.get("input");
    ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(result));
    EXPECT_EQ(as_type<int>(
                  (*as_type<std::shared_ptr<std::vector<Value>>>(result))[0]),
              5);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "5\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}