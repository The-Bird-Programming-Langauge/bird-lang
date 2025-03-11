#include "../helpers/compile_helper.hpp"

// INT
TEST(MiscProgramsTestSuite, MatrixMultiplicationTest) {
  BirdTest::TestOptions options;
  options.code = "fn print_mat(mat: int[][]) -> void {"
                 "for var i = 0; i < 3; i += 1 {"
                 "for var j = 0; j < 3; j += 1 {"
                 "print mat[i][j];"
                 "}"
                 "}"
                 "}"

                 "fn multiply(first: int[][], second: int[][]) -> int[][] {"
                 "var result = ["
                 "[0,0,0],"
                 "[0,0,0],"
                 "[0,0,0]"
                 "];"

                 "for var i = 0; i < 3; i += 1 {"
                 "for var j = 0; j < 3; j+= 1 {"
                 "result[i][j] += first[i][j] * second[j][i];"
                 "}"
                 "}"

                 "return result;"
                 "}"

                 "const first = ["
                 "[1,2,3],"
                 "[4,5,6],"
                 "[7,8,9]"
                 "];"

                 "const second = ["
                 "[2,2,2],"
                 "[2,2,2],"
                 "[2,2,2]"
                 "];"

                 "print_mat(first);"
                 "print_mat(second);"
                 "const result = (multiply(first, second));"
                 "print_mat(result);";

  options.after_interpret = [&](Interpreter &interpreter) {
    using ArrayType = std::shared_ptr<std::vector<Value>>;
    ASSERT_TRUE(interpreter.env.contains("result"));
    const auto result_is_array =
        is_type<ArrayType>(interpreter.env.get("result"));
    ASSERT_TRUE(result_is_array);

    const auto result = as_type<ArrayType>(interpreter.env.get("result"));

    auto count = 0;
    for (const auto &item : *result) {
      const auto arr = as_type<ArrayType>(item);
      for (const auto &val : *arr) {
        ASSERT_EQ(as_type<int>(val), (++count * 2));
      }
    }
  };

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    std::string first;
    std::string second;
    std::string result;

    for (int i = 0; i < 3; i++) {
      for (int j = 1; j <= 3; j++) {
        first.append(std::to_string((i * 3) + j) + "\n");
        second.append("2\n");
        result.append(std::to_string(((i * 3) + j) * 2) + "\n");
      }
    }

    ASSERT_EQ(output, first + second + result + "\n");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}
