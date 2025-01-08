#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(VarTest, VarMutability)
{
    BirdTest::TestOptions options;
    options.code = "var x: int = 4; print x; x = 5; print x;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 5);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("4") != std::string::npos);
        ASSERT_TRUE(output.find("5") != std::string::npos);
        ASSERT_TRUE(output.find("4") < output.find("5"));
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
