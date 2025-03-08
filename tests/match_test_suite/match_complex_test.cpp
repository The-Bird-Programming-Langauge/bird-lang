#include "../helpers/compile_helper.hpp"

TEST(MatchTest, NestedMatch)
{
  BirdTest::TestOptions options;
  options.code = "var x = match 1 {"
                 "  1 => match 2 {"
                 "    2 => 1,"
                 "    else => 2"
                 "  },"
                 "  else => 3"
                 "};"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, ReturnMatch)
{
  BirdTest::TestOptions options;
  options.code = "fn foo() -> int {"
                 "  return match 1 {"
                 "    1 => 1,"
                 "    else => 2"
                 "  };"
                 "}"
                 "var x = foo();"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, VoidMatch)
{
  BirdTest::TestOptions options;
  options.code = "  match 1 {"
                 "    1 => print 1,"
                 "    else => print 2"
                 "  };";
  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(MatchTest, MatchInParams)
{
  BirdTest::TestOptions options;
  options.code = "fn foo(x: int) -> int {"
                 "  return x;"
                 "}"
                 "var x = foo(match 1 {"
                 "  1 => 1,"
                 "  else => 2"
                 "});"
                 "print x;";
  ;

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, MatchWithSideEffects)
{
  BirdTest::TestOptions options;
  options.code = "var x = 0;"
                 "match 1 {"
                 "  1 => x = 1,"
                 "  else => x = 2"
                 "};"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, MatchWithStruct)
{
  BirdTest::TestOptions options;
  options.code = "struct Foo {"
                 "  x: int"
                 "};"
                 "var x = match 1 {"
                 "   1 => Foo { x = 1 },"
                 "   else => Foo { x = 2 }"
                 "}.x;"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(MatchTest, MatchElse)
{
  BirdTest::TestOptions options;
  options.code = "var x = match 1 {"
                 "  2 => 1,"
                 "  else => 2"
                 "};"
                 "print x;";

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