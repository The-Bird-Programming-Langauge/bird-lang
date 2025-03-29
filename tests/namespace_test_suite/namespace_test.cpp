#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, MethodCallFromStructInNamespace) {
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

TEST(Namespaces, MethodReturningSelf) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct B {"
                 "    value: str;"
                 ""
                 "    fn set_value(new: str) -> B {"
                 "      return B { value = new };"
                 "    }"
                 ""
                 "    fn show() {"
                 "      print self.value;"
                 "    }"
                 "  };"
                 ""
                 "  var w: B = B { value = \"A\" };"
                 "}"
                 ""
                 "A::w = A::w.set_value(\"B\");"
                 "A::w.show();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "B\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

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

TEST(Namespaces, MethodReturnFromOuterType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct X {"
                 "    val: int;"
                 "    fn double() -> int { return self.val * 2; }"
                 "  };"
                 "}"
                 "namespace B {"
                 "  var x: A::X = A::X { val = 21 };"
                 "}"
                 "print B::x.double();";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "42\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

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

TEST(Namespaces, InnerMethodReturnOuterType) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct R { r: str; };"
                 "}"
                 "namespace A {"
                 "  namespace B {"
                 "    fn make_r() -> R { return R { r = \"r\" }; }"
                 "    var x: R = make_r();"
                 "  }"
                 "}"
                 "print A::B::x.r;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "r\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(Namespaces, MethodWithParameterOfSameType) {
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

TEST(Namespaces, StructField) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  const string: str = \"A\";"
                 "  struct B { string: str; };"
                 "  var n: B = B { string = string };"
                 "}"
                 "print A::n.string;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "A\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(Namespaces, ReturnTypeFromOuterNamespace) {
  BirdTest::TestOptions options;
  options.code = "namespace A {"
                 "  struct Point { x: int; y: int; };"
                 "}"
                 "namespace B {"
                 "  fn origin() -> A::Point {"
                 "    return A::Point { x = 0, y = 0 };"
                 "  }"
                 "  var p = origin();"
                 "}"
                 "print B::p.x;"
                 "print B::p.y;";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output, "0\n0\n\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
