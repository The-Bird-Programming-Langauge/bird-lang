#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, RecursiveMethodCallInNamespace) {
  BirdTest::TestOptions options;
  options.code = "namespace Loop {"
                 "  struct Counter {"
                 "    val: int;"
                 "    fn count_down() {"
                 "      if self.val <= 0 { return; }"
                 "      print self.val;"
                 "      self.val -= 1;"
                 "      self.count_down();"
                 "    }"
                 "  };"
                 "  var c = Counter { val = 3 };"
                 "}"

                 "Loop::c.count_down();";

  options.interpret = false;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "3\n2\n1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
