#pragma once
#include <optional>
#include <string>

struct SemanticValue {
  bool is_mutable;

  SemanticValue(bool is_mutable) : is_mutable(is_mutable) {}
  SemanticValue() = default;
};