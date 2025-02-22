#include "../helpers/compile_helper.hpp"

TEST(WhileTest, While) {
  BirdTest::TestOptions options;
  options.code = "var x = 0; while x < 10 { x += 1; print x; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 10);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n\n", output);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileFalse) {
  BirdTest::TestOptions options;
  options.code = "var x = 0; while false { x = 1; print x; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 0);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ("\n", output);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileBreak) {
  BirdTest::TestOptions options;
  options.code = "var x = 1; while true { x = 2; print x; break; print 3; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ("2\n\n", output);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileContinue) {
  BirdTest::TestOptions options;
  options.code = "var x = 0; while true { x += 1; if (x <= 2) { continue; } "
                 "print x; break; print 4; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 3);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ("3\n\n", output);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(WhileTest, WhileConstInc) {
  BirdTest::TestOptions options;
  options.code =
      "const inc = 1; var x = 0; while x < 10 { x += inc; print x; }";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 10);

    ASSERT_TRUE(interpreter.env.contains("inc"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("inc")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("inc")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n\n", output);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
