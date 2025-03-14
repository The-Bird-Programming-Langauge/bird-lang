#include "../../helpers/compile_helper.hpp"

struct MethodParamTestFixtureParams {
  std::string params;
  bool expected_result;
  std::string expected_error;
};

class MethodParamTestFixture
    : public testing::TestWithParam<MethodParamTestFixtureParams> {
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
                  GetParam().expected_error);
      }
    };
  }

  void compile() {
    const auto params = GetParam();
    ASSERT_EQ(BirdTest::compile(options), params.expected_result);
  }
};

TEST_P(MethodParamTestFixture, StructMethodParam) {
  setUp();
  compile();
}

INSTANTIATE_TEST_SUITE_P(
    StructMethodParam, MethodParamTestFixture,
    ::testing::Values(
        (MethodParamTestFixtureParams){"1", true},
        (MethodParamTestFixtureParams){"1, 2", false,
                                       ">>[ERROR] type error: Invalid number "
                                       "of arguments. Expected 1, found 2 "
                                       "(line 1, character 205)"},
        (MethodParamTestFixtureParams){"1, 2, 3", false,
                                       ">>[ERROR] type error: Invalid number "
                                       "of arguments. Expected 1, found 3 "
                                       "(line 1, character 205)"},
        (MethodParamTestFixtureParams){
            "1,2,3,3,4", false,
            ">>[ERROR] type error: Invalid number of arguments. Expected 1, "
            "found 5 (line 1, character 205)"},
        (MethodParamTestFixtureParams){
            "", false,
            ">>[ERROR] type error: Invalid number of arguments. Expected 1, "
            "found 0 (line 1, character 205)"},
        (MethodParamTestFixtureParams){"42", true, ""},
        (MethodParamTestFixtureParams){
            "true", false,
            ">>[ERROR] type mismatch: expected int, found bool (line 1, "
            "character 205)"}));
