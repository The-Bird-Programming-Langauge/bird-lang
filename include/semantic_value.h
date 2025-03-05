#pragma once
#include <optional>
#include <string>

struct SemanticValue {
  bool is_mutable;

  bool is_struct = false;
  std::optional<std::string> class_name;

  SemanticValue(bool is_mutable) : is_mutable(is_mutable) {}
  SemanticValue() = default;
};