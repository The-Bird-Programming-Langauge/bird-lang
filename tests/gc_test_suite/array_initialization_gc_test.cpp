#include "../helpers/compile_helper.hpp"

TEST(GcTest, ArrayInitializationArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            fn init_array() -> int[][] {\
              var second: int[] = [1,2,3];\
              var data: int[][] = [[4,5,6,7], second];\
              return data;\
            }\
            const arr = init_array();\
            gc();\
            const result = arr[1];\
            print result[0];\
            print result[1];\
            print result[2];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ArrayInitializationString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            fn init_array() -> str[] {\
              var second: str = \", world!\";\
              var data: str[] = [\"hello\", second];\
              return data;\
            }\
            const result = init_array();\
            gc();\
            print result[0] + result[1];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "hello, world!\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ArrayInitializationStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            fn init_array() -> Data[] {\
              var second: Data = Data { val = 3 };\
              var data: Data[] = [Data { val = 4 }, second];\
              return data;\
            }\
            const result = init_array();\
            gc();\
            print result[1].val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
