#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(VarTest, VarRedeclaration)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0;"
                   "var x = 1;";

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "x");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is already declared. (line 0, character 15)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}
