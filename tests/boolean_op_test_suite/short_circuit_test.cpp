#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(BooleanOpTest, ShortCircuitTest)
{
    BirdTest::TestOptions options;
    options.code = "var x = false and (1/0 == 1);"
                   "var y = true or (1/0 == 1);"
                   "print x;"
                   "print y;";

    options.after_interpret = [](auto &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), false);

        ASSERT_TRUE(interpreter.env.contains("y"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("y")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("y")), true);
    };

    options.after_compile = [](auto &output, auto &codegen)
    {
        ASSERT_EQ(output, "0\n1\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}