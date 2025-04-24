#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(ImportTest, RepeatedImport) {
  BirdTest::TestOptions options;
  options.code =
  "import Math::Trig::sin, Math::Trig::sin";

  options.after_import = [](UserErrorTracker &user_error_tracker, ImportVisitor &import_visitor)
  {
    ASSERT_TRUE(user_error_tracker.has_errors());
    ASSERT_EQ(
        std::get<0>(user_error_tracker.get_errors()[0]),
        ">>[ERROR] import error: Import path overrides the following import items that already exists in the global namespace: Math::Trig::sin (line 1, character 37)");
  };

  BirdTest::compile(options);
}