#include "../helpers/compile_helper.hpp"

TEST(MiscProgramsTestSuite, BubbleSort) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [ 5, 3, 2, 6, 4, 1, 9, 7, 8 ];"
                 "for var i: int = 0; i < 9 - 1; i += 1 {"
                 "    for var j: int = 0; j < 9 - 1 - i; j += 1 {"
                 "        if (x[j] > x[j + 1]) {"
                 "            var temp: int = x[j];"
                 "            x[j] = x[j + 1];"
                 "            x[j + 1] = temp;"
                 "        }"
                 "    }"
                 "}"
                 "for var i: int = 0; i < 9; i += 1 {"
                 "    print x[i];"
                 "}";

  using ARRAY_TYPE = std::shared_ptr<std::vector<Value>>;

  options.after_interpret = [&](auto interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));

    ASSERT_EQ(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->size(), 9);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(0)), 1);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(1)), 2);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(2)), 3);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(3)), 4);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(4)), 5);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(5)), 6);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(6)), 7);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(7)), 8);
    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(8)), 9);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n2\n3\n4\n5\n6\n7\n8\n9\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
