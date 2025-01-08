#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(SymbolTableTest, VarScope)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0;"
                   "{"
                   "x = 1;"
                   "}"
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
