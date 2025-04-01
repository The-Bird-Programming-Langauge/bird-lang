#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(DelcareCharTest, DeclareCharLiteral) {
  BirdTest::TestOptions options;
  options.code = "const c: char = 'x';"
                 "print c;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string output, CodeGen &code_gen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(DelcareCharTest, DeclareCharFromSubscript) {
  BirdTest::TestOptions options;
  options.code = "const c: char = \"foobar\"[0];"
                 "print c;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string output, CodeGen &code_gen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(DelcareCharTest, DeclareCharWithoutType) {
  BirdTest::TestOptions options;
  options.code = "const c = 'c';"
                 "print c;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string output, CodeGen &code_gen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(DelcareCharTest, InvalidDeclareChar) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;
  options.code = "const c: str = 'c';";

  options.after_type_check = [&](auto &error_tracker, auto &type_checker) {

  };

  ASSERT_FALSE(BirdTest::compile(options));
}
