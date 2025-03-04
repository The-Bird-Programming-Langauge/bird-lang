#include "../../include/visitors/interpreter.h"
#include <gtest/gtest.h>
#include <memory>

#define TEST_AND_EXPR(left, right)                                             \
  new Binary(left, Token(Token::Type::AND, "and"), right)

#define TEST_OR_EXPR(left, right)                                              \
  new Binary(left, Token(Token::Type::OR, "or"), right)

#define TEST_TRUE                                                              \
  std::make_unique<Primary>(Token(Token::Type::BOOL_LITERAL, "true"))
#define TEST_FALSE                                                             \
  std::make_unique<Primary>(Token(Token::Type::BOOL_LITERAL, "false"))

#define TEST_BAD_EXPR                                                          \
  std::make_unique<Binary>(                                                    \
      std::make_unique<Primary>(Token(Token::Type::INT_LITERAL, "1")),         \
      Token(Token::Type::SLASH, "/"),                                          \
      std::make_unique<Primary>(Token(Token::Type::INT_LITERAL, "0")))

class BinaryShortCircuitTest : public testing::TestWithParam<Binary *> {
public:
  Interpreter *interpreter;
  bool successful_interpret;

  void setUp() {
    // std::cout << "starting setup" << std::endl;
    auto binary_expr = std::shared_ptr<Binary>(GetParam());
    // auto cast = dynamic_cast<Primary *>(binary_expr->left.get());
    // std::cout << "left arg is primary " << (cast == nullptr) << std::endl;
    // std::cout << "interpreter is null " << (interpreter == nullptr)
    // << std::endl;
    // cast->accept(interpreter);
    try {
      interpreter->visit_binary_short_circuit(binary_expr.get());
      successful_interpret = true;
    } catch (BirdException) {
      successful_interpret = false;
    }
    std::cout << "finished setup" << std::endl;
  }
};

TEST_P(BinaryShortCircuitTest, TestValid) {
  setUp();
  ASSERT_TRUE(false);
}

INSTANTIATE_TEST_SUITE_P(
    ShortCircuitTests, BinaryShortCircuitTest,
    ::testing::Values(new Binary(
        std::make_unique<Primary>(Token(Token::Type::BOOL_LITERAL, "true")),
        Token(Token::Type::AND, "and"),
        std::make_unique<Primary>(Token(Token::Type::BOOL_LITERAL, "true")))
                      // TEST_AND_EXPR(TEST_BAD_EXPR, TEST_BAD_EXPR)
                      // TEST_AND_EXPR(TEST_FALSE, TEST_BAD_EXPR),
                      // TEST_AND_EXPR(TEST_TRUE, TEST_BAD_EXPR),
                      // TEST_OR_EXPR(TEST_FALSE, TEST_BAD_EXPR),
                      // TEST_OR_EXPR(TEST_FALSE, TEST_BAD_EXPR),
                      // TEST_AND_EXPR(TEST_BAD_EXPR, TEST_TRUE),
                      // TEST_AND_EXPR(TEST_BAD_EXPR, TEST_FALSE),
                      // TEST_OR_EXPR(TEST_BAD_EXPR, TEST_FALSE),
                      // TEST_OR_EXPR(TEST_BAD_EXPR, TEST_TRUE)
                      ));