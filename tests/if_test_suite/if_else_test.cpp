#include "helpers/compile_helper.hpp"

TEST(IfTest, IfTrueElse)
{
    BirdTest::TestOptions options;
    options.code = "var x = 1;"
                   "if true {"
                   "x = 2;"
                   "print x;"
                   "} else {"
                   "x = 3;"
                   "print x;"
                   "}";

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

TEST(IfTest, IfFalseElse)
{
    BirdTest::TestOptions options;
    options.code = "var x = 1;"
                   "if false {"
                   "x = 2;"
                   "print x;"
                   "} else {"
                   "x = 3;"
                   "print x;"
                   "}";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 3);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output, "3\n\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
