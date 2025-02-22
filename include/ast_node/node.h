#pragma once

// forward declaration
class Visitor;

/*
 * parent AST node from which all nodes are derived,
 * allows for visitor patter with accept method
 */
class Node {
public:
  virtual void accept(Visitor *visitor) = 0;
};
