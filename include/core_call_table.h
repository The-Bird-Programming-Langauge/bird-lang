#pragma once
#include "bird_type.h"
#include "sym_table.h"
#include <memory>

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