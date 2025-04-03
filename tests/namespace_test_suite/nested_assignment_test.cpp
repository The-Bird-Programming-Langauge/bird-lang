#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, NestedAssignment) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  namespace B {"
                 "    namespace C {"
                 "      struct Data {"
                 "        value: int;"
                 "      };"
                 "      var d = Data { value = 1 };"
                 "    }"
                 "  }"
                 "}"
                 "A::B::C::d.value = 99;"
                 "print A::B::C::d.value;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "99\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}