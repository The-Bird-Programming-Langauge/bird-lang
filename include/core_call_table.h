#pragma once
#include "bird_type.h"
#include "sym_table.h"
#include <memory>

// inline Environment<std::shared_ptr<BirdFunction>> core_call_table;
// core_call_table(std::map<std::string, std::shared_ptr<BirdFunction>>{
//     {{"length",
//       std::make_shared<BirdFunction>(BirdFunction(
//           std::vector{std::make_shared<BirdType>(BirdTypeType::ARRAY)},
//           std::make_shared<BirdType>(BirdTypeType::INT)))}}});

class CoreCallTable {
public:
  Environment<std::shared_ptr<BirdFunction>> table;

  CoreCallTable() {
    table.push_env();
    table.declare(
        "length",
        std::make_shared<BirdFunction>(
            std::vector{std::make_shared<BirdType>(BirdTypeType::ARRAY)},
            std::make_shared<BirdType>(BirdTypeType::INT)));
  }
};