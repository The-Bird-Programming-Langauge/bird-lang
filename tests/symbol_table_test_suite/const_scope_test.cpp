#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(SymbolTableTest, ConstScope)
{
    BirdTest::TestOptions options;
    options.code = "const x = 0;"
                   "{"
                   "x = 1;"
                   "}";
    options.type_check = false;
    options.interpret = false;
    options.compile = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "x");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is not mutable. (line 0, character 14)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}
