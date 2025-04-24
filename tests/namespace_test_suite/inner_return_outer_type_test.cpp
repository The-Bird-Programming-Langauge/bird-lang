#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, InnerReturnOuterType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct R { r: str; };"
                 "}"
                 "namespace A {"
                 "  namespace B {"
                 "    fn make_r() -> R { return R { r = \"r\" }; }"
                 "    var x: R = make_r();"
                 "  }"
                 "}"
                 "print A::B::x.r;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "r\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}