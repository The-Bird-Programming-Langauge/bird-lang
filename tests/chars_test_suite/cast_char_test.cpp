#include "../helpers/compile_helper.hpp"

TEST(CastCharTest, CastCharToStr) {
  BirdTest::TestOptions options;
  options.code = "const c: char = 'c';"
                 "const s: str = c as str;"
                 "print s;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string output, CodeGen &code_gen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(CastCharTest, CastCharToStrFromSubscript) {
  BirdTest::TestOptions options;
  options.code = "const first: str = \"first\";"
                 "const second: str = first[0] as str;"
                 "print second;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string output, CodeGen &code_gen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(CastCharTest, CastCharToInt) {
  BirdTest::TestOptions options;
  options.code = "const c: char = 'c';"
                 "const i: int = c as int;"
                 "print i;";

  options.after_interpret = [&](Interpreter &interpreter) {};
  options.after_compile = [&](std::string output, CodeGen &code_gen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(CastCharTest, InvalidCastCharToFloat) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;
  options.code = "const c: char = 'c';"
                 "const f: float = c as float;"
                 "print f;";

  options.after_type_check = [&](auto &error_tracker, auto &interpreter) {

  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(CastCharTest, CastCharToStrPushToArray) {
  BirdTest::TestOptions options;
  options.code = "const chars: str[] = [];"
                 "const string: str = \"hello, world!\";"
                 "for var i = 0; i < length(string); i += 1 {"
                 "  push(chars, string[i] as str);"
                 "}"
                 "for var i = 0; i < length(chars); i += 1 {"
                 "  print chars[i];"
                 "}";

  options.after_compile = [&](std::string output, CodeGen &code_gen) {
    ASSERT_EQ(output, "h\ne\nl\nl\no\n,\n \nw\no\nr\nl\nd\n!\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}