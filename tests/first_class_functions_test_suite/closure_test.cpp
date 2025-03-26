#include "../helpers/compile_helper.hpp"

TEST(FirstClassFunctionParamTest, GoodClosure) {
  BirdTest::TestOptions options;
  options.code =
      "fn foo() -> fn() -> int { var x = 1; return fn() -> int {return x;}; }"
      "foo()();"
      "foo()();"
      "foo()();";

  // TODO: uncomment
  options.compile = false;
  // ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionParamTest, ClosureWithShadow) {
  BirdTest::TestOptions options;
  options.code =
      "fn foo() -> fn() -> int { var x = 1; return fn() -> int {return x;}; }"
      "var x = 69;"
      "foo()();"
      "foo()();"
      "foo()();";

  // TODO: uncomment
  options.compile = false;
  // ASSERT_TRUE(BirdTest::compile(options));
}