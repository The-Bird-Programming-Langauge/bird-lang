#pragma once
#include "../bird_type.h"
#include <memory>

// forward declaration
class Visitor;

/*
 * parent AST node from which all nodes are derived,
 * allows for visitor patter with accept method
 */
class Node {
  std::shared_ptr<BirdType> type = std::make_shared<ErrorType>();

public:
  virtual void accept(Visitor *visitor) = 0;
  void set_type(std::shared_ptr<BirdType> type) { this->type = type; }
  std::shared_ptr<BirdType> get_type() { return this->type; }
};
