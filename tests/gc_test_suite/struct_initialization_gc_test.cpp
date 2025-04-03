#include "../helpers/compile_helper.hpp"

TEST(GcTest, MemberAssignArray) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Array {\
                array: int[];\
            };\
            var array: Array = Array { array = [1,2,3] };\
            gc();\
            const result = array.array;\
            print result[0];\
            print result[1];\
            print result[2];\
            ";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "1\n2\n3\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, MemberAssignString) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct String {\
                string: str;\
            };\
            var string: String = String { string = \"foo\" };\
            gc();\
            const result = string.string;\
            print result;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "foo\n\n");
  };
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(GcTest, MemberAssignStruct) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.code = "\
            struct Node {\
                next: Node;\
                val: int;\
            };\
            var node: Node = Node { val = 1, next = Node {val = 2} };\
            gc();\
            const result = node.next;\
            print result.val;";

  options.after_compile = [&](auto output, auto &code_gen) {
    ASSERT_EQ(output, "2\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
