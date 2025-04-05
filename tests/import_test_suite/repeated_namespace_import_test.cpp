#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(ImportTest, RepeatedNamespaceImport) {
  BirdTest::TestOptions options;
  options.code =
  "import Math, Math";

  ASSERT_THROW(BirdTest::compile(options), BirdException);
}