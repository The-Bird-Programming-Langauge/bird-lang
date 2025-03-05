#include "../../helpers/compile_helper.hpp"

struct MethodParamCountTestFixtureParams {
  std::string params;
  bool expected_result;
};

class MethodParamCountTestFixture
    : public testing::TestWithParam<MethodParamCountTestFixtureParams> {
public:
  BirdTest::TestOptions options;
  void setUp() {
    options.compile = false;
    options.interpret = false;

    options.code = "\
                struct Foo { \
                    fn print_val(x : int) { \
                        print x;\
                    }\
                };\
                const foo: Foo = Foo{}; \
                foo.print_val(" +
                   GetParam().params + ");";

    options.after_type_check = [&](auto &error_tracker, auto &analyzer) {
      if (GetParam().expected_result == false) {
        ASSERT_TRUE(error_tracker.has_errors());
        ASSERT_EQ(std::get<0>(error_tracker.get_errors()[0]),
                  ">>[ERROR] type error: Invalid number of arguments to "
                  "print_val (line 1, character 205)");
      }
    };
  }

  void compile() {
    const auto params = GetParam();
    ASSERT_EQ(BirdTest::compile(options), params.expected_result);
  }
};

TEST_P(MethodParamCountTestFixture, StructMethodParamCount) {
  setUp();
  compile();
}

INSTANTIATE_TEST_SUITE_P(
    StructMethodParamCount, MethodParamCountTestFixture,
    ::testing::Values((MethodParamCountTestFixtureParams){"1", true},
                      (MethodParamCountTestFixtureParams){"1, 2", false},
                      (MethodParamCountTestFixtureParams){"1, 2, 3", false},
                      (MethodParamCountTestFixtureParams){"1,2,3,3,4", false},
                      (MethodParamCountTestFixtureParams){"", false},
                      (MethodParamCountTestFixtureParams){"42", true}));
