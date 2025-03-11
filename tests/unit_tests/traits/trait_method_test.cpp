#include "../../helpers/compile_helper.hpp"

struct TraitFixtureParams {
  std::string fn;
  bool expected_result;
  std::string expected_error;
};

class TraitTestFixture : public testing::TestWithParam<TraitFixtureParams> {
  void SetUp() override {
    options.compile = false;
    options.interpret = false;

    options.code = "\
        trait Foo {\
        " + GetParam().fn +
                   "};";

    options.after_parse = [&](auto &error_tracker, auto &parser, auto &ast) {
      if (GetParam().expected_result == false) {
        ASSERT_TRUE(error_tracker.has_errors());
        ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
                  GetParam().expected_error);
      }
    };
  }

public:
  BirdTest::TestOptions options;
  void compile() {
    const auto params = GetParam();
    ASSERT_EQ(BirdTest::compile(options), params.expected_result);
  }
};

TEST_P(TraitTestFixture, DeclaringTraits) { compile(); }

INSTANTIATE_TEST_SUITE_P(
    DeclaringTraits, TraitTestFixture,
    ::testing::Values((TraitFixtureParams){"fn foo()", true, ""},
                      (TraitFixtureParams){"fn foo() -> void", true, ""},
                      (TraitFixtureParams){"fn foo() -> str", true, ""},
                      (TraitFixtureParams){"fn foo() -> bool", true, ""},
                      (TraitFixtureParams){"fn foo(x: int) -> int", true, ""},
                      (TraitFixtureParams){"fn foo(x: bool) -> bool", true, ""},
                      (TraitFixtureParams){"fn foo(x: str) -> str", true, ""},
                      (TraitFixtureParams){"fn foo(x: float) -> float", true,
                                           ""}));