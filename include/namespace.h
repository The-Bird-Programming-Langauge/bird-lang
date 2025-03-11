#pragma once
#include "./sym_table.h"
#include <unordered_map>

template <typename T, typename U, typename V> struct Namespace {
  Environment<T> environment;
  Environment<U> call_table;
  Environment<V> type_table;

  std::shared_ptr<Namespace> parent =
      nullptr; // allows for backtracking to higher namespaces
  std::unordered_map<std::string, std::shared_ptr<Namespace>> nested_namespaces;

  Namespace(std::shared_ptr<Namespace> parent_ns) : parent(parent_ns) {
    environment.push_env();
    call_table.push_env();
    type_table.push_env();
  }
};