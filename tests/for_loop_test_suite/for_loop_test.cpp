#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(ForLoopTest, ForLoopIncrement)
{
    BirdTest::TestOptions options;
    options.code = "var z = 0;"
                   "for var x: int = 0; x <= 5; x += 1 do {"
                   "z = x;"
                   "print z;"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("z"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("z")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("z")), 5);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForLoopTest, BreakOutsideLoop)
{
    BirdTest::TestOptions options;
    options.code = "break;";

    options.compile = false;
    options.interpret = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "break");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Break statement is declared outside of a loop. (line 0, character 5)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ForLoopTest, ContinueOutsideLoop)
{
    BirdTest::TestOptions options;
    options.code = "continue;";

    options.compile = false;
    options.interpret = false;

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "continue");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Continue statement is declared outside of a loop. (line 0, character 8)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ForLoopTest, ForLoopBreak)
{
    BirdTest::TestOptions options;
    options.code = "var z = 0;"
                   "for var x: int = 0; x <= 5; x += 1 do {"
                   "    z = x;"
                   "    print z;"
                   "    if z == 2 {"
                   "        break;"
                   "    }"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("z"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("z")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("z")), 2);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("2") != std::string::npos);
        ASSERT_TRUE(output.find("3") == std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForLoopTest, ForLoopContinue)
{
    BirdTest::TestOptions options;
    options.code = "var z = 0;"
                   "for var x: int = 0; x < 5; x += 1 do {"
                   "    print z;"
                   "    if z == 3 {continue;}"
                   "    z += 1;"
                   "    continue;"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("z"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("z")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("z")), 3);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("0") != std::string::npos);
        ASSERT_TRUE(output.find("1") != std::string::npos);
        ASSERT_TRUE(output.find("2") != std::string::npos);
        ASSERT_TRUE(output.find("3") == std::string::npos);
        ASSERT_TRUE(output.find("4") == std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForLoopTest, EmptyForLoop)
{
    BirdTest::TestOptions options;
    options.code = "var x = 0;"
                   "for ; ; do {"
                   "    x += 1;"
                   "    print x;"
                   "   if x == 5 {"
                   "       break;"
                   "   }"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("x")), 5);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_TRUE(output.find("1") != std::string::npos);
        ASSERT_TRUE(output.find("2") != std::string::npos);
        ASSERT_TRUE(output.find("3") != std::string::npos);
        ASSERT_TRUE(output.find("4") != std::string::npos);
        ASSERT_TRUE(output.find("5") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ForLoopTest, FalseForLoop) 
{
    BirdTest::TestOptions options;
    options.code = "var x = 0;"
                   "for ; false; do {"
                    "    x = 1;"
                   "    print x;"
                   "}";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
        EXPECT_EQ(as_type<int>(interpreter.env.get("x")), 0);
    };

    options.after_compile = [&](std::string &output, CodeGen &code_gen)
    {
        ASSERT_EQ(output, "\n");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}