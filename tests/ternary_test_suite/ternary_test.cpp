#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(TernaryTest, TernaryTrue)
{
    BirdTest::TestOptions options;
    options.code = "var x = true ? 1 : 2;"
                   "print x;";

    options.type_check = false;
    options.semantic_analyze = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 1);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "1\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TernaryTest, TernaryFalse)
{
    BirdTest::TestOptions options;
    options.code = "var x = false ? 1 : 2;"
                   "print x;";

    options.type_check = false;
    options.semantic_analyze = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 2);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "2\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(BooleanOpTest, ShortCircuitTest)
{
    BirdTest::TestOptions options;
    options.code = "var x = true ? 1 : 1/0;"
                   "print x;";

    options.after_interpret = [](auto &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 1);
    };

    options.after_compile = [](auto &output, auto &codegen)
    {
        ASSERT_EQ(output, "1\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}