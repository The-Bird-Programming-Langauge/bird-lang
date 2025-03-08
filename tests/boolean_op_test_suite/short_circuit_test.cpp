#include "../helpers/compile_helper.hpp"

TEST(ShortCircuitTest, ShortCircuitTest)
{
  BirdTest::TestOptions options;
  options.code = "var x = false and (1/0 == 1);"
                 "var y = true or (1/0 == 1);"
                 "print x;"
                 "print y;";

  options.after_interpret = [](auto &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("x")), false);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("y")), true);
  };

  options.after_compile = [](auto &output, auto &codegen)
  {
    ASSERT_EQ(output, "false\ntrue\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}