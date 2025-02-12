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
        ASSERT_EQ(as_type<int>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(0)), 1);
        ASSERT_EQ(as_type<int>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(1)), 2);
        ASSERT_EQ(as_type<int>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(2)), 3);
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
        ASSERT_EQ(as_type<double>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(0)), 1.1);
        ASSERT_EQ(as_type<double>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(1)), 2.2);
        ASSERT_EQ(as_type<double>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(2)), 3.3);
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
        ASSERT_EQ(as_type<std::string>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(0)), "hello");
        ASSERT_EQ(as_type<std::string>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(1)), ", ");
        ASSERT_EQ(as_type<std::string>(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("x"))->at(2)), "world!");
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(output.find("hello\n, \nworld!\n\n") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ArrayTestSuite, StructArraySubscript)
{
    BirdTest::TestOptions options;
    options.code = "struct Dog {"
                   "    name: str,"
                   "    age: int"
                   "};"

                   "var marci: Dog = Dog {"
                   "    name = \"marci\","
                   "    age = 5"
                   "};"

                   "var klaus: Dog = Dog {"
                   "    name = \"klaus\","
                   "    age = 10"
                   "};"

                   "var dogs: Dog[] = [ marci, klaus ];"
                   "print dogs[0].name;"
                   "print dogs[0].age;"
                   "print dogs[1].name;"
                   "print dogs[1].age;";

    options.after_interpret = [&](Interpreter &interpreter)
    {
        ASSERT_TRUE(interpreter.type_table.contains("Dog"));

        ASSERT_TRUE(interpreter.env.contains("dogs"));
        ASSERT_EQ(as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("dogs"))->size(), 2);

        ASSERT_EQ(as_type<std::string>(
                      as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
                          as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("dogs"))->at(0))
                          ->at("name")),
                  "marci");

        ASSERT_EQ(as_type<int>(
                      as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
                          as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("dogs"))->at(0))
                          ->at("age")),
                  5);

        ASSERT_EQ(as_type<std::string>(
                      as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
                          as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("dogs"))->at(1))
                          ->at("name")),
                  "klaus");

        ASSERT_EQ(as_type<int>(
                      as_type<std::shared_ptr<std::unordered_map<std::string, Value>>>(
                          as_type<std::shared_ptr<std::vector<Value>>>(interpreter.env.get("dogs"))->at(1))
                          ->at("age")),
                  10);
    };

    options.after_compile = [&](std::string &output, CodeGen &codegen)
    {
        ASSERT_TRUE(output.find("marci\n5\nklaus\n10\n\n") != std::string::npos);
    };

    ASSERT_TRUE(BirdTest::compile(options));
}