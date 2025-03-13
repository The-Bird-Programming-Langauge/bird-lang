#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(StructMethods, PrintMethod) {
  BirdTest::TestOptions options;
  options.code = "  struct Person { \
                        name: str;\
                        fn say_name() {\
                            print self.name; \
                        }\
                    };";

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {};

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructMethods, RecursiveMethod) {
  BirdTest::TestOptions options;
  options.code = "struct Person {"
                 "  name: str;"
                 "  count: int;"
                 "  fn say_name() {"
                 "    if self.count <= 0 {return;}"
                 "    print self.name;"
                 "    self.count -= 1;"
                 "    self.say_name();"
                 "  }"
                 "};"
                 "var x = Person{name=\"John\", count=3};"
                 "x.say_name();";

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "John\nJohn\nJohn\n\n", true);
  };
}

TEST(StructMethods, NeighborMethod) {
  BirdTest::TestOptions options;
  options.code = "struct A {"
                 "  val: int;"
                 "  fn get_other_val(x: A) -> int {"
                 "    return x.val;"
                 "  }"
                 "  fn get_val() -> int {"
                 "    return self.get_other_val(self);"
                 "  }"
                 "};"
                 "var x = A{val=1};"
                 "print x.get_val();";

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n\n", true);
  };
}

TEST(StructMethods, InnerOuterTest) {
  BirdTest::TestOptions options;
  options.code = "struct Inner {"
                 "  value: int;"
                 "  fn double() -> int {"
                 "    return self.value * 2;"
                 "  }"
                 "};"
                 "struct Outer {"
                 "  inner: Inner;"
                 "  fn sum_with_extra(extra: int) -> int {"
                 "    return self.inner.double() + extra;"
                 "  }"
                 "};"
                 "var i = Inner{value=5};"
                 "var o = Outer{inner=i};"
                 "print o.sum_with_extra(3);";

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "13\n\n", true);
  };
}

TEST(StructMethods, MethodCallFailsForNonMethodFunction) {
  BirdTest::TestOptions options;
  options.code = "struct Foo {"
                 "  x: int;"
                 "};"
                 "fn print_foo(foo: Foo) {"
                 "  print foo.x;"
                 "}"
                 "var foo = Foo {x = 1};"
                 "foo.print_foo();";

  options.compile = false;
  options.interpret = false;

  options.after_semantic_analyze = [&](auto &error_tracker,
                                       auto &semantic_analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto errors = error_tracker.get_errors();
    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(std::get<0>(errors[0]),
              ">>[ERROR] method 'print_foo' does not exist in type 'Foo'");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructMethods, MethodCallFailsForNonStructType) {
  BirdTest::TestOptions options;
  options.code = "fn print_int(x: int) {"
                 "  print x;"
                 "}"
                 "var x = 0;"
                 "x.print_int();";

  options.compile = false;
  options.interpret = false;

  options.after_semantic_analyze = [&](auto &error_tracker,
                                       auto &semantic_analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto errors = error_tracker.get_errors();
    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(std::get<0>(errors[0]),
              ">>[ERROR] method call 'print_int' does not exist in type 'int'");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}