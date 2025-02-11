#include "helpers/compile_helper.hpp"

TEST(ArrayTestSuite, IntArraySubscript)
{
    BirdTest::TestOptions options;
    options.code = "var x: int[] = [ 1, 2, 3 ];"
                   "print x[0];"
                   "print x[1];"
                   "print x[2];";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")));

        ASSERT_EQ(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->size(), 3);
        ASSERT_EQ(as_type<int>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[0]), 1);
        ASSERT_EQ(as_type<int>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[1]), 2);
        ASSERT_EQ(as_type<int>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[2]), 3);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(output.find("1\n2\n3\n\n") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayTestSuite, FloatArraySubscript)
{
    BirdTest::TestOptions options;
    options.code = "var x: float[] = [ 1.1, 2.2, 3.3 ];"
                   "print x[0];"
                   "print x[1];"
                   "print x[2];";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")));

        ASSERT_EQ(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->size(), 3);
        ASSERT_EQ(as_type<double>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[0]), 1.1);
        ASSERT_EQ(as_type<double>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[1]), 2.2);
        ASSERT_EQ(as_type<double>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[2]), 3.3);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(output.find("1.1\n2.2\n3.3\n\n") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayTestSuite, StrArraySubscript)
{
    BirdTest::TestOptions options;
    options.code = "var x: str[] = [ \"hello\", \", \", \"world!\" ];"
                   "print x[0];"
                   "print x[1];"
                   "print x[2];";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.env.contains("x"));
        ASSERT_TRUE(is_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")));

        ASSERT_EQ(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->size(), 3);
        ASSERT_EQ(as_type<std::string>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[0]), "hello");
        ASSERT_EQ(as_type<std::string>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[1]), ", ");
        ASSERT_EQ(as_type<std::string>((*as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x")))[2]), "world!");
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(output.find("hello\n, \nworld!\n\n") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}