
#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, StructArrayParamDecorating) {
  BirdTest::TestOptions options;
  options.code = "namespace Points {"
                 "  struct Point { x: int; y: int; };"
                 ""
                 "  fn print_points(points: Point[]) -> void {"
                 "    for var i = 0; i < 2; i += 1 {"
                 "      print \"(\", points[i].x, \",\", points[i].y, \")\";"
                 "    }"
                 "  }"
                 "}"
                 ""
                 "var arr: Points::Point[] = ["
                 "  Points::Point { x = 1, y = 2 },"
                 "  Points::Point { x = 3, y = 4 }"
                 "];"
                 "Points::print_points(arr);";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "(1,2)\n(3,4)\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
