#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, NestedGlobalScope) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct Foo {"
                 "   a : str;"
                 "  };"
                 "}"

                 "namespace B {"
                 "  struct Bar {"
                 "    b : str;"
                 "  };"

                 "  namespace C {"
                 "    struct Shoe {"
                 "      c : str;"
                 "    };"
                 "  }"

                 "  const a : A::Foo = A::Foo{a = \"a\"};"
                 "}"

                 "namespace A {"
                 "  var d: Foo = Foo { a = \"a\" };"
                 "  var b: B::Bar = B::Bar { b = \"b\" };"
                 "  var c: B::C::Shoe = B::C::Shoe { c = \"c\" };"
                 "}"

                 "print B::a.a;"
                 "print A::b.b;"
                 "print A::c.c;";

  options.interpret = false;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "a\nb\nc\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}