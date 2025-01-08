#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(ConstTest, ConstImmutability)
{
    BirdTest::TestOptions options;
    options.code = "const x: int = 4; x = 5;";
    options.interpret = false;
    options.compile = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "x");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is not mutable. (line 0, character 19)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}
