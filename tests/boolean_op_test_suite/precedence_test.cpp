#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(BooleanOpTest, AndOrPrecedenceTest)
{
    BirdTest::TestOptions options;
    options.code = "print false or true and true;";
}