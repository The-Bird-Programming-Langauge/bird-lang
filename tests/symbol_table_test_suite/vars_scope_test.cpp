#include "../helpers/compile_helper.hpp"

TEST(SymbolTableTest, VarScope)
{
  BirdTest::TestOptions options;
  options.code = "var x = 0;"
                 "{"
                 "x = 1;"
                 "}"
                 "print x;";

  options.type_check = false;
  options.semantic_analyze = false;

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(SymbolTableTest, VarScopeInFunction)
{
  BirdTest::TestOptions options;
  options.code = "var x = 0;"
                 "fn foo() {"
                 "x = 1;"
                 "}"
                 "foo();"
                 "print x;";

  options.type_check = false;
  options.semantic_analyze = false;

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(NamespaceTest, TripleNestedNamespace)
{
  BirdTest::TestOptions options;
  options.code =
      "namespace Outer { "
      "  var a: int = 1; "
      "  struct OuterStruct { x: int };"
      "  fn outer_func() -> int { return 100; }"
      "  namespace Middle {"
      "    var b: float = 2.5;"
      "    struct MiddleStruct { y: float };"
      "    fn middle_func() -> float { return 2.5; }"
      "    namespace Inner {"
      "      var c: str = \"deep\";"
      "      struct InnerStruct { z: str };"
      "      fn inner_func() -> str { return \"nested\"; }"
      "    }"
      "  }"
      "}";

  options.compile = false;
  options.interpret = true;

  options.after_interpret = [&](Interpreter &interpreter)
  {
    auto outer = interpreter.global_namespace->nested_namespaces.find("Outer");
    EXPECT_NE(outer, interpreter.global_namespace->nested_namespaces.end());

    auto outer_ns = outer->second;

    EXPECT_TRUE(outer_ns->environment.contains("a"));
    EXPECT_TRUE(outer_ns->type_table.contains("OuterStruct"));
    EXPECT_TRUE(outer_ns->environment.contains("outer_func"));

    auto middle = outer_ns->nested_namespaces.find("Middle");
    EXPECT_NE(middle, outer_ns->nested_namespaces.end());

    auto middle_ns = middle->second;

    EXPECT_TRUE(middle_ns->environment.contains("b"));
    EXPECT_TRUE(middle_ns->type_table.contains("MiddleStruct"));
    EXPECT_TRUE(middle_ns->environment.contains("middle_func"));

    auto inner = middle_ns->nested_namespaces.find("Inner");
    EXPECT_NE(inner, middle_ns->nested_namespaces.end());

    auto inner_ns = inner->second;

    EXPECT_TRUE(inner_ns->environment.contains("c"));
    EXPECT_TRUE(inner_ns->type_table.contains("InnerStruct"));
    EXPECT_TRUE(inner_ns->environment.contains("inner_func"));
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
