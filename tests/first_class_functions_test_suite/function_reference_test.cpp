#include "../helpers/compile_helper.hpp"

TEST(FirstClassFunctionReferenceTest, WithParams) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: int) {}"
                 "const x: fn(int)void = foo;"
                 "x(3);";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, NoParams) {
  BirdTest::TestOptions options;
  options.code = "fn foo() {}"
                 "const x = foo;"
                 "x();";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, VoidReturn) {
  BirdTest::TestOptions options;
  options.code = "fn foo() -> void {}"
                 "const x: fn()void = foo;"
                 "x();";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ReturnInt) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: int) -> int {return x}"
                 "const x: fn(int)int = foo;"
                 "var y = x(3);"
                 "print y;";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ParamTypeMismtach) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: float) -> float {return x;}"
                 "const x: fn(float)float = foo;"
                 "var y = x(\"hello\");"
                 "print y;";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ReturnIntTypeMistmatch) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: str) -> str { return x; }"
                 "const x: fn(str)str = foo;"
                 "var y: int = x(\"hello\");"
                 "print y;";

  ASSERT_FALSE(BirdTest::compile(options));
}