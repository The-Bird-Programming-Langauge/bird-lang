#include "../include/reorder_ast.h"

void reorder_ast(std::vector<std::unique_ptr<Stmt>> &stmts) {
  // auto count = 0;
  // for (int i = 0; i < stmts.size(); i++) {
  //   if (dynamic_cast<TypeStmt *>(stmts[i].get())) {
  //     count += 1;
  //     for (int j = i; j >= count; j--) {
  //       auto temp = std::move(stmts[j]);
  //       stmts[j] = std::move(stmts[j - 1]);
  //       stmts[j - 1] = std::move(temp);
  //     }
  //   }
  // }

  // count = 0;
  // for (int i = 0; i < stmts.size(); i++) {
  //   if (dynamic_cast<StructDecl *>(stmts[i].get())) {
  //     count += 1;
  //     for (int j = i; j >= count; j--) {
  //       auto temp = std::move(stmts[j]);
  //       stmts[j] = std::move(stmts[j - 1]);
  //       stmts[j - 1] = std::move(temp);
  //     }
  //   }
  // }
}