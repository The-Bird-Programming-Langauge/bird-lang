#include "../../helpers/compile_helper.hpp"

struct UsingSelfTestFixtureParams {
  std::string fn;
  std::string fn_call;
  bool expected_result;
  std::string expected_error;
};

class UsingSelfTestFixture
    : public testing::TestWithParam<UsingSelfTestFixtureParams> {
  void SetUp() override {
    options.compile = false;

    options.code = "\
        struct Foo {\
        val: int; \
        " + GetParam().fn +
                   "}; \
        " + "const bar: Foo = Foo {};" +
                   GetParam().fn_call;

    options.after_type_check = [&](auto &error_tracker, auto &analyzer) {
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

TEST_P(UsingSelfTestFixture, UsingSelf) { compile(); }

INSTANTIATE_TEST_SUITE_P(
    UsingSelf, UsingSelfTestFixture,
    ::testing::Values(
        (UsingSelfTestFixtureParams){"fn foo() { print self.val; }",
                                     "bar.foo();", true, ""},
        (UsingSelfTestFixtureParams){"fn foo() -> int { return self.val; }",
                                     "const result: int = bar.foo();", true,
                                     ""},
        (UsingSelfTestFixtureParams){
            "fn foo() -> str { return self.val; }", "bar.foo();", false,
            ">>[ERROR] type mismatch: in return "
            "statement. Expected string, found int (line 1, character 65)"},
        (UsingSelfTestFixtureParams){"fn foo() -> Foo { return self; }",
                                     "const result: Foo = bar.foo();", true,
                                     ""},
        (UsingSelfTestFixtureParams){"fn foo() -> void { return self; }",
                                     "bar.foo();", false,
                                     ">>[ERROR] type mismatch: in return "
                                     "statement. Expected void, found struct "
                                     "Foo (line 1, character 66)"},
        (UsingSelfTestFixtureParams){"fn foo() -> void { self.val += 1; }",
                                     "bar.foo();", true, ""},
        (UsingSelfTestFixtureParams){
            "fn foo() { self.non_existent; }", "bar.foo();", false,
            ">>[ERROR] type error: field does not exist on struct (line 1, "
            "character 63)"}));
