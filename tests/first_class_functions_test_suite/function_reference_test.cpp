#include "../helpers/compile_helper.hpp"

TEST(FirstClassFunctionReferenceTest, WithParams) {
  BirdTest::TestOptions options;
  options.code = "const x: (int)void = fn(x: int) -> void {};"
                 "x(3);";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, NoParams) {
  BirdTest::TestOptions options;
  options.code = "const x = fn() -> void {};"
                 "x();";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, VoidReturn) {
  BirdTest::TestOptions options;
  options.code = "const x: ()void = fn() -> void { print 3; };"
                 "x();";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ReturnInt) {
  BirdTest::TestOptions options;
  options.code = "const x: (int)int = fn(x: int) -> int { return x;};"
                 "var y = x(3);"
                 "print y;";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ParamTypeMismtach) {
  BirdTest::TestOptions options;
  options.code = "const x: (float)float = (x: float) -> float {return x;}"
                 "var y = x(\"hello\");"
                 "print y;";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(FirstClassFunctionReferenceTest, ReturnIntTypeMistmatch) {
  BirdTest::TestOptions options;
  options.code = "const x: (str)str = fn(x: str) -> str { return x; }"
                 "var y: int = x(\"hello\");"
                 "print y;";

  ASSERT_FALSE(BirdTest::compile(options));
}