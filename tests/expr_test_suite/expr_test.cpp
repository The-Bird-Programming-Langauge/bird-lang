#include "../helpers/compile_helper.hpp"

// Add modulus operator test when it has been implemented.
TEST(ExprTest, BinaryExpr) {
  BirdTest::TestOptions options;
  options.code = "var x = (10 + 1) * 3 / -3 - -3;"
                 "print x;";

  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), -8);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "-8\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, BinaryStringString) {
  BirdTest::TestOptions options;
  options.code = "var x = \"hello\" + \"there\";"
                 "print x;";
  // TOOD: turn this on when we have strings
  options.compile = false;

  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<std::string>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<std::string>(interpreter.env.get("x")), "hellothere");
  };

  options.after_compile = [](std::string &output, CodeGen &code_gen) {
    ASSERT_EQ(output, "hellothere\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, BinaryIntString) {
  BirdTest::TestOptions options;
  options.code = "var x = 1 + \"test\";"
                 "print x;";

  options.after_type_check = [](UserErrorTracker &user_error_tracker,
                                TypeChecker &type_checker) {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] type mismatch: in binary operation (line 1, character 11)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ExprTest, BinaryFloatString) {
  BirdTest::TestOptions options;
  options.code = "var x = 1.1 + \"test\";"
                 "print x;";

  options.after_type_check = [](UserErrorTracker &user_error_tracker,
                                TypeChecker &type_checker) {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] type mismatch: in binary operation (line 1, character 13)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ExprTest, BinaryBoolInt) {
  BirdTest::TestOptions options;
  options.code = "var x = true + 11;"
                 "print x;";
  options.compile = false;

  options.after_type_check = [](UserErrorTracker &user_error_tracker,
                                TypeChecker &type_checker) {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] type mismatch: in binary operation (line 1, character 14)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ExprTest, CondExpr) {
  BirdTest::TestOptions options;
  options.code = "var x = 1 == 1 != false;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "true\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, CondExprIntInt) {
  BirdTest::TestOptions options;
  options.code = "var x = 1 >= 1;"
                 "print x;";

  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "true\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, CondExprFloatIntOverflow) {
  BirdTest::TestOptions options;
  options.code = "var x = 0.9999999999 < 1 as float;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "true\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, CondExprIntFloatOverflow) {
  BirdTest::TestOptions options;
  options.code = "var x = 1 as float > 0.9999999999;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "true\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, CondExprIntFloat) {
  BirdTest::TestOptions options;
  options.code = "var x = 10532 == 10532.0 as int;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<bool>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<bool>(interpreter.env.get("x")), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "true\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, CondExprBoolString) {
  BirdTest::TestOptions options;
  options.code = "var x = true < \"true\";"
                 "print x;";

  options.after_type_check = [](UserErrorTracker &user_error_tracker,
                                TypeChecker &type_checker) {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] type mismatch: in binary operation (line 1, character 14)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ExprTest, CondExprFloatBool) {
  BirdTest::TestOptions options;
  options.code = "var x = 1.1 >= \"true\";"
                 "print x;";

  options.after_type_check = [](UserErrorTracker &user_error_tracker,
                                TypeChecker &type_checker) {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] type mismatch: in binary operation (line 1, character 13)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ExprTest, IdentifierInExpr) {
  BirdTest::TestOptions options;
  options.code = "var z: int = 7; const y: float = -9.2; var x: int = 1 - (z * "
                 "y as int) - -y as int;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 55);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "55\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, BinaryDivideByZero) {
  BirdTest::TestOptions options;
  options.code = "var x: int = 10 / 0;";

  ASSERT_THROW(BirdTest::compile(options), BirdException);

  options.interpret = false;
  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "\n");
  };

  BirdTest::compile(options);
}

TEST(ExprTest, DivideByZeroHidden) {
  BirdTest::TestOptions options;
  options.code = "var x: int = 10 / (10 - 10);";

  ASSERT_THROW(BirdTest::compile(options), BirdException);
  options.interpret = false;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "\n");
  };

  BirdTest::compile(options);
}

TEST(ExprTest, BinaryModulus) {
  BirdTest::TestOptions options;
  options.code = "var x: int =  10 % (0 + 5) % 2 + 6 % 2 + 4 % 6;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 4);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "4\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, BinaryModulusFail) {
  BirdTest::TestOptions options;
  options.code = "var x: int = 10 % 0.0 as int;";
  ASSERT_THROW(BirdTest::compile(options), BirdException);
}

TEST(ExprTest, AssignModulus) {
  BirdTest::TestOptions options;
  options.code = "var x: int = 5; x %= 2;"
                 "print x;";
  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, Grouping) {
  BirdTest::TestOptions options;
  options.code = "var x: int = (1 + 1) * 2;"
                 "print x;";

  options.after_interpret = [](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<int>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<int>(interpreter.env.get("x")), 4);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "4\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ExprTest, UnclosedGrouping) {
  BirdTest::TestOptions options;
  options.code = "var x: int = (1 + 1 * 2;"
                 "print x;";

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto errors = error_tracker.get_errors();
    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(std::get<0>(errors[0]),
              ">>[ERROR] expected ) after grouping (line 1, character 24)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ExprTest, MalformedTernaryGrouping) {
  BirdTest::TestOptions options;
  options.code = "var x: int = true (? 1 : 2)"
                 "print x;";

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto errors = error_tracker.get_errors();
    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(std::get<0>(errors[0]),
              ">>[ERROR] expected identifier or i32  (line 1, character 20)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}