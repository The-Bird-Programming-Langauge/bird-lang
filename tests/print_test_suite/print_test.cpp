#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(PrintTest, PrintBool)
{
    BirdTest::TestOptions options;
    options.code = "print true;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "1\n\n");
    };

    EXPECT_TRUE(BirdTest::compile(options));
}

TEST(PrintTest, PrintStr)
{
    BirdTest::TestOptions options;
    options.code = "print \"Hello, World!\";";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "Hello, World!\n\n");
    };

    EXPECT_TRUE(BirdTest::compile(options));
}

TEST(PrintTest, PrintInt)
{
    BirdTest::TestOptions options;
    options.code = "print 42;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "42\n\n");
    };

    EXPECT_TRUE(BirdTest::compile(options));
}

TEST(PrintTest, PrintFloat)
{
    BirdTest::TestOptions options;
    options.code = "print 42.42;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "42.42\n\n");
    };

    EXPECT_TRUE(BirdTest::compile(options));
}

TEST(PrintTest, PrintAlias)
{
    BirdTest::TestOptions options;
    options.code = "type num = int; var x: num = 42; print x;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "42\n\n");
    };

    EXPECT_TRUE(BirdTest::compile(options));
}

TEST(PrintTest, PrintStruct)
{
    BirdTest::TestOptions options;
    options.code = "type Point = struct { x: int, y: int }; var p: Point = { x: 1, y: 2 }; print p;";

    options.after_type_check = [&](UserErrorTracker &error_tracker, TypeChecker &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "print");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type error: cannot print struct type (line 8, character 1)");
    };

    EXPECT_FALSE(BirdTest::compile(options));
}

TEST(PrintTest, PrintVoid)
{
    BirdTest::TestOptions options;
    options.code = "fn foo() -> void { } print foo();";

    options.after_type_check = [&](UserErrorTracker &error_tracker, TypeChecker &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "print");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type error: cannot print void type (line 1, character 22)");
    };

    EXPECT_FALSE(BirdTest::compile(options));
}