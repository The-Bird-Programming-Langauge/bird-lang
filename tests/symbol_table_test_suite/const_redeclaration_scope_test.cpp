#include "../helpers/compile_helper.hpp"

TEST(SymbolTableTest, ConstRedeclarationScope)
{
  BirdTest::TestOptions options;
  options.code = "const x = 0;"
                 "{"
                 "const x = 1;"
                 "}"
                 "print x;";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.current_namespace->environment.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.current_namespace->environment.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.current_namespace->environment.get("x")), 0);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "0\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
