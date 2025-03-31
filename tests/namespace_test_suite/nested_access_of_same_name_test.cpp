#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, NestedAccessOfSameName) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  namespace B {"
                 "    var x = 0;"
                 "  }"
                 "}"
                "namespace C {"
                "  namespace A {"
                "    namespace B {"
                "      var x = 1;"
                "    }"
                "  }"
                "  fn print_x() {"
                "     print A::B::x;"
                "  }"
                "}"
                "C::print_x();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}