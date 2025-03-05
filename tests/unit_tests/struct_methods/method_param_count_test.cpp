#include "../../helpers/compile_helper.hpp"

struct MethodParamCountTestFixtureParams {
  std::string params;
  bool expected_result;
  std::string expected_error;
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
                  GetParam().expected_error);
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
    ::testing::Values(
        (MethodParamCountTestFixtureParams){"1", true},
        (MethodParamCountTestFixtureParams){
            "1, 2", false,
            ">>[ERROR] type error: Invalid number of arguments to "
            "print_val (line 1, character 205)"},
        (MethodParamCountTestFixtureParams){
            "1, 2, 3", false,
            ">>[ERROR] type error: Invalid number of arguments to "
            "print_val (line 1, character 205)"},
        (MethodParamCountTestFixtureParams){
            "1,2,3,3,4", false,
            ">>[ERROR] type error: Invalid number of arguments to "
            "print_val (line 1, character 205)"},
        (MethodParamCountTestFixtureParams){
            "", false,
            ">>[ERROR] type error: Invalid number of arguments to "
            "print_val (line 1, character 205)"},
        (MethodParamCountTestFixtureParams){"42", true, ""},
        (MethodParamCountTestFixtureParams){
            "true", false,
            ">>[ERROR] type mismatch: in function call (line 1, "
            "character 205)"}));
