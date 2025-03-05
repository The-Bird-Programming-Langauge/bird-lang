#include "../helpers/compile_helper.hpp"

#define STRUCT_TYPE Struct
#define ARRAY_TYPE std::shared_ptr<std::vector<Value>>

static void struct_init_helper(Interpreter &interpreter) {
  ASSERT_TRUE(interpreter.env.contains("p"));
  bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("p"));
  ASSERT_TRUE(is_correct_type);
  auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("p"));
  ASSERT_TRUE(instance.fields->find("x") != instance.fields->end());
  ASSERT_TRUE(instance.fields->find("y") != instance.fields->end());

  ASSERT_TRUE(is_type<int>((*instance.fields)["x"]));
  ASSERT_TRUE(is_type<int>((*instance.fields)["y"]));

  ASSERT_EQ(as_type<int>((*instance.fields)["x"]), 1);
  ASSERT_EQ(as_type<int>((*instance.fields)["y"]), 2);
}

TEST(StructTest, VarStructInitializationTest) {
  BirdTest::TestOptions options;
  options.code = "struct Point { x: int; y: int; };"
                 "var p = Point { x = 1, y = 2 };"
                 "print p.x;"
                 "print p.y;";

  options.after_interpret = struct_init_helper;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, ConstStructInitializationTest) {
  BirdTest::TestOptions options;
  options.code = "struct Point { x: int; y: int; };"
                 "const p = Point { x = 1, y = 2 };"
                 "print p.x;"
                 "print p.y;";

  options.after_interpret = struct_init_helper;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, FunctionStructInitializationTest) {
  BirdTest::TestOptions options;
  options.code = "struct Point { x: int; y: int; };"
                 "fn get_point() -> Point { return Point { x = 1, y = 2 }; }"
                 "var p = get_point();"
                 "print get_point().x;"
                 "print get_point().y;";

  options.after_interpret = struct_init_helper;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, AllPrimitiveTypesStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct Test { a: int; b: float; c: str; d: bool; };"
                 "var t = Test { a = 1, b = 2.0, c = \"hello\", d = true };"
                 "print t.a;"
                 "print t.b;"
                 "print t.c;"
                 "print t.d;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("t"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(instance.fields->find("a") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("b") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("c") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("d") != instance.fields->end());

    ASSERT_TRUE(is_type<int>((*instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*instance.fields)["a"]), 1);
    ASSERT_EQ(as_type<double>((*instance.fields)["b"]), 2.0);
    ASSERT_EQ(as_type<std::string>((*instance.fields)["c"]), "hello");
    ASSERT_EQ(as_type<bool>((*instance.fields)["d"]), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\nhello\ntrue\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, HoistedStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "var t = Test { a = 1, b = 2.0, c = \"hello\", d = true };"
                 "print t.a;"
                 "print t.b;"
                 "print t.c;"
                 "print t.d;"
                 "struct Test { a: int; b: float; c: str; d: bool; };";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("t"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(instance.fields->find("a") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("b") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("c") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("d") != instance.fields->end());

    ASSERT_TRUE(is_type<int>((*instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*instance.fields)["a"]), 1);
    ASSERT_EQ(as_type<double>((*instance.fields)["b"]), 2.0);
    ASSERT_EQ(as_type<std::string>((*instance.fields)["c"]), "hello");
    ASSERT_EQ(as_type<bool>((*instance.fields)["d"]), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\nhello\ntrue\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, ParamsOutOfOrderStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct Test { a: int; b: float; c: str; d: bool; };"
                 "var t = Test { b = 2.0, a = 1, d = true, c = \"hello\" };"
                 "print t.a;"
                 "print t.b;"
                 "print t.c;"
                 "print t.d;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("t"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(instance.fields->find("a") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("b") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("c") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("d") != instance.fields->end());

    ASSERT_TRUE(is_type<int>((*instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*instance.fields)["a"]), 1);
    ASSERT_EQ(as_type<double>((*instance.fields)["b"]), 2.0);
    ASSERT_EQ(as_type<std::string>((*instance.fields)["c"]), "hello");
    ASSERT_EQ(as_type<bool>((*instance.fields)["d"]), true);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\nhello\ntrue\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, NoParamsStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct Test { a: int; b: float; c: str; d: bool; };"
                 "var t = Test {};"
                 "print t.a;"
                 "print t.b;"
                 "print t.c;"
                 "print t.d;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("t"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(instance.fields->find("a") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("b") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("c") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("d") != instance.fields->end());

    ASSERT_TRUE(is_type<int>((*instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*instance.fields)["a"]), 0);
    ASSERT_EQ(as_type<double>((*instance.fields)["b"]), 0);
    ASSERT_EQ(as_type<std::string>((*instance.fields)["c"]), "");
    ASSERT_EQ(as_type<bool>((*instance.fields)["d"]), false);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "0\n0\n\nfalse\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, MultipleStructInitialization) {
  BirdTest::TestOptions options;
  options.code =
      "struct Test { a: int; b: float; c: str; d: bool; };"
      "var first = Test { a = 1, b = 2.0, c = \"hello\", d = true };"
      "var second = Test { a = 2, b = 3.0, c = \"world\", d = false };"
      "print first.a;"
      "print first.b;"
      "print first.c;"
      "print first.d;"
      "print second.a;"
      "print second.b;"
      "print second.c;"
      "print second.d;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("first"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("first"));
    ASSERT_TRUE(is_correct_type);
    auto first_instance = as_type<STRUCT_TYPE>(interpreter.env.get("first"));
    ASSERT_TRUE(first_instance.fields->find("a") !=
                first_instance.fields->end());
    ASSERT_TRUE(first_instance.fields->find("b") !=
                first_instance.fields->end());
    ASSERT_TRUE(first_instance.fields->find("c") !=
                first_instance.fields->end());
    ASSERT_TRUE(first_instance.fields->find("d") !=
                first_instance.fields->end());

    ASSERT_TRUE(is_type<int>((*first_instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*first_instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*first_instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*first_instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*first_instance.fields)["a"]), 1);
    ASSERT_EQ(as_type<double>((*first_instance.fields)["b"]), 2.0);
    ASSERT_EQ(as_type<std::string>((*first_instance.fields)["c"]), "hello");
    ASSERT_EQ(as_type<bool>((*first_instance.fields)["d"]), true);

    ASSERT_TRUE(interpreter.env.contains("second"));
    is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("second"));
    ASSERT_TRUE(is_correct_type);
    auto second_instance = as_type<STRUCT_TYPE>(interpreter.env.get("second"));

    ASSERT_TRUE(second_instance.fields->find("a") !=
                second_instance.fields->end());
    ASSERT_TRUE(second_instance.fields->find("b") !=
                second_instance.fields->end());
    ASSERT_TRUE(second_instance.fields->find("c") !=
                second_instance.fields->end());
    ASSERT_TRUE(second_instance.fields->find("d") !=
                second_instance.fields->end());

    ASSERT_TRUE(is_type<int>((*second_instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*second_instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*second_instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*second_instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*second_instance.fields)["a"]), 2);
    ASSERT_EQ(as_type<double>((*second_instance.fields)["b"]), 3.0);
    ASSERT_EQ(as_type<std::string>((*second_instance.fields)["c"]), "world");
    ASSERT_EQ(as_type<bool>((*second_instance.fields)["d"]), false);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, SomeParamsStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct Test { a: int; b: float; c: str; d: bool; };"
                 "var t = Test { a = 1, c = \"hello\" };"
                 "print t.a;"
                 "print t.b;"
                 "print t.c;"
                 "print t.d;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("t"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
    ASSERT_TRUE(instance.fields->find("a") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("b") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("c") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("d") != instance.fields->end());

    ASSERT_TRUE(is_type<int>((*instance.fields)["a"]));
    ASSERT_TRUE(is_type<double>((*instance.fields)["b"]));
    ASSERT_TRUE(is_type<std::string>((*instance.fields)["c"]));
    ASSERT_TRUE(is_type<bool>((*instance.fields)["d"]));

    ASSERT_EQ(as_type<int>((*instance.fields)["a"]), 1);
    ASSERT_EQ(as_type<double>((*instance.fields)["b"]), 0);
    ASSERT_EQ(as_type<std::string>((*instance.fields)["c"]), "hello");
    ASSERT_EQ(as_type<bool>((*instance.fields)["d"]), false);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n0\nhello\nfalse\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, NestedStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct Point { x: int; y: int; };"
                 "struct Line { start: Point; end: Point; };"
                 "var l = Line { start = Point { x = 1, y = 2 }, end = Point { "
                 "x = 3, y = 4 } };"
                 "print l.start.x;"
                 "print l.start.y;"
                 "print l.end.x;"
                 "print l.end.y;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("l"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("l"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("l"));
    ASSERT_TRUE(instance.fields->find("start") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("end") != instance.fields->end());

    auto start_instance = as_type<STRUCT_TYPE>((*instance.fields)["start"]);
    auto end_instance = as_type<STRUCT_TYPE>((*instance.fields)["end"]);

    ASSERT_TRUE(start_instance.fields->find("x") !=
                start_instance.fields->end());
    ASSERT_TRUE(start_instance.fields->find("y") !=
                start_instance.fields->end());
    ASSERT_TRUE(end_instance.fields->find("x") != end_instance.fields->end());
    ASSERT_TRUE(end_instance.fields->find("y") != end_instance.fields->end());

    ASSERT_TRUE(is_type<int>((*start_instance.fields)["x"]));
    ASSERT_TRUE(is_type<int>((*start_instance.fields)["y"]));
    ASSERT_TRUE(is_type<int>((*end_instance.fields)["x"]));
    ASSERT_TRUE(is_type<int>((*end_instance.fields)["y"]));

    ASSERT_EQ(as_type<int>((*start_instance.fields)["x"]), 1);
    ASSERT_EQ(as_type<int>((*start_instance.fields)["y"]), 2);
    ASSERT_EQ(as_type<int>((*end_instance.fields)["x"]), 3);
    ASSERT_EQ(as_type<int>((*end_instance.fields)["y"]), 4);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\n3\n4\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

// TODO: fix segfault
TEST(StructTest, AliasStructInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct Point { x: int; y: int; };"
                 "type P = Point;"
                 "var p = P { x = 1, y = 2 };"
                 "print p.x;"
                 "print p.y;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("p"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("p"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("p"));
    ASSERT_TRUE(instance.fields->find("x") != instance.fields->end());
    ASSERT_TRUE(instance.fields->find("y") != instance.fields->end());

    ASSERT_TRUE(is_type<int>((*instance.fields)["x"]));
    ASSERT_TRUE(is_type<int>((*instance.fields)["y"]));

    ASSERT_EQ(as_type<int>((*instance.fields)["x"]), 1);
    ASSERT_EQ(as_type<int>((*instance.fields)["y"]), 2);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, StructRecursiveInitialization) {
  BirdTest::TestOptions options;
  options.code = "struct A { b: B; };"
                 "struct B { a: A; };"
                 "var a = A { b = B { a = A {} } };"
                 "print a.b.a.b?;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("a"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("a"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("a"));
    ASSERT_TRUE(instance.fields->find("b") != instance.fields->end());

    auto b_instance = as_type<STRUCT_TYPE>((*instance.fields)["b"]);
    ASSERT_TRUE(b_instance.fields->find("a") != b_instance.fields->end());

    auto a_instance = as_type<STRUCT_TYPE>((*b_instance.fields)["a"]);
    ASSERT_TRUE(a_instance.fields->find("b") != a_instance.fields->end());

    ASSERT_TRUE(is_type<std::nullptr_t>((*a_instance.fields)["b"]));
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "false\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, StructWithAdditionalField) {
  BirdTest::TestOptions options;
  options.code = "struct Point { x: int; y: int; };"
                 "var p = Point { x = 1, y = 2, z = 3 };";

  options.after_type_check = [&](UserErrorTracker error_tracker,
                                 TypeChecker &checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto tup = error_tracker.get_errors()[0];

    ASSERT_EQ(std::get<1>(tup).lexeme, "Point");
    ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type error: field \"z\" does not "
                                "exist in struct Point (line 1, character 42)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructTest, StructWithArrayField) {
  BirdTest::TestOptions options;
  options.code = "struct A {"
                 "    a: int[];"
                 "};"

                 "var b: A = A {"
                 "    a = [ 1, 2, 3 ]"
                 "};"

                 "print b.a[0];"
                 "print b.a[1];"
                 "print b.a[2];";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("b"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("b"));
    ASSERT_TRUE(is_correct_type);

    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("b"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>((*instance.fields)["a"]));

    auto array_instance = as_type<ARRAY_TYPE>((*instance.fields)["a"]);
    ASSERT_EQ(array_instance->size(), 3);

    std::vector<int> expected = {1, 2, 3};

    for (int i = 0; i < array_instance->size(); i++) {
      ASSERT_EQ(as_type<int>((*array_instance)[i]), expected[i]);
    }
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n2\n3\n\n", true);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}