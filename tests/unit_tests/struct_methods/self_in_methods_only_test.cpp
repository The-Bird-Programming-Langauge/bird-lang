#include "../../helpers/compile_helper.hpp"

TEST(StructMethod, SelfUsedOutSideOfMethod) {
  BirdTest::TestOptions options;
  options.compile = false;
  options.interpret = false;
  options.type_check = false;

  options.code = "print self.name;";

  options.after_semantic_analyze = [&](auto &error_tracker, auto &analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(error_tracker.get_errors().size(), 1);
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] semantic error: Use of self outside of struct member "
              "function. (line 1, character 7)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructMethod, SelfUsedInFunction) {
  BirdTest::TestOptions options;
  options.compile = false;
  options.interpret = false;
  options.type_check = false;

  options.code = "fn foobar() { print self.name; }";

  options.after_semantic_analyze = [&](auto &error_tracker, auto &analyzer) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(error_tracker.get_errors().size(), 1);
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] semantic error: Use of self outside of struct member "
              "function. (line 1, character 21)");
  };

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

  options.after_semantic_analyze = [&](auto &error_tracker, auto &analyzer) {
    ASSERT_FALSE(error_tracker.has_errors());
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
