#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

#define STRUCT_TYPE std::shared_ptr<std::unordered_map<std::string, Value>>

static void struct_init_helper(Interpreter &interpreter)
{
    ASSERT_TRUE(interpreter.env.contains("p"));
    bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("p"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("p"));
    ASSERT_TRUE(instance->find("x") != instance->end());
    ASSERT_TRUE(instance->find("y") != instance->end());

    ASSERT_TRUE(is_type<int>((*instance)["x"]));
    ASSERT_TRUE(is_type<int>((*instance)["y"]));

    ASSERT_EQ(as_type<int>((*instance)["x"]), 1);
    ASSERT_EQ(as_type<int>((*instance)["y"]), 2);
}

TEST(StructTest, VarStructInitializationTest)
{
    BirdTest::TestOptions options;
    options.code = "struct Point { x: int, y: int };"
                   "var p = Point { x = 1, y = 2 };"
                   "print p.x;"
                   "print p.y;";

    options.after_interpret = struct_init_helper;

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, ConstStructInitializationTest)
{
    BirdTest::TestOptions options;
    options.code = "struct Point { x: int, y: int };"
                   "const p = Point { x = 1, y = 2 };"
                   "print p.x;"
                   "print p.y;";

    options.after_interpret = struct_init_helper;

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, FunctionStructInitializationTest)
{
    BirdTest::TestOptions options;
    options.code = "struct Point { x: int, y: int };"
                   "fn get_point() -> Point { return Point { x = 1, y = 2 }; }"
                   "var p = get_point();"
                   "print get_point().x;"
                   "print get_point().y;";

    options.after_interpret = struct_init_helper;

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, AllPrimitiveTypesStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Test { a: int, b: float, c: str, d: bool };"
                   "var t = Test { a = 1, b = 2.0, c = \"hello\", d = true };"
                   "print t.a;"
                   "print t.b;"
                   "print t.c;"
                   "print t.d;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("t"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(instance->find("a") != instance->end());
        ASSERT_TRUE(instance->find("b") != instance->end());
        ASSERT_TRUE(instance->find("c") != instance->end());
        ASSERT_TRUE(instance->find("d") != instance->end());

        ASSERT_TRUE(is_type<int>((*instance)["a"]));
        ASSERT_TRUE(is_type<double>((*instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*instance)["d"]));

        ASSERT_EQ(as_type<int>((*instance)["a"]), 1);
        ASSERT_EQ(as_type<double>((*instance)["b"]), 2.0);
        ASSERT_EQ(as_type<std::string>((*instance)["c"]), "hello");
        ASSERT_EQ(as_type<bool>((*instance)["d"]), true);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\nhello\n1\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, HoistedStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "var t = Test { a = 1, b = 2.0, c = \"hello\", d = true };"
                   "print t.a;"
                   "print t.b;"
                   "print t.c;"
                   "print t.d;"
                   "struct Test { a: int, b: float, c: str, d: bool };";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("t"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(instance->find("a") != instance->end());
        ASSERT_TRUE(instance->find("b") != instance->end());
        ASSERT_TRUE(instance->find("c") != instance->end());
        ASSERT_TRUE(instance->find("d") != instance->end());

        ASSERT_TRUE(is_type<int>((*instance)["a"]));
        ASSERT_TRUE(is_type<double>((*instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*instance)["d"]));

        ASSERT_EQ(as_type<int>((*instance)["a"]), 1);
        ASSERT_EQ(as_type<double>((*instance)["b"]), 2.0);
        ASSERT_EQ(as_type<std::string>((*instance)["c"]), "hello");
        ASSERT_EQ(as_type<bool>((*instance)["d"]), true);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\nhello\n1\n\n", true);
    };

    ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructTest, ParamsOutOfOrderStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Test { a: int, b: float, c: str, d: bool };"
                   "var t = Test { b = 2.0, a = 1, d = true, c = \"hello\" };"
                   "print t.a;"
                   "print t.b;"
                   "print t.c;"
                   "print t.d;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("t"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(instance->find("a") != instance->end());
        ASSERT_TRUE(instance->find("b") != instance->end());
        ASSERT_TRUE(instance->find("c") != instance->end());
        ASSERT_TRUE(instance->find("d") != instance->end());

        ASSERT_TRUE(is_type<int>((*instance)["a"]));
        ASSERT_TRUE(is_type<double>((*instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*instance)["d"]));

        ASSERT_EQ(as_type<int>((*instance)["a"]), 1);
        ASSERT_EQ(as_type<double>((*instance)["b"]), 2.0);
        ASSERT_EQ(as_type<std::string>((*instance)["c"]), "hello");
        ASSERT_EQ(as_type<bool>((*instance)["d"]), true);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\nhello\n1\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, NoParamsStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Test { a: int, b: float, c: str, d: bool };"
                   "var t = Test {};"
                   "print t.a;"
                   "print t.b;"
                   "print t.c;"
                   "print t.d;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("t"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(instance->find("a") != instance->end());
        ASSERT_TRUE(instance->find("b") != instance->end());
        ASSERT_TRUE(instance->find("c") != instance->end());
        ASSERT_TRUE(instance->find("d") != instance->end());

        ASSERT_TRUE(is_type<int>((*instance)["a"]));
        ASSERT_TRUE(is_type<double>((*instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*instance)["d"]));

        ASSERT_EQ(as_type<int>((*instance)["a"]), 0);
        ASSERT_EQ(as_type<double>((*instance)["b"]), 0);
        ASSERT_EQ(as_type<std::string>((*instance)["c"]), "");
        ASSERT_EQ(as_type<bool>((*instance)["d"]), false);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "0\n0\n\n0\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, MultipleStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Test { a: int, b: float, c: str, d: bool };"
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

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("first"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("first"));
        ASSERT_TRUE(is_correct_type);
        auto first_instance = as_type<STRUCT_TYPE>(interpreter.env.get("first"));
        ASSERT_TRUE(first_instance->find("a") != first_instance->end());
        ASSERT_TRUE(first_instance->find("b") != first_instance->end());
        ASSERT_TRUE(first_instance->find("c") != first_instance->end());
        ASSERT_TRUE(first_instance->find("d") != first_instance->end());

        ASSERT_TRUE(is_type<int>((*first_instance)["a"]));
        ASSERT_TRUE(is_type<double>((*first_instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*first_instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*first_instance)["d"]));

        ASSERT_EQ(as_type<int>((*first_instance)["a"]), 1);
        ASSERT_EQ(as_type<double>((*first_instance)["b"]), 2.0);
        ASSERT_EQ(as_type<std::string>((*first_instance)["c"]), "hello");
        ASSERT_EQ(as_type<bool>((*first_instance)["d"]), true);

        ASSERT_TRUE(interpreter.env.contains("second"));
        is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("second"));
        ASSERT_TRUE(is_correct_type);
        auto second_instance = as_type<STRUCT_TYPE>(interpreter.env.get("second"));

        ASSERT_TRUE(second_instance->find("a") != second_instance->end());
        ASSERT_TRUE(second_instance->find("b") != second_instance->end());
        ASSERT_TRUE(second_instance->find("c") != second_instance->end());
        ASSERT_TRUE(second_instance->find("d") != second_instance->end());

        ASSERT_TRUE(is_type<int>((*second_instance)["a"]));
        ASSERT_TRUE(is_type<double>((*second_instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*second_instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*second_instance)["d"]));

        ASSERT_EQ(as_type<int>((*second_instance)["a"]), 2);
        ASSERT_EQ(as_type<double>((*second_instance)["b"]), 3.0);
        ASSERT_EQ(as_type<std::string>((*second_instance)["c"]), "world");
        ASSERT_EQ(as_type<bool>((*second_instance)["d"]), false);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen) {};

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, SomeParamsStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Test { a: int, b: float, c: str, d: bool };"
                   "var t = Test { a = 1, c = \"hello\" };"
                   "print t.a;"
                   "print t.b;"
                   "print t.c;"
                   "print t.d;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("t"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("t"));
        ASSERT_TRUE(instance->find("a") != instance->end());
        ASSERT_TRUE(instance->find("b") != instance->end());
        ASSERT_TRUE(instance->find("c") != instance->end());
        ASSERT_TRUE(instance->find("d") != instance->end());

        ASSERT_TRUE(is_type<int>((*instance)["a"]));
        ASSERT_TRUE(is_type<double>((*instance)["b"]));
        ASSERT_TRUE(is_type<std::string>((*instance)["c"]));
        ASSERT_TRUE(is_type<bool>((*instance)["d"]));

        ASSERT_EQ(as_type<int>((*instance)["a"]), 1);
        ASSERT_EQ(as_type<double>((*instance)["b"]), 0);
        ASSERT_EQ(as_type<std::string>((*instance)["c"]), "hello");
        ASSERT_EQ(as_type<bool>((*instance)["d"]), false);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n0\nhello\n0\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, NestedStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Point { x: int, y: int };"
                   "struct Line { start: Point, end: Point };"
                   "var l = Line { start = Point { x = 1, y = 2 }, end = Point { x = 3, y = 4 } };"
                   "print l.start.x;"
                   "print l.start.y;"
                   "print l.end.x;"
                   "print l.end.y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("l"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("l"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("l"));
        ASSERT_TRUE(instance->find("start") != instance->end());
        ASSERT_TRUE(instance->find("end") != instance->end());

        auto start_instance = as_type<STRUCT_TYPE>((*instance)["start"]);
        auto end_instance = as_type<STRUCT_TYPE>((*instance)["end"]);

        ASSERT_TRUE(start_instance->find("x") != start_instance->end());
        ASSERT_TRUE(start_instance->find("y") != start_instance->end());
        ASSERT_TRUE(end_instance->find("x") != end_instance->end());
        ASSERT_TRUE(end_instance->find("y") != end_instance->end());

        ASSERT_TRUE(is_type<int>((*start_instance)["x"]));
        ASSERT_TRUE(is_type<int>((*start_instance)["y"]));
        ASSERT_TRUE(is_type<int>((*end_instance)["x"]));
        ASSERT_TRUE(is_type<int>((*end_instance)["y"]));

        ASSERT_EQ(as_type<int>((*start_instance)["x"]), 1);
        ASSERT_EQ(as_type<int>((*start_instance)["y"]), 2);
        ASSERT_EQ(as_type<int>((*end_instance)["x"]), 3);
        ASSERT_EQ(as_type<int>((*end_instance)["y"]), 4);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\n3\n4\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

