#include "../helpers/compile_helper.hpp"

// FLOATS
TEST(VarTest, VarWithoutTypeFloat)
{
  BirdTest::TestOptions options;
  options.code = "var x = 4.0; print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<double>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<double>(interpreter.current_namespace->environment.get("x")), 4.0);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen)
  {
    ASSERT_TRUE(output.find("4") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// INTS
TEST(VarTest, VarWithoutTypeInt)
{
  BirdTest::TestOptions options;
  options.code = "var x = 4; print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 4);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen)
  {
    ASSERT_TRUE(output.find("4") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// STRINGS
TEST(VarTest, VarWithoutTypeString)
{
  BirdTest::TestOptions options;
  options.code = "var x = \"hello\"; print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<std::string>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<std::string>(interpreter.current_namespace->environment.get("x")), "hello");
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen)
  {
    ASSERT_TRUE(output.find("hello") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// BOOLS
TEST(VarTest, VarWithoutTypeBoolTrue)
{
  BirdTest::TestOptions options;
  options.code = "var x = true; print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("x")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen)
  {
    ASSERT_TRUE(output.find("true") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithoutTypeBoolFalse)
{
  BirdTest::TestOptions options;
  options.code = "var y = false; print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("y")), false);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen)
  {
    ASSERT_TRUE(output.find("false") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
