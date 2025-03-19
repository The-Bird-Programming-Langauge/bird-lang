#include "../helpers/compile_helper.hpp"

TEST(FirstClassFunctionReturnTest, WithParams) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: fn(int) -> int) -> fn(int) -> int {return x;}";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReturnTest, UseReturnedFunction) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: fn(int) -> int) -> fn(int) -> int {return x;}"
                 "const bar = foo(fn(x: int) -> int { return x; });"
                 "const result = bar(3);"
                 "print result;";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReturnTest, ReturnMismatchFunction) {
  BirdTest::TestOptions options;
  options.code =
      "fn foo(bar: fn(int) -> void) -> fn(void) -> int { return bar; }";

  ASSERT_FALSE(BirdTest::compile(options));
}