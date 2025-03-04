#include "../../helpers/compile_helper.hpp"

TEST(StructMethod, SelfUsedOutSideOfMethod) {
  BirdTest::TestOptions options;
  options.compile = false;
  options.interpret = false;
  options.type_check = false;

  options.code = "print self.name;";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructMethod, SelfUsedInFunction) {
  BirdTest::TestOptions options;
  options.compile = false;
  options.interpret = false;
  options.type_check = false;

  options.code = "fn foobar() { print self.name; }";

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructMethod, SelfUsedInMethod) {
  BirdTest::TestOptions options;
  options.compile = false;
  options.interpret = false;
  options.type_check = false;

  options.code = "struct Person {\
                    name: int; \
                    fn foobar() { \
                        print self.name; \
                    }\
                };";

  ASSERT_TRUE(BirdTest::compile(options));
}