#include "../helpers/compile_helper.hpp"

// INT
TEST(MiscProgramsTestSuite, BinaryPreorder) {
  BirdTest::TestOptions options;
  options.code = "struct Node { val: int; left: Node; right: Node; };\n"
                 "const tree : Node = Node{val = 1,\n"
                 "                     left = Node{val = 2,\n"
                 "                                    left = Node{val = 3},\n"
                 "                                    right = Node{val = 4}\n"
                 "                                },\n"
                 "                     right = Node{val = 5,\n"
                 "                                      left = Node{val = 6},\n"
                 "                                      right = Node{val = 7}\n"
                 "                              }\n"
                 "                       };\n"
                 "fn preorder(tree: Node) -> void { if (tree?) { print "
                 "tree.val; preorder(tree.left); preorder(tree.right); } }"
                 "preorder(tree);";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n2\n3\n4\n5\n6\n7\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}