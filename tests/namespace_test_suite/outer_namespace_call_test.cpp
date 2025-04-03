#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, OuterNamespaceCall) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  fn get() -> str {"
                 "    return \"A\";"
                 "  }"
                 "  namespace B {"
                 "    var response: str = get();"
                 "  }"
                 "}"
                 "print A::B::response;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "A\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}