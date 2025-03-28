#pragma once
#include "bird_type.h"
#include "sym_table.h"
#include <memory>
#include <unordered_map>

class CoreCallTable {
public:
  std::unordered_map<std::string, std::shared_ptr<BirdFunction>> table;

  CoreCallTable() {
    table["length"] = std::make_shared<BirdFunction>(
        std::vector<std::shared_ptr<BirdType>>{
            std::make_shared<ArrayType>(std::make_shared<IntType>())},
        std::make_shared<IntType>());
  }
};