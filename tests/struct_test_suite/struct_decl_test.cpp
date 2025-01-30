#include <gtest/gtest.h>
#include "helpers/compile_helper.hpp"

TEST(StructTest, StructDeclTest)
{
    BirdTest::TestOptions options;
    options.code = "struct Test { a: int, b: float, c: str, d: bool };";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_EQ(interpreter.type_table.contains("Test"), true);
        auto type = interpreter.type_table.get("Test");
        ASSERT_EQ(type->type, BirdTypeType::STRUCT);

        auto struct_type = std::dynamic_pointer_cast<StructType>(type);

        ASSERT_EQ(struct_type->fields.size(), 4);

        ASSERT_EQ(struct_type->fields[0].first, "a");
        ASSERT_EQ(struct_type->fields[0].second->type, BirdTypeType::INT);

        ASSERT_EQ(struct_type->fields[1].first, "b");
        ASSERT_EQ(struct_type->fields[1].second->type, BirdTypeType::FLOAT);

        ASSERT_EQ(struct_type->fields[2].first, "c");
        ASSERT_EQ(struct_type->fields[2].second->type, BirdTypeType::STRING);

        ASSERT_EQ(struct_type->fields[3].first, "d");
        ASSERT_EQ(struct_type->fields[3].second->type, BirdTypeType::BOOL);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, StructNestedTest)
{
    BirdTest::TestOptions options;
    options.code = " struct Third { a: int, b: float, c: str, d: bool }; struct Second { third: Third }; struct First { second: Second };";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_EQ(interpreter.type_table.contains("First"), true);
        auto type = interpreter.type_table.get("First");
        ASSERT_EQ(type->type, BirdTypeType::STRUCT);

        auto struct_type = std::dynamic_pointer_cast<StructType>(type);

        ASSERT_EQ(struct_type->fields.size(), 1);
        ASSERT_EQ(struct_type->fields[0].first, "second");

        auto second_type = struct_type->fields[0].second;
        ASSERT_EQ(second_type->type, BirdTypeType::STRUCT);

        auto second_struct_type = std::dynamic_pointer_cast<StructType>(second_type);

        ASSERT_EQ(second_struct_type->fields.size(), 1);
        ASSERT_EQ(second_struct_type->fields[0].first, "third");

        auto third_type = second_struct_type->fields[0].second;
        ASSERT_EQ(third_type->type, BirdTypeType::STRUCT);

        auto third_struct_type = std::dynamic_pointer_cast<StructType>(third_type);

        ASSERT_EQ(third_struct_type->fields.size(), 4);

        ASSERT_EQ(third_struct_type->fields[0].first, "a");
        ASSERT_EQ(third_struct_type->fields[0].second->type, BirdTypeType::INT);

        ASSERT_EQ(third_struct_type->fields[1].first, "b");
        ASSERT_EQ(third_struct_type->fields[1].second->type, BirdTypeType::FLOAT);

        ASSERT_EQ(third_struct_type->fields[2].first, "c");
        ASSERT_EQ(third_struct_type->fields[2].second->type, BirdTypeType::STRING);

        ASSERT_EQ(third_struct_type->fields[3].first, "d");
        ASSERT_EQ(third_struct_type->fields[3].second->type, BirdTypeType::BOOL);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructTest, StructRecursiveTest)
{
    BirdTest::TestOptions options;
    options.code = "struct Recursive { a: int, b: Recursive };";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_EQ(interpreter.type_table.contains("Recursive"), true);
        auto type = interpreter.type_table.get("Recursive");
        ASSERT_EQ(type->type, BirdTypeType::STRUCT);

        auto struct_type = std::dynamic_pointer_cast<StructType>(type);

        ASSERT_EQ(struct_type->fields.size(), 2);

        ASSERT_EQ(struct_type->fields[0].first, "a");
        ASSERT_EQ(struct_type->fields[0].second->type, BirdTypeType::INT);

        ASSERT_EQ(struct_type->fields[1].first, "b");
        ASSERT_EQ(struct_type->fields[1].second->type, BirdTypeType::PLACEHOLDER);

        auto inner_struct_type = std::dynamic_pointer_cast<PlaceholderType>(struct_type->fields[1].second);

        ASSERT_EQ(inner_struct_type->name, "Recursive");
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_EQ(output == "\n", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
