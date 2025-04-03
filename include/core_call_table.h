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
            std::vector<std::shared_ptr<BirdType>>{std::make_shared<ArrayType>(
                std::make_shared<Generic>())}, // any type will be fine
            std::make_shared<IntType>()));

    table.declare(
        "push",
        std::make_shared<BirdFunction>(
            std::vector<std::shared_ptr<BirdType>>{
                std::make_shared<ArrayType>(std::make_shared<Generic>()),
                std::make_shared<Generic>()},
            std::make_shared<VoidType>()));

    table.declare("gc", std::make_shared<BirdFunction>(
                            std::vector<std::shared_ptr<BirdType>>{},
                            std::make_shared<VoidType>()));

    table.declare(
        "iter",
        std::make_shared<BirdFunction>(
            std::vector<std::shared_ptr<BirdType>>{std::make_shared<Generic>()},
            std::make_shared<IteratorType>(std::make_shared<Generic>())));
  }
};