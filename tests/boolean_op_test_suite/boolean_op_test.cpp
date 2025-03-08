#include "../helpers/compile_helper.hpp"

TEST(BooleanOpTest, AndTest)
{
  BirdTest::TestOptions options;
  options.code = "var tt = true and true;"
                 "var tf = true and false;"
                 "var ft = false and true;"
                 "var ff = false and false;"
                 "print tt;"
                 "print tf;"
                 "print ft;"
                 "print ff;";

  options.after_interpret = [](auto &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("tt"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("tt")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("tt")), true);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("tf"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("tf")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("tf")), false);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("ft"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("ft")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("ft")), false);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("ff"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("ff")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("ff")), false);
  };

  options.after_compile = [](auto &output, auto &codegen)
  {
    ASSERT_EQ(output, "true\nfalse\nfalse\nfalse\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(BooleanOpTest, OrTest)
{
  BirdTest::TestOptions options;
  options.code = "var tt = true or true;"
                 "var tf = true or false;"
                 "var ft = false or true;"
                 "var ff = false or false;"
                 "print tt;"
                 "print tf;"
                 "print ft;"
                 "print ff;";

  options.after_interpret = [](auto &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("tt"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("tt")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("tt")), true);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("tf"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("tf")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("tf")), true);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("ft"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("ft")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("ft")), true);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("ff"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("ff")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("ff")), false);
  };

  options.after_compile = [](auto &output, auto &codegen)
  {
    ASSERT_EQ(output, "true\ntrue\ntrue\nfalse\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(BooleanOpTest, XorTest)
{
  BirdTest::TestOptions options;
  options.code = "var tt = true xor true;"
                 "var tf = true xor false;"
                 "var ft = false xor true;"
                 "var ff = false xor false;"
                 "print tt;"
                 "print tf;"
                 "print ft;"
                 "print ff;";

  options.after_interpret = [](auto &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("tt"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("tt")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("tt")), false);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("tf"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("tf")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("tf")), true);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("ft"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("ft")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("ft")), true);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("ff"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("ff")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("ff")), false);
  };

  options.after_compile = [](auto &output, auto &codegen)
  {
    ASSERT_EQ(output, "false\ntrue\ntrue\nfalse\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(BooleanOpTest, NotTest)
{
  BirdTest::TestOptions options;
  options.code = "var t = not true;"
                 "var f = not false;"
                 "print t;"
                 "print f;";

  options.after_interpret = [](auto &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("t"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("t")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("t")), false);

    ASSERT_TRUE(interpreter.current_namespace->environment.contains("f"));
    ASSERT_TRUE(is_type<bool>(interpreter.current_namespace->environment.get("f")));
    ASSERT_EQ(as_type<bool>(interpreter.current_namespace->environment.get("f")), true);
  };

  options.after_compile = [](auto &output, auto &codegen)
  {
    ASSERT_EQ(output, "false\ntrue\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}