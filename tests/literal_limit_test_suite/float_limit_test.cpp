#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(LiteralLimitTest, MaxPositiveFloat) {
  BirdTest::TestOptions options;
  options.code = "1.7976931348623158E+308;";
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, OverMaxPositiveFloat) {
  BirdTest::TestOptions options;
  options.code = "1.7976931348623159E+308;";

  options.after_literal_limit_check = [](UserErrorTracker &error_tracker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto error = error_tracker.get_errors()[0];
    ASSERT_EQ(std::get<0>(error),
              ">>[ERROR] semantic error: Float literal out of range (line 1, "
              "character 1)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, MaxNegativeFloat) {
  BirdTest::TestOptions options;
  options.code = "-1.7976931348623158E+308;";
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, UnderMaxNegativeFloat) {
  BirdTest::TestOptions options;
  options.code = "-1.7976931348623159E+308;";

  options.after_literal_limit_check = [](UserErrorTracker &error_tracker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto error = error_tracker.get_errors()[0];
    ASSERT_EQ(std::get<0>(error),
              ">>[ERROR] semantic error: Float literal out of range (line 1, "
              "character 2)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, MinPositiveFloat) {
  BirdTest::TestOptions options;
  options.code = "2.2250738585072014E-308;";
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, UnderMinPositiveFloat) {
  BirdTest::TestOptions options;
  options.code = "2.2250738585072013E-308;";

  options.after_literal_limit_check = [](UserErrorTracker &error_tracker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto error = error_tracker.get_errors()[0];
    ASSERT_EQ(std::get<0>(error),
              ">>[ERROR] semantic error: Float literal out of range (line 1, "
              "character 1)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, MinNegativeFloat) {
  BirdTest::TestOptions options;
  options.code = "-2.2250738585072014E-308;";
  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(LiteralLimitTest, UnderMinNegativeFloat) {
  BirdTest::TestOptions options;
  options.code = "-2.2250738585072013E-308;";

  options.after_literal_limit_check = [](UserErrorTracker &error_tracker) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto error = error_tracker.get_errors()[0];
    ASSERT_EQ(std::get<0>(error),
              ">>[ERROR] semantic error: Float literal out of range (line 1, "
              "character 2)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}
