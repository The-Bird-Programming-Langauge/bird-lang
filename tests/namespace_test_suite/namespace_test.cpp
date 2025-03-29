#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, MethodCallFromStructInNamespace) {
  BirdTest::TestOptions options;
  options.code = "namespace Test {"
                 "  struct Hello  {"
                 "    name: str;"
                 "    fn hello() {"
                 "      print self.name;"
                 "    }"
                 "  };"
                 "  var g = Hello {name = \"hello\"};"
                 "}"

                 "Test::g.hello();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "hello\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(Namespaces, MethodWithParameterOfSameType) {
  BirdTest::TestOptions options;
  options.code = "namespace Data {"
                 "  struct Int {"
                 "    value: int;"
                 "    fn add_self(other: Int) -> int {"
                 "      return self.value + other.value;"
                 "    }"
                 "  };"
                 "  var a = Int { value = 5 };"
                 "  var b = Int { value = 7 };"
                 "}"

                 "print Data::a.add_self(Data::b);";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "12\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

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

TEST(Namespaces, RecursiveMethodCallInNamespace) {
  BirdTest::TestOptions options;
  options.code = "namespace Loop {"
                 "  struct Counter {"
                 "    val: int;"
                 "    fn count_down() {"
                 "      if self.val <= 0 { return; }"
                 "      print self.val;"
                 "      self.val -= 1;"
                 "      self.count_down();"
                 "    }"
                 "  };"
                 "  var c = Counter { val = 3 };"
                 "}"

                 "Loop::c.count_down();";

  options.interpret = false;

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "3\n2\n1\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

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