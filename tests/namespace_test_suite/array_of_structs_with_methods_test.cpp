#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, ArrayOfStructsWithMethods) {
  BirdTest::TestOptions options;
  options.code = "namespace Test {"
                 "  struct Bar {"
                 "    x: int;"
                 "    y: int;"
                 "  };"

                 "  namespace Other {"
                 "    struct Bar {"
                 "      x: int;"
                 "      y: int;"

                 "      fn describe() {"
                 "        print self.x;"
                 "        print self.y;"
                 "      }"

                 "      fn add() -> int {"
                 "        return self.x + self.y;"
                 "      }"
                 "    };"
                 "  }"

                 "  var t: Other::Bar = Other::Bar { x = 10, y = 20 };"
                 "  var u: Other::Bar = Other::Bar { x = 30, y = 90 };"
                 "  var z: Other::Bar[] = [ t, u ];"
                 "}"

                 "Test::t.describe();"
                 "print \"arr[0]\";"
                 "Test::z[0].describe();"
                 "print Test::z[0].add();"
                 "print \"arr[1]\";"
                 "Test::z[1].describe();"
                 "print Test::z[1].add();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "10\n"
                      "20\n"
                      "arr[0]\n"
                      "10\n"
                      "20\n"
                      "30\n"
                      "arr[1]\n"
                      "30\n"
                      "90\n"
                      "120\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}