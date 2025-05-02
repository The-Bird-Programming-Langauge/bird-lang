#include "../helpers/compile_helper.hpp"
#define ARRAY_TYPE std::shared_ptr<std::vector<Value>>

TEST(EmptyArrayTest, EmptyArrayToInt) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [];"
                 "push(x, 1);"
                 "var result: int = x[0];"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<int>(result));
    ASSERT_EQ(as_type<int>(result), 1);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, EmptyArrayToFloat) {
  BirdTest::TestOptions options;
  options.code = "var x: float[] = [];"
                 "push(x, 3.9);"
                 "var result: float = x[0];"
                 "print result;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<double>(result));
    ASSERT_EQ(as_type<double>(result), 3.9);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "3.9\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, EmptyArrayToStruct) {
  BirdTest::TestOptions options;
  options.code = "struct Data { val: int; };"
                 "var x: Data[] = [];"
                 "push(x, Data {val = 3});"
                 "var result: Data = x[0];"
                 "print result.val;";

  options.after_interpret = [&](Interpreter &interpreter) {
    ASSERT_TRUE(interpreter.env.contains("x"));
    ASSERT_TRUE(is_type<ARRAY_TYPE>(interpreter.env.get("x")));
    ASSERT_TRUE(interpreter.env.contains("result"));
    auto result = interpreter.env.get("result");
    ASSERT_TRUE(is_type<Struct>(result));
    ASSERT_EQ(as_type<int>((*as_type<Struct>(result).fields)["val"]), 3);
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, VoidArray) {
  BirdTest::TestOptions options;
  options.code = "var x: void[] = [];";

  options.after_type_check = [&](UserErrorTracker &error_tracker,
                                 TypeChecker &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] type error: cannot declare void type (line 1, "
              "character 5)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, UnknownArrayTypeVar) {
  BirdTest::TestOptions options;
  options.code = "var x = [];";

  options.after_type_check = [&](UserErrorTracker &error_tracker,
                                 TypeChecker &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] type error: when initializing an empty array, the array type must be specified (line 1, character 5)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, UnknownArrayTypeConst) {
  BirdTest::TestOptions options;
  options.code = "const x = [];";

  options.after_type_check = [&](UserErrorTracker &error_tracker,
                                 TypeChecker &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] type error: when initializing an empty array, the array type must be specified (line 1, character 7)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(EmtpyArrayTest, ReturnEmptyArrayInt) {
	BirdTest::TestOptions options;
  options.code = "fn ret_empty_array() -> int[] {return [];}\
									const arr = ret_empty_array();\
									push(arr, 1);\
									push(arr, 2);\
									for i in iter(arr) {\
										print i;\
									}\
									";

	options.after_compile = [&](auto& code, auto& code_gen) {
		ASSERT_EQ(code, "1\n2\n\n");
	};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmtpyArrayTest, ReturnEmptyArrayFloat) {
	BirdTest::TestOptions options;
  options.code = "fn ret_empty_array() -> float[] {return [];}\
									const arr = ret_empty_array();\
									push(arr, 1.5);\
									push(arr, 2.5);\
									for i in iter(arr) {\
										print i;\
									}\
									";

 	options.after_compile = [&](auto& code, auto& code_gen) {
		ASSERT_EQ(code, "1.5\n2.5\n\n");
	};

	ASSERT_TRUE(BirdTest::compile(options));
}


TEST(EmptyArrayTest, PassEmptyArrayInt) {
	BirdTest::TestOptions options;
  options.code = "fn pass_empty_array(arr: int[]) {\
											push(arr, 3);\
											push(arr, 4);\
											for i in iter(arr) {\
												print i;\
											}\
									}\
									pass_empty_array([]);\
									";

	options.after_compile = [&](auto& code, auto& code_gen) {
		ASSERT_EQ(code, "3\n4\n\n");
	};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(EmptyArrayTest, PassEmptyArrayFloat) {
	BirdTest::TestOptions options;
  options.code = "fn pass_empty_array(arr: float[]) {\
											push(arr, 3.5);\
											push(arr, 4.5);\
											for i in iter(arr) {\
												print i;\
											}\
									}\
									pass_empty_array([]);\
									";
	
	options.after_compile = [&](auto& code, auto& code_gen) {
		ASSERT_EQ(code, "3.5\n4.5\n\n");
	};

  ASSERT_TRUE(BirdTest::compile(options));
}
