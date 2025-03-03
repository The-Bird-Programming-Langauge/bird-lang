#include "../../include/ast_node/index.h"
#include "../../include/reorder_ast.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#define TEST_STRUCT_DECL                                                       \
  new StructDecl(                                                              \
      Token(),                                                                 \
      std::vector<std::pair<std::string, std::shared_ptr<ParseType::Type>>>())

#define TEST_TYPE_STMT new TypeStmt(Token(), std::shared_ptr<ParseType::Type>())
#define TEST_EXPR_STMT new ExprStmt(std::unique_ptr<Expr>())

class ReorderAstTest : public testing::TestWithParam<std::vector<Stmt *>> {
public:
  int initial_size;
  std::vector<std::unique_ptr<Stmt>> stmts;
  void setUp() {
    auto stmt_ptrs = GetParam();

    for (auto stmt : stmt_ptrs) {
      this->stmts.push_back(std::unique_ptr<Stmt>(stmt));
    }

    initial_size = stmt_ptrs.size();
    reorder_ast(this->stmts);
  }

  bool structs_then_type_stmts_first() {
    bool type_stmt_seen = false;
    bool seen_else = false;
    for (auto &stmt : stmts) {
      if (dynamic_cast<StructDecl *>(stmt.get())) {
        if (type_stmt_seen || seen_else) {
          return false;
        }
      } else if (dynamic_cast<TypeStmt *>(stmt.get())) {
        if (seen_else) {
          return false;
        }
        type_stmt_seen = true;
      } else {
        seen_else = true;
      }
    }

    return true;
  }
};

TEST_P(ReorderAstTest, TestOrder) {
  setUp();

  ASSERT_EQ(initial_size, stmts.size());
  ASSERT_TRUE(structs_then_type_stmts_first());
}

INSTANTIATE_TEST_SUITE_P(
    ReorderTests, ReorderAstTest,
    ::testing::Values(
        std::vector<Stmt *>({TEST_EXPR_STMT, TEST_TYPE_STMT, TEST_STRUCT_DECL}),
        std::vector<Stmt *>({TEST_STRUCT_DECL, TEST_TYPE_STMT}),
        std::vector<Stmt *>(), std::vector<Stmt *>({TEST_EXPR_STMT}),
        std::vector<Stmt *>({TEST_TYPE_STMT, TEST_STRUCT_DECL, TEST_TYPE_STMT,
                             TEST_EXPR_STMT}),
        std::vector<Stmt *>({TEST_STRUCT_DECL, TEST_TYPE_STMT, TEST_STRUCT_DECL,
                             TEST_EXPR_STMT})));
