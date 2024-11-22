#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/value.h"
#include "../../include/visitors/interpreter.h"
#include "../../src/callable.cpp"
#include "../helpers/parse_test_helper.hpp"
#include "../../include/visitors/semantic_analyzer.h"
#include "../../include/visitors/type_checker.h"

TEST(FunctionTest, FunctionReturnTypeInt)
{
    auto code = "fn function(i: int, j: int) -> int"
                "{"
                "return 3;"
                "}"
                "var result: int = function(2, 3);";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.call_table.contains("function"));

    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<int>(result));
    EXPECT_EQ(as_type<int>(result), 3);
}

TEST(FunctionTest, FunctionReturnTypeString)
{
    auto code = "fn function(i: int, j: int) -> str"
                "{"
                "return \"string\";"
                "}"
                "var result: str = function(2, 3);";
    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    Interpreter interpreter;
    interpreter.evaluate(&ast);

    ASSERT_TRUE(interpreter.call_table.contains("function"));

    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<std::string>(result));
    EXPECT_EQ(as_type<std::string>(result), "string");
}

TEST(FunctionTest, FunctionWrongReturnType)
{
    auto code = "fn function(i: int, j: int) -> int"
                "{"
                "return \"string\";"
                "}";

    auto ast = parse_code(code);

    auto user_error_tracker = UserErrorTracker(code);
    SemanticAnalyzer analyze_semantics(&user_error_tracker);
    analyze_semantics.analyze_semantics(&ast);
    ASSERT_FALSE(user_error_tracker.has_errors());

    TypeChecker type_checker(&user_error_tracker);
    type_checker.check_types(&ast);

    ASSERT_TRUE(user_error_tracker.has_errors());
    auto errors = user_error_tracker.get_errors();
    EXPECT_EQ(errors.size(), 1);
}