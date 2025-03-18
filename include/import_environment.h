#pragma once

#include "exceptions/bird_exception.h"
#include "token.h"
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <sstream>

enum ImportType
{
  IMPORT_NAMESPACE,
  IMPORT_VARIABLE,
  IMPORT_TYPE,
  IMPORT_STRUCT,
  IMPORT_FUNCTION,
};

class ImportItem
{
public:
  ImportType type;
  std::optional<std::unordered_map<std::string, ImportItem>> import_items;

  ImportItem() = default;

  ImportItem(ImportType type)
  {
    this->type = type;

    if (type == IMPORT_NAMESPACE)
    {
      import_items.emplace();
    }
  }
};

class ImportEnvironment
{
public:
  std::unordered_map<std::string, ImportItem> import_items;

  ImportEnvironment() = default;

  ImportEnvironment(std::unordered_map<std::string, ImportItem> import_items)
  {
    this->import_items = std::move(import_items);
  }

  bool contains_item(std::vector<std::string> import_path)
  {
    auto* current = &import_items;

    for (int i = 0; i < import_path.size() - 1; i += 1)
    {
      if (current->find(import_path[i]) == current->end())
      {
        return false;
      }

      if ((*current)[import_path[i]].type != IMPORT_NAMESPACE)
      {
        throw BirdException("'" + import_path[i] + "' is not a namespace");
      }

      current = &(*current)[import_path[i]].import_items.value();
    }

    return current->find(import_path.back()) != current->end();
  }

  bool contains_item(std::vector<Token> import_path)
  {
    return this->contains_item(this->to_string_path(import_path));
  }

  bool contains_item(std::string import_path)
  {
    return this->contains_item(this->path_split(import_path));
  }

  void add_item(std::vector<std::string> import_path, ImportType import_type)
  {
    auto* current = &import_items;

    for (int i = 0; i < import_path.size() - 1; i += 1)
    {
      if (current->find(import_path[i]) == current->end())
      {
        (*current)[import_path[i]] = ImportItem(IMPORT_NAMESPACE);
      }

      if ((*current)[import_path[i]].type != IMPORT_NAMESPACE)
      {
        throw BirdException("'" + import_path[i] + "' is not a namespace");
      }

      current = &(*current)[import_path[i]].import_items.value();
    }

    (*current)[import_path.back()] = ImportItem(import_type);
  }

  void add_item(std::vector<Token> import_path, ImportType import_type)
  {
    this->add_item(this->to_string_path(import_path), import_type);
  }

  void add_item(std::string import_path, ImportType import_type)
  {
    this->add_item(this->path_split(import_path), import_type);
  }

  ImportItem get_item(std::vector<std::string> import_path)
  {
    auto* current = &import_items;

    for (int i = 0; i < import_path.size() - 1; i += 1)
    {
      current = &(*current)[import_path[i]].import_items.value();
    }

    return (*current)[import_path.back()];
  }

  ImportItem get_item(std::vector<Token> import_path)
  {
    return this->get_item(this->to_string_path(import_path));
  }

  ImportItem get_item(std::string import_path)
  {
    return this->get_item(this->path_split(import_path));
  }

  // add a function that returns a list of all edge import item paths under the umbrella of a passed in namespace import item path

  std::vector<std::string> to_string_path(std::vector<Token> path)
  {
    std::vector<std::string> result;
  
    for (int i = 0; i < path.size(); i += 1)
    {
      result.push_back(path[i].lexeme);
    }

    return result;
  }

  std::vector<std::string> path_split(std::string path)
  {
    std::vector<std::string> result;
    std::stringstream ss(path);
    std::string token;

    while (std::getline(ss, token, ':'))
    {
        if (!token.empty())
        {
            result.push_back(token);
        }
    }

    return result;
  }
};