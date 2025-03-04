#include "../helpers/compile_helper.hpp"

TEST(StructTest, DirectAccessTest) {
  BirdTest::TestOptions options;
  options.code = "struct Test { a: int; b: float; c: str; d: bool; }; "
                 "var t = Test { a = 1, b = 2.0, c = \"hello\", d = true }; "
                 "var a = t.a; "
                 "var b = t.b; "
                 "var c = t.c; "
                 "var d = t.d; "
                 "print t.a;"
                 "print t.b;"
                 "print t.c;"
                 "print t.d;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("a"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("a")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("a")), 1);

    ASSERT_TRUE(interpreter.env.contains("b"));
    ASSERT_TRUE(is_type<double>(interpreter.env.get("b")));
    ASSERT_EQ(as_type<double>(interpreter.env.get("b")), 2.0);

    ASSERT_TRUE(interpreter.env.contains("c"));
    ASSERT_TRUE(is_type<std::string>(interpreter.env.get("c")));
    ASSERT_EQ(as_type<std::string>(interpreter.env.get("c")), "hello");

    ASSERT_TRUE(interpreter.env.contains("d"));
    ASSERT_TRUE(is_type<bool>(interpreter.env.get("d")));
    ASSERT_EQ(as_type<bool>(interpreter.env.get("d")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\nhello\ntrue\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, DirectAccessTestWithNonExistentField) {
  BirdTest::TestOptions options;
  options.code = "struct Test { a: int; b: float; c: str; d: bool; }; "
                 "var t = Test { a = 1, b = 2.0, c = \"hello\", d = true }; "
                 "print t.e;";

  options.after_type_check = [&](UserErrorTracker error_tracker,
                                 TypeChecker &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "e");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type error: field does not exist on "
                                "struct (line 1, character 117)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}
