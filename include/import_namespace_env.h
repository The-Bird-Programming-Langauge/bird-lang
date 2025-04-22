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
};