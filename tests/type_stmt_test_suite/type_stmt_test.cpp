#include "../helpers/compile_helper.hpp"

TEST(TypeStmtTest, TypeRedeclaration)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "type x = bool;";

  options.after_semantic_analyze = [&](UserErrorTracker &error_tracker,
                                       SemanticAnalyzer &analyzer)
  {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is "
                                "already declared. (line 1, character 19)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(TypeStmtTest, TypeStmtWithTypeLiteral)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->type_table.contains("x"));
    ASSERT_EQ(interpreter.current_namespace->type_table.get("x")->type, BirdTypeType::INT);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_TRUE(codegen.get_type_table().contains("x"));
    ASSERT_EQ(codegen.get_type_table().get("x")->type, BirdTypeType::INT);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, TypeStmtWithTypeIdentifier)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "type y = x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->type_table.contains("y"));
    ASSERT_EQ(interpreter.current_namespace->type_table.get("y")->type, BirdTypeType::INT);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_TRUE(codegen.get_type_table().contains("x"));
    ASSERT_EQ(codegen.get_type_table().get("y")->type, BirdTypeType::INT);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, DeclStmtWithTypeIdentifer)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "var y: x = 2 as x;"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("y")), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_TRUE(codegen.get_environment().contains("y"));
    ASSERT_EQ(codegen.get_environment().get("y").type->type, BirdTypeType::INT);
    ASSERT_EQ(codegen.get_environment().get("y").value, 0);
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, ConstStmtWithTypeIdentifer)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "const y: x = 2 as x;"
                 "print y;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("y"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("y")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("y")), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_TRUE(codegen.get_environment().contains("y"));
    ASSERT_EQ(codegen.get_environment().get("y").type->type, BirdTypeType::INT);
    ASSERT_EQ(codegen.get_environment().get("y").value, 0);
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, FuncWithTypeIdentifier)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "fn foo(y: x) -> x"
                 "{"
                 "return y;"
                 "}"
                 "var z: x = foo(2 as x);"
                 "print foo(z);";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("z"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("z")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("z")), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_TRUE(codegen.get_environment().contains("z"));
    ASSERT_EQ(codegen.get_environment().get("z").type->type, BirdTypeType::INT);
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(TypeStmtTest, TypeDeclIdentiferRedeclaration)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "var x = 2;";

  options.after_semantic_analyze = [&](UserErrorTracker &error_tracker,
                                       SemanticAnalyzer &analyzer)
  {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is "
                                "already declared. (line 1, character 18)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(TypeStmtTest, FuncTypeIdentiferRedeclaration)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "fn x() -> int {return 3;}";

  options.after_semantic_analyze = [&](UserErrorTracker &error_tracker,
                                       SemanticAnalyzer &analyzer)
  {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] semantic error: Identifier 'x' is "
                                "already declared. (line 1, character 17)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(TypeStmtTest, DeclTypeMismatch)
{
  BirdTest::TestOptions options;
  options.code = "type x = int;"
                 "var y: x = true;";

  options.after_type_check = [&](UserErrorTracker &error_tracker,
                                 TypeChecker &type_checker)
  {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "x");
    ASSERT_EQ(std::get<0>(tup),
              ">>[ERROR] type mismatch: in declaration (line 1, character 21)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}