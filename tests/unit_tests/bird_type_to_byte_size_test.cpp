#include "../../include/bird_type.h"
#include "../../src/visitors/code_gen/utils.cpp"
#include <gtest/gtest.h>
#include <memory>

#define BIRD_TYPE_FLOAT std::make_shared<BirdType>(BirdTypeType::FLOAT)
#define BIRD_TYPE_BOOL std::make_shared<BirdType>(BirdTypeType::BOOL)
#define BIRD_TYPE_INT std::make_shared<BirdType>(BirdTypeType::INT)

#define BIRD_TYPE_PLACEHOLDER std::make_shared<BirdType>(BirdTypeType::PLACEHOLDER)
#define BIRD_TYPE_STRUCT std::make_shared<BirdType>(BirdTypeType::STRUCT)
#define BIRD_TYPE_STRING std::make_shared<BirdType>(BirdTypeType::STRING)
#define BIRD_TYPE_ARRAY std::make_shared<BirdType>(BirdTypeType::ARRAY)

#define BIRD_TYPE_VOID std::make_shared<BirdType>(BirdType(BirdTypeType::VOID))

#define FLOAT_SIZE 8
#define BOOL_SIZE 4
#define INT_SIZE 4

#define PLACEHOLDER_SIZE 4
#define STRUCT_SIZE 4
#define STRING_SIZE 4
#define ARRAY_SIZE 4

#define VOID_SIZE 0

class BirdTypeToByteSizeTest : public testing::TestWithParam<std::pair<std::shared_ptr<BirdType>, unsigned int>>
{
public:
  bool setup()
  {
    std::shared_ptr<BirdType> bird_type = GetParam().first;
    unsigned int expected_size = GetParam().second;
    unsigned int return_size = bird_type_byte_size(bird_type);

    return return_size == expected_size;
  }
};

TEST_P(BirdTypeToByteSizeTest, ByteSizeTest)
{
  ASSERT_TRUE(setup());
}

INSTANTIATE_TEST_SUITE_P(
    ByteSizeTest,
    BirdTypeToByteSizeTest,
    ::testing::Values(
        std::make_pair(BIRD_TYPE_INT, INT_SIZE),
        std::make_pair(BIRD_TYPE_BOOL, BOOL_SIZE),
        std::make_pair(BIRD_TYPE_FLOAT, FLOAT_SIZE),
        std::make_pair(BIRD_TYPE_STRING, STRING_SIZE),
        std::make_pair(BIRD_TYPE_STRUCT, STRUCT_SIZE),
        std::make_pair(BIRD_TYPE_ARRAY, ARRAY_SIZE),
        std::make_pair(BIRD_TYPE_VOID, VOID_SIZE),
        std::make_pair(BIRD_TYPE_PLACEHOLDER, PLACEHOLDER_SIZE)));
