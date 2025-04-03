#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, ReturnGlobalType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct Char { char: str; };"
                 "}"
                 "namespace B {"
                 "  fn write() -> A::Char {"
                 "    return A::Char { char = \"A\" };"
                 "  }"
                 "  var i = write();"
                 "}"
                 "print B::i.char;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "A\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}