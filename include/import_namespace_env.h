#pragma once

#include "ast_node/stmt/namespace_stmt.h"
#include "import_path.h"
#include <vector>
#include <unordered_map>
#include <iostream>

class ImportNamespaceItem {
public:
  virtual ~ImportNamespaceItem() {}
};

class NamespaceStmtItem : public ImportNamespaceItem {
public:
  std::unordered_map<std::string, ImportNamespaceItem*> namespace_items;
  NamespaceStmt* stmt;

  NamespaceStmtItem() : stmt(nullptr) {}

  NamespaceStmtItem(NamespaceStmt* stmt) : stmt(stmt) {}
};
  
class DefaultStmtItem : public ImportNamespaceItem {
public:
  Stmt* stmt;

  DefaultStmtItem() = default;

  DefaultStmtItem(Stmt* stmt) : stmt(stmt) {}
};

class ImportNamespaceEnvironment {
public:
  NamespaceStmtItem namespace_item;

  ImportNamespaceEnvironment() = default;

  bool contains_item(ImportPath import_path) {
    NamespaceStmtItem* current = &this->namespace_item;

    for (int i = 0; i < import_path.path.size() - 1; i += 1) {
      if (current->namespace_items.find(import_path.path[i]) == current->namespace_items.end()) {
        return false;
      }

      NamespaceStmtItem* namespace_item = dynamic_cast<NamespaceStmtItem*>(current->namespace_items[import_path.path[i]]);
      current = namespace_item;
    }

    return current->namespace_items.find(import_path.path.back()) != current->namespace_items.end();
  }

  void add_item(ImportPath import_path, std::unique_ptr<Stmt> stmt) {
    NamespaceStmtItem* current = &this->namespace_item;

    for (int i = 0; i < import_path.path.size() - 1; i += 1) {
      if (current->namespace_items.find(import_path.path[i]) == current->namespace_items.end()) {
        NamespaceStmtItem* new_namespace_item = new NamespaceStmtItem(
          new NamespaceStmt(
            Token(Token::Type::IDENTIFIER, import_path.path[i], 0, 0),
            std::vector<std::unique_ptr<Stmt>>()));

        current->namespace_items[import_path.path[i]] = new_namespace_item;

        if (current->stmt) {
          current->stmt->members.push_back(std::unique_ptr<Stmt>(new_namespace_item->stmt));
        }
      }

      NamespaceStmtItem* namespace_item = dynamic_cast<NamespaceStmtItem*>(current->namespace_items[import_path.path[i]]);
      current = namespace_item;
    }

    DefaultStmtItem* new_default_item = new DefaultStmtItem();
    new_default_item->stmt = dynamic_cast<Stmt*>(stmt.get());

    if (current) {
      current->stmt->members.push_back(std::move(stmt));
    }

    current->namespace_items[import_path.path.back()] = new_default_item;
  }

  void debug() {
    std::cout << "Namespace Environment Debug Output:\n";
    debug_namespace_item(namespace_item, 0);
}

private:
  // Helper function to recursively print namespace items
  void debug_namespace_item(NamespaceStmtItem& item, int level) {
    // Print the current level of the namespace
    std::cout << std::string(level * 2, ' ') << "NamespaceStmtItem (" 
              << (item.stmt ? "has stmt" : "no stmt") << "):\n";

    // If there's a stmt, print the details of the NamespaceStmt
    if (item.stmt) {
      std::cout << std::string((level + 1) * 2, ' ') << "NamespaceStmt:\n";
      std::cout << std::string((level + 2) * 2, ' ') << "Identifier: " 
                << item.stmt->identifier.lexeme << "\n";  // Assuming 'lexeme' is the string in Token

      std::cout << std::string((level + 2) * 2, ' ') << "Members:\n";
      for (const auto& member : item.stmt->members) {
        std::cout << std::string((level + 3) * 2, ' ') << "Stmt: " 
                  << (member ? "has stmt" : "no stmt") << "\n";
      }
    }

    // Iterate through the namespace items and print their keys and types
    for (const auto& pair : item.namespace_items) {
      std::cout << std::string((level + 1) * 2, ' ') << "Key: " << pair.first << "\n";

      // Recursively call the function if it's a NamespaceStmtItem
      NamespaceStmtItem* ns_item = dynamic_cast<NamespaceStmtItem*>(pair.second);
      if (ns_item) {
        debug_namespace_item(*ns_item, level + 1);
      } else {
        // For DefaultStmtItem, print the type of the statement
        DefaultStmtItem* default_item = dynamic_cast<DefaultStmtItem*>(pair.second);
        if (default_item) {
          // Check if stmt is not null before calling typeid
          if (default_item->stmt) {
            std::cout << std::string((level + 2) * 2, ' ') << "DefaultStmtItem (has stmt, Type: " 
                      << typeid(*default_item->stmt).name() << ")\n";
          } else {
            std::cout << std::string((level + 2) * 2, ' ') << "DefaultStmtItem (no stmt)\n";
          }
        }
      }
    }
  }
};