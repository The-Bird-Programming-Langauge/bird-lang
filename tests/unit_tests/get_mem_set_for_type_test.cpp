#include "../../include/bird_type.h"
#include "../../include/visitors/code_gen.h"
#include <gtest/gtest.h>

#define BIRD_TYPE_INT BirdType(BirdTypeType::INT)
#define BIRD_TYPE_BOOL BirdType(BirdTypeType::BOOL)
#define BIRD_TYPE_FLOAT BirdType(BirdTypeType::FLOAT)
#define BIRD_TYPE_STRUCT BirdType(BirdTypeType::STRUCT)
#define BIRD_TYPE_ARRAY BirdType(BirdTypeType::ARRAY)
#define BIRD_TYPE_PLACEHOLDER BirdType(BirdTypeType::PLACEHOLDER)
#define BIRD_TYPE_VOID BirdType(BirdTypeType::VOID)

#define INT_MEM_SET "mem_set_32"
#define BOOL_MEM_SET "mem_set_32"
#define FLOAT_MEM_SET "mem_set_64"
#define STRUCT_MEM_SET "mem_set_ptr"
#define ARRAY_MEM_SET "mem_set_ptr"
#define PLACEHOLDER_MEM_SET "mem_set_ptr"
#define VOID_MEM_SET "mem_set_32"

class GetMemSetForTypeTest
    : public testing::TestWithParam<std::pair<BirdType, std::string>> {
public:
  bool run() {
    auto params = GetParam();
    BirdTypeType type = params.first.type;
    std::string expected_mem_set_func = params.second;

    std::string actual_mem_set_func = get_mem_set_for_type(type);

    return actual_mem_set_func == expected_mem_set_func;
  }
};

TEST_P(GetMemSetForTypeTest, GetMemSetTest) { ASSERT_TRUE(run()); }

INSTANTIATE_TEST_SUITE_P(
    GetMemSetTest, GetMemSetForTypeTest,
    ::testing::Values(std::make_pair(BIRD_TYPE_INT, INT_MEM_SET),
                      std::make_pair(BIRD_TYPE_BOOL, BOOL_MEM_SET),
                      std::make_pair(BIRD_TYPE_FLOAT, FLOAT_MEM_SET),
                      std::make_pair(BIRD_TYPE_STRUCT, STRUCT_MEM_SET),
                      std::make_pair(BIRD_TYPE_ARRAY, ARRAY_MEM_SET),
                      std::make_pair(BIRD_TYPE_PLACEHOLDER,
                                     PLACEHOLDER_MEM_SET),
                      std::make_pair(BIRD_TYPE_VOID, VOID_MEM_SET)));