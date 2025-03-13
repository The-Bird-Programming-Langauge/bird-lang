#include "../helpers/compile_helper.hpp"

TEST(FirstClassFunctionReturnTest, WithParams) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: fn(int)int) -> fn(int)int {return x;}";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReturnTest, UseReturnedFunction) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: fn(int)int) -> fn(int)int {return x;}"
                 "fn internal(x: int) {return x;}"
                 "const bar = foo(internal);"
                 "const result = bar(3);"
                 "print result;";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReturnTest, ReturnMismatchFunction) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn(int)int) -> fn(int)void { return bar; }";

  ASSERT_FALSE(BirdTest::compile(options));
}