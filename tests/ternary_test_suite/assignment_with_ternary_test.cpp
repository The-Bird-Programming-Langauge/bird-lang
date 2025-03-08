#include "../helpers/compile_helper.hpp"

TEST(TernaryTest, AssignmentWithTernary)
{
  BirdTest::TestOptions options;
  options.code = "var x = 1;"
                 "x += true ? 1 : 2;"
                 "print x;";

  options.type_check = false;
  options.semantic_analyze = false;

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}