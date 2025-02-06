#include "helpers/compile_helper.hpp"

TEST(PrintTest, PrintBool)
{
    BirdTest::TestOptions options;
    options.code = "print true;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "1\n\n");
    };

    BirdTest::compile(options);
}

TEST(PrintTest, PrintStr)
{
    BirdTest::TestOptions options;
    options.code = "print \"Hello, World!\";";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "Hello, World!\n\n");
    };

    BirdTest::compile(options);
}

TEST(PrintTest, PrintInt)
{
    BirdTest::TestOptions options;
    options.code = "print 42;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "42\n\n");
    };

    BirdTest::compile(options);
}

TEST(PrintTest, PrintFloat)
{
    BirdTest::TestOptions options;
    options.code = "print 42.42;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "42.42\n\n");
    };

    BirdTest::compile(options);
}

TEST(PrintTest, PrintAlias)
{
    BirdTest::TestOptions options;
    options.code = "type num = int; var x: num = 42; print x;";

    options.after_compile = [](std::string &code, CodeGen &code_gen)
    {
        EXPECT_EQ(code, "42\n\n");
    };

    BirdTest::compile(options);
}
