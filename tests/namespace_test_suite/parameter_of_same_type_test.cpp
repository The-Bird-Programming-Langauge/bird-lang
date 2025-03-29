#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, ParameterOfSameType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct Int {"
                 "    value: int;"
                 "    fn add_self(other: Int) -> int {"
                 "      return self.value + other.value;"
                 "    }"
                 "  };"
                 "  var a = Int { value = 5 };"
                 "  var b = Int { value = 7 };"
                 "}"

                 "print A::a.add_self(A::b);";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "12\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}