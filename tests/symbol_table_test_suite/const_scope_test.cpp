#include "../helpers/compile_helper.hpp"

TEST(SymbolTableTest, ConstScope) {
  BirdTest::TestOptions options;
  options.code = "const x = 0;"
                 "{"
                 "x = 1;"
                 "}";
  options.type_check = false;
  options.interpret = false;
  options.compile = false;

  options.after_semantic_analyze = [&](UserErrorTracker &error_tracker,
                                       SemanticAnalyzer &analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is "
                                "not mutable. (line 1, character 14)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(SymbolTableTest, ConstRedeclarationScopeInFunction) {
  BirdTest::TestOptions options;
  options.code = "const x = 0;"
                 "fn foo() {"
                 "x = 1;"
                 "}"
                 "foo();";
  options.type_check = false;
  options.interpret = false;
  options.compile = false;

  options.after_semantic_analyze = [&](UserErrorTracker &error_tracker,
                                       SemanticAnalyzer &analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is "
                                "not mutable. (line 1, character 23)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(SymbolTableTest, ConstScopeInFunction) {
  BirdTest::TestOptions options;
  options.code = "const x = 42;"
                 "fn foo() {"
                 "print x;"
                 "}"
                 "foo();";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 42);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "42\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}