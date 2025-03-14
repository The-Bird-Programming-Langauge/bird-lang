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
                 "const x: ()void = foo;"
                 "x();";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ReturnInt) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: int) -> int {return x}"
                 "const x: (int)int = foo;"
                 "var y = x(3);"
                 "print y;";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ParamTypeMismtach) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: float) -> float {return x;}"
                 "const x: (float)float = foo;"
                 "var y = x(\"hello\");"
                 "print y;";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ReturnIntTypeMistmatch) {
  BirdTest::TestOptions options;
  options.code = "fn foo(x: str) -> str { return x; }"
                 "const x: (str)str = foo;"
                 "var y: int = x(\"hello\");"
                 "print y;";

  ASSERT_FALSE(BirdTest::compile(options));
}