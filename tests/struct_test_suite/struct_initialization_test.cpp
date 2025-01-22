#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

static void struct_init_helper(Interpreter &interpreter)
{
    ASSERT_TRUE(interpreter.env.contains("p"));
    bool is_correct_type = is_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(interpreter.env.get("p"));
    ASSERT_TRUE(is_correct_type);
    auto instance = as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(interpreter.env.get("p"));
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

    options.after_compile = [&](std::string &output, CodeGen &codegen) {
        ASSERT_EQ(output == "1\n2\n\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
