#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, MethodReturningSelf) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct B {"
                 "    value: str;"
                 ""
                 "    fn set_value(new: str) -> B {"
                 "      return B { value = new };"
                 "    }"
                 ""
                 "    fn show() {"
                 "      print self.value;"
                 "    }"
                 "  };"
                 ""
                 "  var w: B = B { value = \"A\" };"
                 "}"
                 ""
                 "A::w = A::w.set_value(\"B\");"
                 "A::w.show();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "B\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}