#include "../helpers/compile_helper.hpp"

// INT
TEST(MiscProgramsTestSuite, FibTest) {
  BirdTest::TestOptions options;
  options.code = "fn fib(n: int) -> int { if (n <= 1) { return n; } return "
                 "fib(n - 1) + fib(n - 2); } var x = fib(10); print x;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 55);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_TRUE(output.find("55") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
