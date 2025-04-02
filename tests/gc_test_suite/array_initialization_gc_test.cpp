#include "../helpers/compile_helper.hpp"

TEST(GcTest, ArrayInitializationArray) {
  BirdTest::TestOptions options;
  options.code = "\
            var data: int[] = [[1,2,3]];\
            data[0] = [4,5,6,7];\
            const result = data[0];\
            print data[0][0];";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ArrayInitializationString) {
  BirdTest::TestOptions options;
  options.code = "\
            var data: str[] = [\"foo\"];\
            data[0] = \"bar\";\
            const result = data[0];\
            print data[0];";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, ArrayInitializationStruct) {
  BirdTest::TestOptions options;
  options.code = "\
            struct Data {\
                val: int;\
            };\
            \
            var data: Data[] = [Data { val = 1}];\
            data[0] = Data { val = 3 };\
            print data[0].val;";

  ASSERT_TRUE(BirdTest::compile(options));
}
