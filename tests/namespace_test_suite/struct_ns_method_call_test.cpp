#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, StructNsMethodCall) {
  BirdTest::TestOptions options;
  options.code = "namespace Test {"
                 "  struct Hello  {"
                 "    say_hi: str;"
                 "    fn hello() {"
                 "      print self.say_hi;"
                 "    }"
                 "  };"
                 "  var g = Hello {say_hi = \"hello\"};"
                 "}"

                 "Test::g.hello();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "hello\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}