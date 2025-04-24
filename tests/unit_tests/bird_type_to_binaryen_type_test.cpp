#include "../../include/bird_type.h"
#include "../../src/visitors/code_gen/utils.cpp"
#include <gtest/gtest.h>
#include <memory>

#define BIRD_TYPE_BOOL std::make_shared<BoolType>()
#define BIRD_TYPE_INT std::make_shared<IntType>()
#define BIRD_TYPE_FLOAT std::make_shared<FloatType>()
#define BIRD_TYPE_VOID std::make_shared<VoidType>()
#define BIRD_TYPE_STRING std::make_shared<StringType>()
#define BIRD_TYPE_STRUCT                                                       \
  std::make_shared<StructType>(                                                \
      "", std::vector<std::pair<std::string, std::shared_ptr<BirdType>>>{})
#define BIRD_TYPE_PLACEHOLDER std::make_shared<PlaceholderType>("")
#define BIRD_TYPE_ARRAY                                                        \
  std::make_shared<ArrayType>(std::make_shared<VoidType>())
#define BIRD_TYPE_FUNCTION                                                     \
  std::make_shared<BirdFunction>(std::vector<std::shared_ptr<BirdType>>{},     \
                                 std::make_shared<VoidType>())
#define BIRD_TYPE_ERROR std::make_shared<ErrorType>()

#define BINARYEN_TYPE_INT32 BinaryenTypeInt32()
#define BINARYEN_TYPE_FLOAT64 BinaryenTypeFloat64()
#define BINARYEN_TYPE_NONE BinaryenTypeNone()

class BirdTypeToBinaryenTypeTest
    : public testing::TestWithParam<
          std::pair<std::shared_ptr<BirdType>, std::optional<BinaryenType>>> {
public:
  bool valid_type() {
    std::shared_ptr<BirdType> bird_type = GetParam().first;
    std::optional<BinaryenType> expected_binaryen_type = GetParam().second;

    try {
      BinaryenType return_binaryen_type = bird_type_to_binaryen_type(bird_type);

      if (!expected_binaryen_type.has_value()) {
        return false;
      }

      return return_binaryen_type == expected_binaryen_type.value();
    } catch (const BirdException &) {
      return !expected_binaryen_type.has_value();
    }

    return false;
  }
};

TEST_P(BirdTypeToBinaryenTypeTest, BinaryenTypeTest) {
  ASSERT_TRUE(valid_type());
}

INSTANTIATE_TEST_SUITE_P(
    BinaryenTypeTest, BirdTypeToBinaryenTypeTest,
    ::testing::Values(std::make_pair(BIRD_TYPE_BOOL, BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_INT, BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_FLOAT, BINARYEN_TYPE_FLOAT64),
                      std::make_pair(BIRD_TYPE_VOID, BINARYEN_TYPE_NONE),
                      std::make_pair(BIRD_TYPE_STRING, BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_STRUCT, BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_PLACEHOLDER,
                                     BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_ARRAY, BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_FUNCTION, BINARYEN_TYPE_INT32),
                      std::make_pair(BIRD_TYPE_ERROR, std::nullopt)));