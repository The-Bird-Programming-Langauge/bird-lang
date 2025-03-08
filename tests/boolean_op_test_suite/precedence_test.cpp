#include "../helpers/compile_helper.hpp"

TEST(PrecedenceTest, AndOrPrecedenceTest)
{
  BirdTest::TestOptions options;
  options.code = "var x = false or true and false;"
                 "print x;";

  options.after_interpret = [](auto &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("x")), false);
  };

  options.after_compile = [](auto &output, auto &codegen)
  {
    ASSERT_EQ(output, "false\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}