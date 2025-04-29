#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(LiteralLimitTest, MaxInt) {
  BirdTest::TestOptions options;
  options.code = "2147483647;";
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, OverMaxInt) {
  BirdTest::TestOptions options;
  options.code = "2147483648;";

  options.after_literal_limit_check = [](UserErrorTracker &error_tracker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto error = error_tracker.get_errors()[0];
    ASSERT_EQ(std::get<0>(error),
              ">>[ERROR] semantic error: Int literal out of range (line 1, "
              "character 1)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, MinInt) {
  BirdTest::TestOptions options;
  options.code = "-2147483647;";

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, UnderMinInt) {
  BirdTest::TestOptions options;
  options.code = "-2147483648;";

  options.after_literal_limit_check = [](UserErrorTracker &error_tracker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto error = error_tracker.get_errors()[0];
    ASSERT_EQ(std::get<0>(error),
              ">>[ERROR] semantic error: Int literal out of range (line 1, "
              "character 2)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}