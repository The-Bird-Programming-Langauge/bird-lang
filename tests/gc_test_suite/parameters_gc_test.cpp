#include "../helpers/compile_helper.hpp"

TEST(GcTest, ParameterArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            fn init_array(a: int[]) -> void {\
                return;\
            }\
            var a: int[] = [1,2,3];\
            init_array(a);\
            gc();\
            print a[0];\
            print a[1];\
            print a[2];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ParameterString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            fn init_array(s: str) -> void {\
                return;\
            }\
            const s = \"foobar\";\
            init_array(s);\
            gc();\
            print s;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "foobar\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ParameterStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            fn init_array(d: Data) -> void {\
              return;\
            }\
            const data = Data {val = 1}; \
            init_array(data);\
            gc();\
            print data.val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
