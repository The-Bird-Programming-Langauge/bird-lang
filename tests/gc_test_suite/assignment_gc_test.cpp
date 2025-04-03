#include "../helpers/compile_helper.hpp"

TEST(GcTest, AssignmentArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            var data: int[] = [1,2,3];\
            {\
            data = [4,5,6,7];\
            }\
            gc();\
            print data[0];\
            print data[1];\
            print data[2];\
            print data[3];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "4\n5\n6\n7\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, AssignmentString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            var data: str = \"foo\";\
            {\
            data = \"bar\";\
            }\
            gc();\
            print data;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "bar\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, AssignmentStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            \
            var data: Data = Data { val = 1};\
            {\
            data = Data { val = 3 };\
            }\
            gc();\
            print data.val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
