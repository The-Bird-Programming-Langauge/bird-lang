#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, NestedNamespaceDirectMemberAccess) {
  BirdTest::TestOptions options;
  options.code = "namespace X {"
                 "  namespace Y {"
                 "    namespace Z {"
                 "      struct Deep { msg: str; };"
                 "    }"
                 "    var deep = Z::Deep { msg = \"deep\" };"
                 "  }"
                 "}"
                 "print X::Y::deep.msg;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "deep\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}