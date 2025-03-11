#include "../../helpers/compile_helper.hpp"

TEST(SemanticAnalyzeTrait, DeclareTraitOutsideFunction) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;

  options.code = "trait Foo {\
                    fn foobar() -> void\
                };";

  options.after_semantic_analyze = [&](auto &error_tracker, auto &analyzer) {
    ASSERT_FALSE(error_tracker.has_errors());
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(SemanticAnalyzeTrait, DeclareTraitInsideFunction) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;
  options.code = "\
                fn shoebar() {\
                    trait Foo { fn foobar() -> void };\
                }";

  options.after_parse = [&](auto &error_tracker, auto &parser, auto &stmt) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] syntax error, unexpected trait, expecting } (line 1, "
              "character 51)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}