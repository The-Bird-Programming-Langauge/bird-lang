#include "../helpers/compile_helper.hpp"

TEST(GcTest, VarArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            var foo: int[] = [1,2,3];\
            var data = foo;\
            gc();\
            print data[0];\
            print data[1];\
            print data[2];";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, VarString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            var foo: str = \"bar\";\
            var data = foo;\
            gc();\
            print data;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "bar\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, VarStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            \
            var foo: Data = Data { val = 3};\
            var data = foo;\
            gc();\
            print data.val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