// TODO: fix segfault
TEST(StructTest, AliasStructInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct Point { x: int, y: int };"
                   "type P = Point;"
                   "var p = P { x = 1, y = 2 };"
                   "print p.x;"
                   "print p.y;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("p"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("p"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("p"));
        ASSERT_TRUE(instance->find("x") != instance->end());
        ASSERT_TRUE(instance->find("y") != instance->end());

        ASSERT_TRUE(is_type<int>((*instance)["x"]));
        ASSERT_TRUE(is_type<int>((*instance)["y"]));

        ASSERT_EQ(as_type<int>((*instance)["x"]), 1);
        ASSERT_EQ(as_type<int>((*instance)["y"]), 2);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "1\n2\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, StructRecursiveInitialization)
{
    BirdTest::TestOptions options;
    options.code = "struct A { b: B };"
                   "struct B { a: A };"
                   "var a = A { b = B { a = A {} } };"
                   "print a.b.a.b?;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("a"));
        bool is_correct_type = is_type<STRUCT_TYPE>(interpreter.env.get("a"));
        ASSERT_TRUE(is_correct_type);
        auto instance = as_type<STRUCT_TYPE>(interpreter.env.get("a"));
        ASSERT_TRUE(instance->find("b") != instance->end());

        auto b_instance = as_type<STRUCT_TYPE>((*instance)["b"]);
        ASSERT_TRUE(b_instance->find("a") != b_instance->end());

        auto a_instance = as_type<STRUCT_TYPE>((*b_instance)["a"]);
        ASSERT_TRUE(a_instance->find("b") != a_instance->end());

        ASSERT_TRUE(is_type<std::nullptr_t>((*a_instance)["b"]));
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "0\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, StructWithAdditionalField)
{
    BirdTest::TestOptions options;
    options.code = "struct Point { x: int, y: int };"
                   "var p = Point { x = 1, y = 2, z = 3 };";

    options.after_type_check = [&](UserErrorTracker error_tracker, TypeChecker &checker)
    {
        ASSERT_TRUE(error_tracker.has_errors());
        auto tup = error_tracker.get_errors()[0];

        ASSERT_EQ(std::get<1>(tup).lexeme, "Point");
        ASSERT_EQ(std::get<0>(tup), ">>[ERROR] type error: field \"z\" does not exist in struct Point (line 1, character 41)");
    };

    ASSERT_FALSE(BirdTest::compile(options));
}
