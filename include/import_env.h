#pragma once

#include "exceptions/bird_exception.h"
#include "token.h"
#include "import_path.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>

class ImportItem {
  public:
    virtual ~ImportItem() {}
  };
  
class ImportNamespace : public ImportItem {
public:
  std::unordered_map<std::string, ImportItem*> import_items;

  ImportNamespace() = default;

  ImportNamespace(std::unordered_map<std::string, ImportItem*> import_items) {
    this->import_items = import_items;
  }
};
  
class DefaultImportItem : public ImportItem {};

class ImportEnvironment {
public:
  ImportNamespace namespace_item;

  ImportEnvironment() = default;

  ImportEnvironment(ImportNamespace namespace_item) {
    this->namespace_item = namespace_item;
  }

  bool contains_item(ImportPath import_path) {
    ImportNamespace* current = &this->namespace_item;

    for (int i = 0; i < import_path.path.size() - 1; i += 1) {
      if (current->import_items.find(import_path.path[i]) == current->import_items.end()) {
        return false;
      }

      ImportNamespace* next_namespace_item = dynamic_cast<ImportNamespace*>(current->import_items[import_path.path[i]]);
      if (!next_namespace_item) {
        throw BirdException("'" + import_path.path[i] + "' is not a namespace");
      }

      current = next_namespace_item;
    }

    return current->import_items.find(import_path.path.back()) != current->import_items.end();
  }

  void add_item(ImportPath import_path, ImportItem* import_item) {
    ImportNamespace* current = &this->namespace_item;

    for (int i = 0; i < import_path.path.size() - 1; i += 1) {
      if (current->import_items.find(import_path.path[i]) == current->import_items.end()) {
        current->import_items[import_path.path[i]] = new ImportNamespace();
      }

      ImportNamespace* namespace_item = dynamic_cast<ImportNamespace*>(current->import_items[import_path.path[i]]);
      if (!namespace_item) {
        throw BirdException("'" + import_path.path[i] + "' is not a namespace");
      }

      current = namespace_item;
    }

    current->import_items[import_path.path.back()] = import_item;
  }

  void add_item(ImportPath import_path) {
    this->add_item(import_path, new DefaultImportItem());
  }

  ImportItem* get_item(ImportPath import_path) {
    ImportNamespace* current = &this->namespace_item;

    for (int i = 0; i < import_path.path.size() - 1; i += 1) {
      ImportNamespace* next_namespace_item = dynamic_cast<ImportNamespace*>(current->import_items[import_path.path[i]]);
      if (!next_namespace_item) {
        throw BirdException("'" + import_path.path[i] + "' is not a namespace");
      }

      current = next_namespace_item;
    }

    return current->import_items[import_path.path.back()];
  }

  std::tuple<std::vector<ImportPath>, std::vector<ImportItem*>> get_items_recursively(ImportPath import_path) {
    std::vector<ImportPath> paths;
    std::vector<ImportItem*> items;

    ImportNamespace* current = &this->namespace_item;

    for (int i = 0; i < import_path.path.size() - 1; i += 1) {
      ImportNamespace* next_namespace_item = dynamic_cast<ImportNamespace*>(current->import_items[import_path.path[i]]);
      if (!next_namespace_item) {
        throw BirdException("'" + import_path.path[i] + "' is not a namespace");
      }

      current = next_namespace_item;
    }

    ImportItem* current_item = current->import_items[import_path.path.back()];

    std::function<void(ImportPath, ImportItem*)> dfs = [&](ImportPath current_path, ImportItem* import_item) {
      ImportNamespace* namespace_item = dynamic_cast<ImportNamespace*>(import_item);
      if (namespace_item) {
        for (auto& [key, value] : namespace_item->import_items) {
          ImportPath new_path = current_path;
          new_path.add_string_token(key);
          dfs(new_path, value);
        }
      } else {
        paths.push_back(current_path);
        items.push_back(import_item);
      }
    };

    dfs(import_path, current_item);
    return std::make_tuple(paths, items);
  }
  
  void debug_print(const ImportNamespace& namespace_item, const std::string& indent = "") const {
    for (const auto& [key, value] : namespace_item.import_items) {
      std::cout << indent << "- '" << key << "'" << std::endl;

      ImportNamespace* sub_namespace = dynamic_cast<ImportNamespace*>(value);
      if (sub_namespace) {
        debug_print(*sub_namespace, indent + "  ");
      }
    }
  }

  void debug() const {
    std::cout << "Import Environment Debug Output:\n";
    debug_print(namespace_item);
  }
};