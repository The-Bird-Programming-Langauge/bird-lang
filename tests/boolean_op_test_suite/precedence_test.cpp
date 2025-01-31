#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(BooleanOpTest, AndOrPrecedenceTest)
{
    BirdTest::TestOptions options;
    options.code = "var x = false or true and false;"
                   "print x;";

    options.after_interpret = [](auto &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), false);
    };

    options.after_compile = [](auto &output, auto &codegen)
    {
        ASSERT_EQ(output, "0\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}