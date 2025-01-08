#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

// INT
TEST(ConstTest, ConstWithTypeInt)
{
    BirdTest::TestOptions options;
    options.code = "const x: int = 4;"
                   "print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 4);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "4\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// FLOAT
TEST(ConstTest, ConstWithTypeFloat)
{
    BirdTest::TestOptions options;
    options.code = "const x: float = 4.0;"
                   "print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<double>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<double>(interpreter.env.get("x")), 4.0);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "4\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// STRINGS
TEST(ConstTest, ConstWithTypeString)
{
    BirdTest::TestOptions options;
    options.code = "const x: str = \"hello\"; print x;";
    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::string>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<std::string>(interpreter.env.get("x")), "hello");
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "hello\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// BOOL
TEST(ConstTest, ConstWithTypeBool)
{
    BirdTest::TestOptions options;
    options.code = "const x: bool = true;"
                   "const y: bool = false;"
                   "print x;"
                   "print y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("y")), false);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "1\n0\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
