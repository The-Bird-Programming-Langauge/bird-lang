#include "../helpers/compile_helper.hpp"

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
  options.code =
  "struct Person {"
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
  "x.say_name();"
  ;

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "John\nJohn\nJohn\n\n", true);
  };
}

TEST(StructMethods, NeighborMethod) {
  BirdTest::TestOptions options;
  options.code =
  "struct A {"
  "  val: int;"
  "  fn get_other_val(x: A) -> int {"
  "    return x.val;"
  "  }"
  "  fn get_val() -> int {"
  "    return self.get_other_val(self);"
  "  }"
  "};"
  "var x = A{val=1};"
  "print x.get_val();"
  ;

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "1\n\n", true);
  };
}

TEST(StructMethods, InnerOuterTest) {
  BirdTest::TestOptions options;
  options.code =
  "struct Inner {"
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
  "print o.sum_with_extra(3);"
  ;

  options.after_interpret = [&](Interpreter &interpreter) {};

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_EQ(output == "13\n\n", true);
  };
}