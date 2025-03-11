#include "../../helpers/compile_helper.hpp"

TEST(StructImplsTrait, StructCanImplTrait) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;
  options.code = "\
                    trait Foo { fn foobar() -> void };\
                    struct Bar implements Foo {\
                        fn foobar() -> void {} \
                    };\
                    ";

  options.after_parse = [&](auto &error_tracker, auto &parser, auto &stmt) {
    ASSERT_FALSE(error_tracker.has_errors());
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(StructImplsTrait, ErrorWhenStructImproperlyImplsTrait) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;
  options.code = "\
                    trait Foo { fn foobar() -> void };\
                    struct Bar implements Foo {\
                    };\
                    ";

  options.after_type_check = [&](auto &error_tracker, auto &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] type error: struct Bar incorrectly implements trait "
              "(line 1, character 82)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(StructImplsTrait, ErrorWhenStructImplsFnWithWrongTypes) {
  BirdTest::TestOptions options;
  options.interpret = false;
  options.compile = false;
  options.code = "\
                    trait Foo { fn foobar() -> void };\
                    struct Bar implements Foo {\
                        fn foobar() -> int {} \
                    };\
                    ";

  options.after_type_check = [&](auto &error_tracker, auto &type_checker) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
              ">>[ERROR] type error: struct Bar incorrectly implements trait "
              "(line 1, character 82)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}