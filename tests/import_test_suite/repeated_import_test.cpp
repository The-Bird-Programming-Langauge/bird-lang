#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(ImportTest, RepeatedImport) {
  BirdTest::TestOptions options;
  options.code =
  "import Math::Trig::sin, Math::Trig::sin";

  ASSERT_THROW(BirdTest::compile(options), BirdException);
}