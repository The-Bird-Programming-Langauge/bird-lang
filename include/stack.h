#pragma once
#include <vector>

template <typename T> struct Stack {
  std::vector<T> stack;

  void push(T value) { this->stack.push_back(std::move(value)); }

  T pop() {
    T value = std::move(this->stack.back());
    this->stack.pop_back();
    return std::move(value);
  }

  bool empty() { return this->stack.empty(); }

  T peek() const { return this->stack.back(); }
};