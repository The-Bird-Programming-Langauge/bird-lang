#include "../../helpers/compile_helper.hpp"

struct MethodReturnTestFixtureParams {
  std::string function;
  bool expected_result;
  std::string expected_error;
};

class MethodReturnTestFixture
    : public testing::TestWithParam<MethodReturnTestFixtureParams> {
public:
  BirdTest::TestOptions options;
  void setUp() {
    options.compile = false;
    options.interpret = false;
    options.code = "\
                struct Foo {" +
                   GetParam().function + "}; \
                   const foo: Foo = Foo {}; \
                   foo.foo();\
                   ";

    options.after_type_check = [&](auto &error_tracker, auto &analyzer) {
      if (GetParam().expected_result == false) {
        ASSERT_TRUE(error_tracker.has_errors());
        ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
                  GetParam().expected_error);
      }
    };
  }

  void compile() {
    const auto params = GetParam();
    ASSERT_EQ(BirdTest::compile(options), params.expected_result);
  }
};

TEST_P(MethodReturnTestFixture, StructMethodReturn) {
  setUp();
  compile();
}

INSTANTIATE_TEST_SUITE_P(
    StructMethodReturn, MethodReturnTestFixture,
    ::testing::Values(
        (MethodReturnTestFixtureParams){"fn foo() -> void {}", true, ""},
        (MethodReturnTestFixtureParams){"fn foo() {}", true, ""},
        (MethodReturnTestFixtureParams){"fn foo() {return 3;}", false,
                                        ">>[ERROR] type mismatch: in return "
                                        "statement (line 1, character 39)"},
        (MethodReturnTestFixtureParams){"fn foo() -> void {return 3;}", false,
                                        ">>[ERROR] type mismatch: in return "
                                        "statement (line 1, character 47)"},
        (MethodReturnTestFixtureParams){
            "fn foo() -> int {}", false,
            ">>[ERROR] type error: Missing return in a non-void function "
            "'foo.' (line 1, character 32)"},
        (MethodReturnTestFixtureParams){"fn foo() -> int {return 3;}", true,
                                        ""},
        (MethodReturnTestFixtureParams){"fn foo() -> int {return true;}", false,
                                        ">>[ERROR] type mismatch: in return "
                                        "statement (line 1, character 46)"}));
