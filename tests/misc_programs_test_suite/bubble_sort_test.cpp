#include "../helpers/compile_helper.hpp"

TEST(MiscProgramsTestSuite, BubbleSort) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [ 5, 3, 2, 6, 4, 1, 9, 7, 8 ];"
                 "for var i: int = 0; i < 9 - 1; i += 1 {"
                 "    for var j: int = 0; j < 9 - 1 - i; j += 1 {"
                 "        if (x[j] > x[j + 1]) {"
                 "            var temp: int = x[j];"
                 "            x[j] = x[j + 1];"
                 "            x[j + 1] = temp;"
                 "        }"
                 "    }"
                 "}"
                 "for var i: int = 0; i < 9; i += 1 {"
                 "    print x[i];"
                 "}";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n2\n3\n4\n5\n6\n7\n8\n9\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
