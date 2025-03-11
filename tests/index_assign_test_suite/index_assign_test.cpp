#include "../helpers/compile_helper.hpp"

#define STRUCT_TYPE std::shared_ptr<std::unordered_map<std::string, Value>>
#define ARRAY_TYPE std::shared_ptr<std::vector<Value>>

TEST(IndexAssignTests, IndexAssignWithTypeInt)
{
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [ 0 ];"
                 "x[0] = 1;"
                 "print x[0];";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->size(), 1);

    ASSERT_EQ(
        as_type<int>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(0)),
        1);
  };
  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "1\n\n");
  };

  BirdTest::compile(options);
}

TEST(IndexAssignTests, IndexAssignWithTypeStr)
{
  BirdTest::TestOptions options;
  options.code = "var x: str[] = [ \"Hello\", \", \", \"World!\" ];"
                 "x[0] = \"Goodbye\";"
                 "print x[0], x[1], x[2];";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->size(), 3);

    ASSERT_EQ(
        as_type<std::string>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(0)),
        "Goodbye");
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "Goodbye, World!\n\n");
  };

  BirdTest::compile(options);
}

TEST(IndexAssignTests, IndexAssignWithTypeFloat)
{
  BirdTest::TestOptions options;
  options.code = "var x: float[] = [ 1.1 ];"
                 "x[0] = 3.14;"
                 "print x[0];";

  options.after_interpret = [&](Interpreter &interpreter)
  {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_EQ(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->size(), 1);

    ASSERT_EQ(
        as_type<double>(as_type<ARRAY_TYPE>(interpreter.env.get("x"))->at(0)),
        3.14);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen)
  {
    ASSERT_EQ(output, "3.14\n\n");
  };

  BirdTest::compile(options);
}

TEST(IndexAssignTests, IncompatibleType)
{
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [ 0 ];"
                 "x[0] = \"str\";";

  options.after_type_check = [](UserErrorTracker &user_error_tracker, TypeChecker &type_checker)
  {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] type mismatch: in assignment (line 1, character 23)");
  };

  BirdTest::compile(options);
}