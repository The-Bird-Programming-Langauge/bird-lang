#include "../helpers/compile_helper.hpp"

// FLOATS
TEST(ConstTest, ConstWithoutTypeFloat)
{
  BirdTest::TestOptions options;
  options.code = "const x = 4.0;"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<double>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<double>(interpreter.current_namespace->environment.get("x")), 4.0);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "4\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// INTS
TEST(ConstTest, ConstWithoutTypeInt)
{
  BirdTest::TestOptions options;
  options.code = "const x = 4;"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 4.0);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "4\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// STRINGS
TEST(ConstTest, ConstWithoutTypeString)
{
  BirdTest::TestOptions options;
  options.code = "var x = \"hello\"; print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<std::string>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<std::string>(interpreter.current_namespace->environment.get("x")), "hello");
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "hello\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// BOOLS
TEST(ConstTest, ConstWithoutTypeBool)
{
  BirdTest::TestOptions options;
  options.code = "const x = true;"
                 "const y = false;"
                 "print x;"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("x")), true);
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("y")), false);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "true\nfalse\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
