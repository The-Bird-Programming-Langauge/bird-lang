#include "../helpers/compile_helper.hpp"

TEST(GcTest, IndexAssignArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            var data: int[][] = [[1,2,3]];\
            {\
            data[0] = [4,5,6,7];\
            }\
            gc();\
            const result = data[0];\
            print result[0];\
            print result[1];\
            print result[2];\
            print result[3];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "4\n5\n6\n7\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, IndexAssignString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            var data: str[] = [\"foo\"];\
            {\
            data[0] = \"bar\";\
            }\
            gc();\
            const result = data[0];\
            print data[0];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "bar\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, IndexAssignStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            \
            var data: Data[] = [Data { val = 1}];\
            {\
            data[0] = Data { val = 3 };\
            }\
            gc();\
            const result = data[0];\
            print result.val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
