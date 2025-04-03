#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, ReturnGlobalType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct Char { c: str; };"
                 "}"
                 "namespace B {"
                 "  fn write() -> A::Char {"
                 "    return A::Char { c = \"A\" };"
                 "  }"
                 "  var i = write();"
                 "}"
                 "print B::i.c;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "A\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}