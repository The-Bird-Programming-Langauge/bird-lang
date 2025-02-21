#include "helpers/compile_helper.hpp"

TEST(FunctionTest, FunctionReturnTypeInt)
{
    BirdTest::TestOptions options;
    options.code = "fn function(i: int, j: int) -> int"
                   "{"
                   "return 3;"
                   "}"
                   "var result: int = function(2, 3);";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.call_table.contains("function"));
        ASSERT_TRUE(interpreter.env.contains("result"));
        auto result = interpreter.env.get("result");
        ASSERT_TRUE(is_type<int>(result));
        EXPECT_EQ(as_type<int>(result), 3);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FunctionTest, FunctionReturnTypeString)
{
    BirdTest::TestOptions options;
    options.code = "fn function(i: int, j: int) -> str"
                   "{"
                   "return \"string\";"
                   "}"
                   "var result: str = function(2, 3);";

    options.compile = false;

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.call_table.contains("function"));
        ASSERT_TRUE(interpreter.env.contains("result"));
        auto result = interpreter.env.get("result");
        ASSERT_TRUE(is_type<std::string>(result));
        EXPECT_EQ(as_type<std::string>(result), "string");
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FunctionTest, FunctionWrongReturnType)
{
    BirdTest::TestOptions options;
    options.code = "fn function(i: int, j: int) -> int"
                   "{"
                   "return \"string\";"
                   "}";

    options.after_type_check = [&](UserErrorTracker &error_tracker, TypeChecker &type_checker)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "return");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type mismatch: in return statement (line 1, character 36)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, FunctionNoReturnType)
{
    BirdTest::TestOptions options;
    options.code = "fn function()"
                   "{"
                   "return 34;"
                   "}";

    options.after_type_check = [&](UserErrorTracker &error_tracker, TypeChecker &type_checker)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "return");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type mismatch: in return statement (line 1, character 15)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, ExplicitReturnTypeNoReturn)
{
    BirdTest::TestOptions options;
    options.code = "fn function() -> int {}";

    options.after_semantic_analyze = [&](UserErrorTracker &error_tracker, SemanticAnalyzer &semantic_analyzer)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "function");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Function 'function' does not have a return statement. (line 1, character 4)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, FunctionWithJustReturn)
{
    // BirdTest::TestOptions options;

    // options.code = "fn function()"
    //                "{"
    //                "return;"
    //                "}";

    // ASSERT_TRUE(BirdTest::compile(options));
}