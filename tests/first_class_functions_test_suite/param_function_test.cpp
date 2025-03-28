#include "../helpers/compile_helper.hpp"

TEST(FirstClassFunctionParamTest, WithParams) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn(int) -> void) {bar(3);}";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionParamTest, NoParams) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn() -> void) {bar();}";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionParamTest, ReturnInt) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn() -> int) -> int {return bar();}";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionParamTest, InternalParamTypeMismtach) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn(int) -> int) -> int { return bar(true);}";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FirstClassFunctionParamTest, ParamTypeMismtach) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn(int) -> int) -> int { return bar(3);}"
                 "fn x(y: int) { return y;}"
                 "foo(x);";
  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FirstClassFunctionParamTest, ReturnIntTypeMistmatch) {
  BirdTest::TestOptions options;
  options.code = "fn foo(bar: fn(int) -> int) -> str { return bar(3); }";

  ASSERT_FALSE(BirdTest::compile(options));
}