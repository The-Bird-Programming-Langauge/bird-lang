
#include "../helpers/compile_helper.hpp"

TEST(MatchTest, MatchString)
{
  BirdTest::TestOptions options;
  options.code = "var x = \"hello\";"
                 "var y =  match (x) {"
                 "  \"hello\" => 1,"
                 "  \"world\" => 2,"
                 "  else => 3"
                 "};"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("y")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, MatchInt)
{
  BirdTest::TestOptions options;
  options.code = "var x = 1;"
                 "var y =  match (x) {"
                 "  1 => \"one\","
                 "  2 => \"two\","
                 "  else => \"three\""
                 "};"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<std::string>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<std::string>(interpreter.current_namespace->environment.get("y")), "one");
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "one\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, MatchFloat)
{
  BirdTest::TestOptions options;
  options.code = "var x = 1.0;"
                 "var y =  match (x) {"
                 "  1.0 => \"one\","
                 "  2.0 => \"two\","
                 "  else => \"three\""
                 "};"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<std::string>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<std::string>(interpreter.current_namespace->environment.get("y")), "one");
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "one\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, MatchBool)
{
  BirdTest::TestOptions options;
  options.code = "var x = true;"
                 "var y =  match (x) {"
                 "  true => \"true\","
                 "  false => \"false\","
                 "  else => \"neither\""
                 "};"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<std::string>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<std::string>(interpreter.current_namespace->environment.get("y")), "true");
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "true\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
