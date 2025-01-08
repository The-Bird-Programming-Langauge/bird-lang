#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

// INT
TEST(VarTest, VarWithTypeInt)
{
    BirdTest::TestOptions options;
    options.code = "var x: int = 4; print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 4);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("4") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithTypeFloatExplicit)
{
    BirdTest::TestOptions options;
    options.code = "var x: float = 4.1; print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<double>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<double>(interpreter.env.get("x")), 4.1);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("4.1") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithTypeFloatImplicit)
{
    BirdTest::TestOptions options;
    options.code = "var y: float = 4; print y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<double>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<double>(interpreter.env.get("y")), 4.0);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("4") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// STRINGS
TEST(VarTest, VarWithTypeString)
{
    BirdTest::TestOptions options;
    options.code = "var x: str = \"hello\"; print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::string>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<std::string>(interpreter.env.get("x")), "hello");
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("hello") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// BOOL
TEST(VarTest, VarWithTypeBoolTrue)
{
    BirdTest::TestOptions options;
    options.code = "var x: bool = true; print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("1") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(VarTest, VarWithTypeBoolFalse)
{
    BirdTest::TestOptions options;
    options.code = "var y: bool = false; print y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("y")), false);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("0") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
