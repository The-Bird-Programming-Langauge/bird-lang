#include "../helpers/compile_helper.hpp"
#include <gtest/gtest.h>

TEST(Namespaces, TrigFunctionsWithStruct) {
  BirdTest::TestOptions options;
  options.code =
      "namespace Math {"
      "  namespace Trig {"
      "    fn sin(a: float, c: float) -> float { return a / c; }"
      "    fn cos(b: float, c: float) -> float { return b / c; }"
      "    fn tan(a: float, b: float) -> float { return a / b; }"
      ""
      "    fn arcsin(x: float) -> float {"
      "      var sum: float = x;"
      "      var term: float = x;"
      "      var n: int = 1;"
      "      var prev_sum: float = sum;"
      "      while n < 10 {"
      "        term *= (x * x) * ((2.0 * n as float - 1.0) / (2.0 * n as "
      "float));"
      "        sum += term / (2.0 * n as float + 1.0);"
      "        if (sum == prev_sum) { break; }"
      "        prev_sum = sum;"
      "        n += 1;"
      "      }"
      "      return sum;"
      "    }"
      ""
      "    fn arccos(x: float) -> float { return 3.1415926 / 2.0 - arcsin(x); }"
      ""
      "    fn arctan(x: float) -> float {"
      "      if x > 1.0 { return 3.14159 / 2.0 - arctan(1.0 / x); }"
      "      var sum: float = x;"
      "      var term: float = x;"
      "      var n: int = 1;"
      "      var sign: float = -1.0;"
      "      while n < 10 {"
      "        term *= x * x;"
      "        sum += (sign * term) / (2 * n + 1) as float;"
      "        sign *= -1.0;"
      "        n += 1;"
      "      }"
      "      return sum;"
      "    }"
      ""
      "    fn to_degrees(theta: float) -> float { return theta * 180.0 / "
      "3.14159; }"
      ""
      "    struct Triangle { a: float; b: float; c: float; };"
      "  }"
      "}"
      ""
      "var t: Math::Trig::Triangle = Math::Trig::Triangle { a = 3.0, b = 4.0, "
      "c = 5.0 };"
      "var sin_x: float = Math::Trig::sin(t.a, t.c);"
      "var cos_x: float = Math::Trig::cos(t.b, t.c);"
      "var tan_x: float = Math::Trig::tan(t.a, t.b);"
      ""
      "print \"sin(x): \", Math::Trig::to_degrees(sin_x), "
      "\" (theta: \", Math::Trig::to_degrees(Math::Trig::arcsin(sin_x)), \" "
      "degrees)\";"
      "print \"cos(x): \", Math::Trig::to_degrees(cos_x), "
      "\" (theta: \", Math::Trig::to_degrees(Math::Trig::arccos(cos_x)), \" "
      "degrees)\";"
      "print \"tan(x): \", Math::Trig::to_degrees(tan_x), "
      "\" (theta: \", Math::Trig::to_degrees(Math::Trig::arctan(tan_x)), \" "
      "degrees)\";";

  options.after_compile = [&](std::string &output, CodeGen &codegen) {
    ASSERT_TRUE(output.find("sin(x): 34.377") != std::string::npos);
    ASSERT_TRUE(output.find("theta: 36.869") != std::string::npos);

    ASSERT_TRUE(output.find("cos(x): 45.836") != std::string::npos);
    ASSERT_TRUE(output.find("theta: 36.880") != std::string::npos);

    ASSERT_TRUE(output.find("tan(x): 42.971") != std::string::npos);
    ASSERT_TRUE(output.find("theta: 36.865") != std::string::npos);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}