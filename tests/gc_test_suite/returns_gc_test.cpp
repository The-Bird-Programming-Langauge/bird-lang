#include "../helpers/compile_helper.hpp"

TEST(GcTest, ReturnArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            fn init_array() -> int[] {\
                return [1,2,3];\
            }\
            const result = init_array();\
            gc();\
            print result[0];\
            print result[1];\
            print result[2];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ReturnString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            fn init_array() -> str {\
                return \"foobar\";\
            }\
            const result = init_array();\
            gc();\
            print result;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "foobar\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ReturnStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            fn init_array() -> Data {\
              return Data {val = 1};\
            }\
            const result = init_array();\
            gc();\
            print result.val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
