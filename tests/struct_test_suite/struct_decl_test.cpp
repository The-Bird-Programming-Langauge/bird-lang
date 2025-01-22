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
        ASSERT_EQ(output == "", true);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}
