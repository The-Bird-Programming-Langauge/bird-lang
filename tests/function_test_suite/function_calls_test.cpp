#include "../helpers/compile_helper.hpp"

TEST(FunctionTest, GoodFunctionCall) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int) -> int {return i;} "
                 "var result: int = function(4);"
                 "print(result);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("function"));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<int>(result));
    EXPECT_EQ(as_type<int>(result), 4);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "4\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FunctionTest, MalformedCall) {
  BirdTest::TestOptions options;
  options.code = "fn function() {}"
                 "function(;";

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, ";");
    ASSERT_EQ(std::get<0>(tup),
              ">>[ERROR] expected identifier or i32  (line 1, character 25)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, CallWithIncorrectTypes) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int, j: str) {}"
                 "function(4, 6);";

  options.after_type_check = [&](UserErrorTracker &error_tracker,
                                 TypeChecker &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "function");
    ASSERT_EQ(
        std::get<0>(tup),
        ">>[ERROR] type mismatch: in function call (line 1, character 31)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, StoreReturnWithIncorrectVarType) {
  BirdTest::TestOptions options;
  options.code = "fn function() -> int {return 3;} "
                 "var result: str = function();";

  options.type_check = true;

  options.after_type_check = [&](UserErrorTracker &error_tracker,
                                 TypeChecker &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "str");
    ASSERT_EQ(std::get<0>(tup),
              ">>[ERROR] type mismatch: in declaration (line 1, character 46)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, ArityFail) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int, j: str) {}"
                 "function(4, 6, 7);";

  options.after_type_check = [&](auto &error_tracker, auto &analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "function");

    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type error: Invalid number of "
                                "arguments to function (line 1, character 31)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, FunctionRedeclaration) {
  BirdTest::TestOptions options;
  options.code = "fn x() -> int {return 3;}"
                 "fn x() -> int {return 1;}";

  options.after_semantic_analyze = [&](UserErrorTracker &error_tracker,
                                       SemanticAnalyzer &analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is "
                                "already declared. (line 1, character 29)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FunctionTest, FunctionIdentityCall) {
  BirdTest::TestOptions options;
  options.code = "fn function(y: int) -> int"
                 "{"
                 "return y;"
                 "}"
                 "var x: int = foo(2);"
                 "print foo(x);";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_TRUE(codegen.get_environment().contains("x"));
    ASSERT_EQ(codegen.get_environment().get("x").type->get_tag(), TypeTag::INT);
    ASSERT_EQ(output, "2\n\n");
  };
}

TEST(FunctionTest, FunctionReturnBool) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int) -> bool {return i < 0;}"
                 "var result: bool = function(4);"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("function"));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<bool>(result));
    EXPECT_EQ(as_type<bool>(result), false);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "0\n\n");
  };
}