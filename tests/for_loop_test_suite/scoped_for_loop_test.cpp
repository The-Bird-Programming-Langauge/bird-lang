#include "../helpers/compile_helper.hpp"

TEST(ForLoopTest, ScopedForLoopIncrement)
{
  BirdTest::TestOptions options;
  options.code = "var x = 0;"
                 "{"
                 "   for var y = 0; y <= 5; y += 1 {"
                 "       x = y;"
                 "   }"
                 "}"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    EXPECT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 5);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen)
  {
    ASSERT_TRUE(output.find("5") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
