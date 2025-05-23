#include "../helpers/compile_helper.hpp"

TEST(ParserTest, ParseBlockStmt) {
  BirdTest::TestOptions options;
  options.code = "{ var x: int = 10; var y: int = 20; }";
  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    Block *block = dynamic_cast<Block *>(ast[0].get());
    ASSERT_NE(block, nullptr);

    ASSERT_EQ(block->stmts.size(), 2);

    DeclStmt *first_decl_stmt = dynamic_cast<DeclStmt *>(block->stmts[0].get());
    ASSERT_NE(first_decl_stmt, nullptr);
    EXPECT_EQ(first_decl_stmt->identifier.lexeme, "x");
    ASSERT_TRUE(first_decl_stmt->type.has_value());
    ParseType::Primitive *primitive_type = dynamic_cast<ParseType::Primitive *>(
        first_decl_stmt->type.value().get());
    ASSERT_NE(primitive_type, nullptr);
    EXPECT_EQ(primitive_type->type.lexeme, "int");

    Primary *first_decl_value =
        dynamic_cast<Primary *>(first_decl_stmt->value.get());
    ASSERT_NE(first_decl_value, nullptr);
    EXPECT_EQ(first_decl_value->value.lexeme, "10");

    DeclStmt *second_decl_stmt =
        dynamic_cast<DeclStmt *>(block->stmts[1].get());
    ASSERT_NE(second_decl_stmt, nullptr);
    EXPECT_EQ(second_decl_stmt->identifier.lexeme, "y");
    ASSERT_TRUE(second_decl_stmt->type.has_value());
    ParseType::Primitive *second_primitive_type =
        dynamic_cast<ParseType::Primitive *>(
            second_decl_stmt->type.value().get());
    ASSERT_NE(second_primitive_type, nullptr);
    EXPECT_EQ(second_primitive_type->type.lexeme, "int");

    Primary *second_decl_value =
        dynamic_cast<Primary *>(second_decl_stmt->value.get());
    ASSERT_NE(second_decl_value, nullptr);
    EXPECT_EQ(second_decl_value->value.lexeme, "20");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseConstStmt) {
  BirdTest::TestOptions options;
  options.code = "const z: str = \"foobar\";";
  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    ConstStmt *const_stmt = dynamic_cast<ConstStmt *>(ast[0].get());
    ASSERT_NE(const_stmt, nullptr);
    EXPECT_EQ(const_stmt->identifier.lexeme, "z");
    EXPECT_EQ(const_stmt->type.has_value(), true);
    EXPECT_EQ(const_stmt->type.value()->get_token().lexeme, "str");

    Primary *primary_expr = dynamic_cast<Primary *>(const_stmt->value.get());
    ASSERT_NE(primary_expr, nullptr);
    EXPECT_EQ(primary_expr->value.lexeme, "foobar");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseDeclStmt) {
  BirdTest::TestOptions options;
  options.code = "var x: int = 3;";
  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(ast[0].get());
    ASSERT_NE(decl_stmt, nullptr);
    EXPECT_EQ(decl_stmt->identifier.lexeme, "x");

    ASSERT_TRUE(decl_stmt->type.has_value());
    ParseType::Primitive *primitive_type =
        dynamic_cast<ParseType::Primitive *>(decl_stmt->type.value().get());
    ASSERT_NE(primitive_type, nullptr);
    EXPECT_EQ(primitive_type->type.lexeme, "int");

    Primary *primary_expr = dynamic_cast<Primary *>(decl_stmt->value.get());
    ASSERT_NE(primary_expr, nullptr);
    EXPECT_EQ(primary_expr->value.lexeme, "3");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseExprStmt) {
  /*
   *                    (1 + 2 - 3 / 6 * 4 <=  4)
   *                        /              |    \
   *           (1 + 2 - 3 / 6 * 4)        `<=`  `4`
   *          /       |        \
   *     (1 + 2)     `-`    (3 / 6 * 4)
   *    /   |  \            /      |  \
   *  `1`  `+` `2`      (3  / 6)  `*` `4`
   *                     /  |   \
   *                   `3` `/` `6`
   */
  BirdTest::TestOptions options;
  options.code = "1 + 2 - 3 / 6 * 4 <= 4;";
  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    ExprStmt *expr_stmt = dynamic_cast<ExprStmt *>(ast[0].get());
    ASSERT_NE(expr_stmt, nullptr);

    Binary *outer_binary_expr = dynamic_cast<Binary *>(expr_stmt->expr.get());
    ASSERT_NE(outer_binary_expr, nullptr);
    EXPECT_EQ(outer_binary_expr->op.lexeme, "<=");

    Primary *rhs_primary =
        dynamic_cast<Primary *>(outer_binary_expr->right.get());
    ASSERT_NE(rhs_primary, nullptr);
    EXPECT_EQ(rhs_primary->value.lexeme, "4");

    Binary *lhs_binary_expr =
        dynamic_cast<Binary *>(outer_binary_expr->left.get());
    ASSERT_NE(lhs_binary_expr, nullptr);
    EXPECT_EQ(lhs_binary_expr->op.lexeme, "-");

    Binary *addition_expr = dynamic_cast<Binary *>(lhs_binary_expr->left.get());
    ASSERT_NE(addition_expr, nullptr);
    EXPECT_EQ(addition_expr->op.lexeme, "+");

    Primary *lhs_primary_one =
        dynamic_cast<Primary *>(addition_expr->left.get());
    ASSERT_NE(lhs_primary_one, nullptr);
    EXPECT_EQ(lhs_primary_one->value.lexeme, "1");

    Primary *rhs_primary_two =
        dynamic_cast<Primary *>(addition_expr->right.get());
    ASSERT_NE(rhs_primary_two, nullptr);
    EXPECT_EQ(rhs_primary_two->value.lexeme, "2");

    Binary *multiplication_expr =
        dynamic_cast<Binary *>(lhs_binary_expr->right.get());
    ASSERT_NE(multiplication_expr, nullptr);
    EXPECT_EQ(multiplication_expr->op.lexeme, "*");

    Binary *division_expr =
        dynamic_cast<Binary *>(multiplication_expr->left.get());
    ASSERT_NE(division_expr, nullptr);
    EXPECT_EQ(division_expr->op.lexeme, "/");

    Primary *lhs_primary_three =
        dynamic_cast<Primary *>(division_expr->left.get());
    ASSERT_NE(lhs_primary_three, nullptr);
    EXPECT_EQ(lhs_primary_three->value.lexeme, "3");

    Primary *rhs_primary_six =
        dynamic_cast<Primary *>(division_expr->right.get());
    ASSERT_NE(rhs_primary_six, nullptr);
    EXPECT_EQ(rhs_primary_six->value.lexeme, "6");

    Primary *rhs_primary_four =
        dynamic_cast<Primary *>(multiplication_expr->right.get());
    ASSERT_NE(rhs_primary_four, nullptr);
    EXPECT_EQ(rhs_primary_four->value.lexeme, "4");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseFuncStmt) {
  BirdTest::TestOptions options;
  options.code =
      "fn add(first: int, second: int) -> int { return first + second; }";
  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(ast[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "add");
    ASSERT_TRUE(func_stmt->return_type.has_value());
    EXPECT_EQ(func_stmt->return_type.value()->get_token().lexeme, "int");

    ASSERT_EQ(func_stmt->param_list.size(), 2);
    EXPECT_EQ(func_stmt->param_list[0].first.lexeme, "first");
    EXPECT_EQ(func_stmt->param_list[0].second->get_token().lexeme, "int");
    EXPECT_EQ(func_stmt->param_list[1].first.lexeme, "second");
    EXPECT_EQ(func_stmt->param_list[1].second->get_token().lexeme, "int");

    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 1);

    ReturnStmt *return_stmt =
        dynamic_cast<ReturnStmt *>(block_stmt->stmts[0].get());
    ASSERT_NE(return_stmt, nullptr);

    ASSERT_TRUE(return_stmt->expr.has_value());

    Binary *binary_expr =
        dynamic_cast<Binary *>(return_stmt->expr.value().get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op.lexeme, "+");

    Primary *lhs_primary = dynamic_cast<Primary *>(binary_expr->left.get());
    ASSERT_NE(lhs_primary, nullptr);
    EXPECT_EQ(lhs_primary->value.lexeme, "first");

    Primary *rhs_primary = dynamic_cast<Primary *>(binary_expr->right.get());
    ASSERT_NE(rhs_primary, nullptr);
    EXPECT_EQ(rhs_primary->value.lexeme, "second");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseFunctionNoArgsNoReturnType) {
  BirdTest::TestOptions options;
  options.code = "fn function() {}";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(ast[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "function");
    ASSERT_FALSE(func_stmt->return_type.has_value());

    ASSERT_EQ(func_stmt->param_list.size(), 0);

    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 0);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseFunctionNoArgs) {
  BirdTest::TestOptions options;
  options.code = "fn function() -> int { return 3; }";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(ast[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "function");
    ASSERT_TRUE(func_stmt->return_type.has_value());
    EXPECT_EQ(func_stmt->return_type.value()->get_token().lexeme, "int");

    ASSERT_EQ(func_stmt->param_list.size(), 0);

    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 1);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseFunctionNoReturnType) {
  BirdTest::TestOptions options;
  options.code = "fn function(i: int, j: str) {}";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    Func *func_stmt = dynamic_cast<Func *>(ast[0].get());
    ASSERT_NE(func_stmt, nullptr);

    EXPECT_EQ(func_stmt->identifier.lexeme, "function");
    ASSERT_FALSE(func_stmt->return_type.has_value());

    ASSERT_EQ(func_stmt->param_list.size(), 2);
    EXPECT_EQ(func_stmt->param_list[0].first.lexeme, "i");
    EXPECT_EQ(func_stmt->param_list[0].second->get_token().lexeme, "int");
    EXPECT_EQ(func_stmt->param_list[1].first.lexeme, "j");
    EXPECT_EQ(func_stmt->param_list[1].second->get_token().lexeme, "str");

    Block *block_stmt = dynamic_cast<Block *>(func_stmt->block.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 0);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, FunctionFailsArrowNoReturnType) {
  BirdTest::TestOptions options;
  options.code = "fn function() -> {}";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_TRUE(error_tracker.has_errors());
    auto errors = error_tracker.get_errors();
    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(std::get<0>(errors[0]),
              ">>[ERROR] expected type after arrow (line 0, character 15)");
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ParserTest, ParseIfStmt) {
  BirdTest::TestOptions options;
  options.code =
      "if 1 > 2 {print 1;} else if 1 == 2 {print 2;} else {print 3;}";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    IfStmt *if_stmt = dynamic_cast<IfStmt *>(ast[0].get());
    ASSERT_NE(if_stmt, nullptr);

    Binary *condition_if = dynamic_cast<Binary *>(if_stmt->condition.get());
    ASSERT_NE(condition_if, nullptr);
    EXPECT_EQ(condition_if->op.lexeme, ">");

    Primary *lhs_condition_if =
        dynamic_cast<Primary *>(condition_if->left.get());
    ASSERT_NE(lhs_condition_if, nullptr);
    EXPECT_EQ(lhs_condition_if->value.lexeme, "1");

    Primary *rhs_condition_if =
        dynamic_cast<Primary *>(condition_if->right.get());
    ASSERT_NE(rhs_condition_if, nullptr);
    EXPECT_EQ(rhs_condition_if->value.lexeme, "2");

    Block *then_block = dynamic_cast<Block *>(if_stmt->then_branch.get());
    ASSERT_NE(then_block, nullptr);
    ASSERT_EQ(then_block->stmts.size(), 1);

    PrintStmt *print_stmt =
        dynamic_cast<PrintStmt *>(then_block->stmts[0].get());

    Primary *then_arg = dynamic_cast<Primary *>(print_stmt->args[0].get());
    ASSERT_NE(then_arg, nullptr);
    EXPECT_EQ(then_arg->value.lexeme, "1");

    ASSERT_TRUE(if_stmt->else_branch.has_value());
    IfStmt *else_if_stmt = dynamic_cast<IfStmt *>(if_stmt->else_branch->get());
    ASSERT_NE(else_if_stmt, nullptr);

    Binary *condition_else_if =
        dynamic_cast<Binary *>(else_if_stmt->condition.get());
    ASSERT_NE(condition_else_if, nullptr);
    EXPECT_EQ(condition_else_if->op.lexeme, "==");

    Primary *lhs_condition_else_if =
        dynamic_cast<Primary *>(condition_else_if->left.get());
    ASSERT_NE(lhs_condition_else_if, nullptr);
    EXPECT_EQ(lhs_condition_else_if->value.lexeme, "1");

    Primary *rhs_condition_else_if =
        dynamic_cast<Primary *>(condition_else_if->right.get());
    ASSERT_NE(rhs_condition_else_if, nullptr);
    EXPECT_EQ(rhs_condition_else_if->value.lexeme, "2");

    Block *else_if_then_block =
        dynamic_cast<Block *>(else_if_stmt->then_branch.get());
    ASSERT_NE(else_if_then_block, nullptr);
    ASSERT_EQ(else_if_then_block->stmts.size(), 1);

    PrintStmt *else_if_print_stmt =
        dynamic_cast<PrintStmt *>(else_if_then_block->stmts[0].get());

    Primary *else_if_arg =
        dynamic_cast<Primary *>(else_if_print_stmt->args[0].get());
    ASSERT_NE(else_if_arg, nullptr);
    EXPECT_EQ(else_if_arg->value.lexeme, "2");

    ASSERT_TRUE(else_if_stmt->else_branch.has_value());

    Block *else_block = dynamic_cast<Block *>(else_if_stmt->else_branch->get());
    ASSERT_NE(else_block, nullptr);
    ASSERT_EQ(else_block->stmts.size(), 1);

    PrintStmt *else_print_stmt =
        dynamic_cast<PrintStmt *>(else_block->stmts[0].get());

    Primary *else_arg = dynamic_cast<Primary *>(else_print_stmt->args[0].get());
    ASSERT_NE(else_arg, nullptr);
    EXPECT_EQ(else_arg->value.lexeme, "3");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParsePrintStmt) {
  BirdTest::TestOptions options;
  options.code = "print 28;";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    PrintStmt *print_stmt = dynamic_cast<PrintStmt *>(ast[0].get());
    ASSERT_NE(print_stmt, nullptr);

    ASSERT_EQ(print_stmt->args.size(), 1);
    Primary *primary_expr = dynamic_cast<Primary *>(print_stmt->args[0].get());
    ASSERT_NE(primary_expr, nullptr);
    EXPECT_EQ(primary_expr->value.lexeme, "28");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseWhileStmt) {
  BirdTest::TestOptions options;
  options.code = "while (true) { print 1; }";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_EQ(ast.size(), 1);

    WhileStmt *while_stmt = dynamic_cast<WhileStmt *>(ast[0].get());
    ASSERT_NE(while_stmt, nullptr);

    Primary *condition_expr =
        dynamic_cast<Primary *>(while_stmt->condition.get());
    ASSERT_NE(condition_expr, nullptr);
    EXPECT_EQ(condition_expr->value.lexeme, "true");

    Block *block_stmt = dynamic_cast<Block *>(while_stmt->stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    ASSERT_EQ(block_stmt->stmts.size(), 1);

    PrintStmt *print_stmt =
        dynamic_cast<PrintStmt *>(block_stmt->stmts[0].get());
    ASSERT_NE(print_stmt, nullptr);
    ASSERT_EQ(print_stmt->args.size(), 1);

    Primary *print_arg = dynamic_cast<Primary *>(print_stmt->args[0].get());
    ASSERT_NE(print_arg, nullptr);
    EXPECT_EQ(print_arg->value.lexeme, "1");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseBreakStmt) {
  BirdTest::TestOptions options;
  options.code = "break;";

  options.compile = false;
  options.interpret = false;
  options.semantic_analyze = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    BreakStmt *break_stmt = dynamic_cast<BreakStmt *>(ast[0].get());
    ASSERT_NE(break_stmt, nullptr);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseContinueStmt) {
  BirdTest::TestOptions options;
  options.code = "continue;";

  options.compile = false;
  options.interpret = false;
  options.semantic_analyze = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    ContinueStmt *continue_stmt = dynamic_cast<ContinueStmt *>(ast[0].get());
    ASSERT_NE(continue_stmt, nullptr);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseTernaryAccept) {
  BirdTest::TestOptions options;
  options.code = "print 2.3 > 2.1 ? 1 : 0;";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    PrintStmt *print_stmt = dynamic_cast<PrintStmt *>(ast[0].get());
    ASSERT_NE(print_stmt, nullptr);

    ASSERT_EQ(print_stmt->args.size(), 1);
    Ternary *ternary_expr = dynamic_cast<Ternary *>(print_stmt->args[0].get());
    ASSERT_NE(ternary_expr, nullptr);

    Binary *condition = dynamic_cast<Binary *>(ternary_expr->condition.get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->op.lexeme, ">");

    Primary *lhs_primary = dynamic_cast<Primary *>(condition->left.get());
    ASSERT_NE(lhs_primary, nullptr);
    EXPECT_EQ(lhs_primary->value.lexeme, "2.3");

    Primary *rhs_primary = dynamic_cast<Primary *>(condition->right.get());
    ASSERT_NE(rhs_primary, nullptr);
    EXPECT_EQ(rhs_primary->value.lexeme, "2.1");

    Primary *ternary_true =
        dynamic_cast<Primary *>(ternary_expr->true_expr.get());
    ASSERT_NE(ternary_true, nullptr);
    EXPECT_EQ(ternary_true->value.lexeme, "1");

    Primary *ternary_false =
        dynamic_cast<Primary *>(ternary_expr->false_expr.get());
    ASSERT_NE(ternary_false, nullptr);
    EXPECT_EQ(ternary_false->value.lexeme, "0");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParseTernaryReject) {
  BirdTest::TestOptions options;
  options.code = "(1 > 2) print 1 : print 2;";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_TRUE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 0);
  };

  ASSERT_FALSE(BirdTest::compile(options));
}

TEST(ParserTest, ParserForLoop) {
  BirdTest::TestOptions options;
  options.code = "for var x: int = 0; x <= 5; x += 1 { print x; }";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [&](UserErrorTracker &error_tracker, Parser &parser,
                            const std::vector<std::unique_ptr<Stmt>> &ast) {
    ASSERT_EQ(ast.size(), 1);

    ForStmt *for_stmt = dynamic_cast<ForStmt *>(ast[0].get());
    ASSERT_NE(for_stmt, nullptr);

    DeclStmt *initializer =
        dynamic_cast<DeclStmt *>(for_stmt->initializer.value().get());
    ASSERT_NE(initializer, nullptr);
    EXPECT_EQ(initializer->identifier.lexeme, "x");
    ASSERT_TRUE(initializer->type.has_value());

    ParseType::Primitive *primitive_type =
        dynamic_cast<ParseType::Primitive *>(initializer->type.value().get());
    ASSERT_NE(primitive_type, nullptr);
    EXPECT_EQ(primitive_type->type.lexeme, "int");

    Binary *condition =
        dynamic_cast<Binary *>(for_stmt->condition.value().get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->op.lexeme, "<=");
    Primary *lhs_condition = dynamic_cast<Primary *>(condition->left.get());
    ASSERT_NE(lhs_condition, nullptr);
    EXPECT_EQ(lhs_condition->value.lexeme, "x");
    Primary *rhs_condition = dynamic_cast<Primary *>(condition->right.get());
    ASSERT_NE(rhs_condition, nullptr);
    EXPECT_EQ(rhs_condition->value.lexeme, "5");

    AssignExpr *increment =
        dynamic_cast<AssignExpr *>(for_stmt->increment.value().get());
    ASSERT_NE(increment, nullptr);
    EXPECT_EQ(increment->assign_operator.lexeme, "+=");
    EXPECT_EQ(increment->identifier.lexeme, "x");
    Primary *rhs_increment = dynamic_cast<Primary *>(increment->value.get());
    ASSERT_NE(rhs_increment, nullptr);
    EXPECT_EQ(rhs_increment->value.lexeme, "1");

    ASSERT_NE(for_stmt->body, nullptr);
    Block *block = dynamic_cast<Block *>(for_stmt->body.get());
    ASSERT_NE(block, nullptr);
    ASSERT_EQ(block->stmts.size(), 1);

    PrintStmt *print_stmt = dynamic_cast<PrintStmt *>((block->stmts[0]).get());
    ASSERT_NE(print_stmt, nullptr);
    ASSERT_EQ(print_stmt->args.size(), 1);

    Primary *print_arg = dynamic_cast<Primary *>(print_stmt->args[0].get());
    ASSERT_NE(print_arg, nullptr);
    EXPECT_EQ(print_arg->value.lexeme, "x");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, MultilineComments) {
  BirdTest::TestOptions options;
  options.code = "/* "
                 "anything can go here\n"
                 "or on the next line\n"
                 "we can also have another open /*"
                 "but when we close the comment code works"
                 "*/"
                 "var x = 5;"
                 "print x;";

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ParserBooleanOp) {
  BirdTest::TestOptions options;
  options.code = "false or true and false;";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_EQ(error_tracker.has_errors(), false);

    ASSERT_EQ(ast.size(), 1);

    ExprStmt *stmt = dynamic_cast<ExprStmt *>(ast[0].get());

    Binary *or_expr = dynamic_cast<Binary *>(stmt->expr.get());
    ASSERT_NE(or_expr, nullptr);
    ASSERT_EQ(or_expr->op.lexeme, "or");

    Binary *and_expr = dynamic_cast<Binary *>(or_expr->right.get());
    ASSERT_NE(and_expr, nullptr);
    ASSERT_EQ(and_expr->op.lexeme, "and");

    Primary *left = dynamic_cast<Primary *>(or_expr->left.get());
    ASSERT_NE(left, nullptr);
    ASSERT_EQ(left->value.token_type, Token::Type::FALSE);
    ASSERT_EQ(left->value.lexeme, "false");

    Primary *middle = dynamic_cast<Primary *>(and_expr->left.get());
    ASSERT_NE(middle, nullptr);
    ASSERT_EQ(middle->value.token_type, Token::Type::TRUE);
    ASSERT_EQ(middle->value.lexeme, "true");

    Primary *right = dynamic_cast<Primary *>(and_expr->right.get());
    ASSERT_NE(right, nullptr);
    ASSERT_EQ(right->value.token_type, Token::Type::FALSE);
    ASSERT_EQ(right->value.lexeme, "false");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, SingleLineComments) {
  BirdTest::TestOptions options;
  options.code = "// this is a comment\n"
                 "var x = 5; // this is another comment\n"
                 "print x;";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, ArrayWithoutInit) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [] as int[];";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(ast[0].get());
    ASSERT_NE(decl_stmt, nullptr);
    ASSERT_EQ(decl_stmt->identifier.lexeme, "x");
    ASSERT_TRUE(decl_stmt->type.has_value());

    ParseType::Array *array =
        dynamic_cast<ParseType::Array *>(decl_stmt->type.value().get());
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array->get_token().lexeme, "int");

    AsCast *as_cast = dynamic_cast<AsCast *>(decl_stmt->value.get());
    ASSERT_NE(as_cast, nullptr);

    ArrayInit *array_init = dynamic_cast<ArrayInit *>(as_cast->expr.get());
    ASSERT_NE(array_init, nullptr);
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, IntArrayWithInit) {
  BirdTest::TestOptions options;
  options.code = "var x: int[] = [ 1, 2, 3 ];";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(ast[0].get());
    ASSERT_NE(decl_stmt, nullptr);
    ASSERT_EQ(decl_stmt->identifier.lexeme, "x");
    ASSERT_TRUE(decl_stmt->type.has_value());

    ParseType::Array *array =
        dynamic_cast<ParseType::Array *>(decl_stmt->type.value().get());
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array->get_token().lexeme, "int");

    ArrayInit *array_init = dynamic_cast<ArrayInit *>(decl_stmt->value.get());
    ASSERT_NE(array_init, nullptr);
    ASSERT_EQ(array_init->elements.size(), 3);

    Primary *first = dynamic_cast<Primary *>(array_init->elements[0].get());
    ASSERT_EQ(first->value.lexeme, "1");

    Primary *second = dynamic_cast<Primary *>(array_init->elements[1].get());
    ASSERT_EQ(second->value.lexeme, "2");

    Primary *third = dynamic_cast<Primary *>(array_init->elements[2].get());
    ASSERT_EQ(third->value.lexeme, "3");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, FloatArrayWithInit) {
  BirdTest::TestOptions options;
  options.code = "var x: float[] = [ 1.1, 2.2, 3.3 ];";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(ast[0].get());
    ASSERT_NE(decl_stmt, nullptr);
    ASSERT_EQ(decl_stmt->identifier.lexeme, "x");
    ASSERT_TRUE(decl_stmt->type.has_value());

    ParseType::Array *array =
        dynamic_cast<ParseType::Array *>(decl_stmt->type.value().get());
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array->get_token().lexeme, "float");

    ArrayInit *array_init = dynamic_cast<ArrayInit *>(decl_stmt->value.get());
    ASSERT_NE(array_init, nullptr);
    ASSERT_EQ(array_init->elements.size(), 3);

    Primary *first = dynamic_cast<Primary *>(array_init->elements[0].get());
    ASSERT_EQ(first->value.lexeme, "1.1");

    Primary *second = dynamic_cast<Primary *>(array_init->elements[1].get());
    ASSERT_EQ(second->value.lexeme, "2.2");

    Primary *third = dynamic_cast<Primary *>(array_init->elements[2].get());
    ASSERT_EQ(third->value.lexeme, "3.3");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, StrArrayWithInit) {
  BirdTest::TestOptions options;
  options.code = "var x: str[] = [ \"hello\", \", \", \"world!\" ];";

  options.compile = false;
  options.interpret = false;

  options.after_parse = [](auto &error_tracker, auto &parser, auto &ast) {
    ASSERT_FALSE(error_tracker.has_errors());
    ASSERT_EQ(ast.size(), 1);

    DeclStmt *decl_stmt = dynamic_cast<DeclStmt *>(ast[0].get());
    ASSERT_NE(decl_stmt, nullptr);
    ASSERT_EQ(decl_stmt->identifier.lexeme, "x");
    ASSERT_TRUE(decl_stmt->type.has_value());

    ParseType::Array *array =
        dynamic_cast<ParseType::Array *>(decl_stmt->type.value().get());
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(array->get_token().lexeme, "str");

    ArrayInit *array_init = dynamic_cast<ArrayInit *>(decl_stmt->value.get());
    ASSERT_NE(array_init, nullptr);
    ASSERT_EQ(array_init->elements.size(), 3);

    Primary *first = dynamic_cast<Primary *>(array_init->elements[0].get());
    ASSERT_EQ(first->value.lexeme, "hello");

    Primary *second = dynamic_cast<Primary *>(array_init->elements[1].get());
    ASSERT_EQ(second->value.lexeme, ", ");

    Primary *third = dynamic_cast<Primary *>(array_init->elements[2].get());
    ASSERT_EQ(third->value.lexeme, "world!");
  };

  ASSERT_TRUE(BirdTest::compile(options));
}

TEST(ParserTest, StructWithMemberFunction) {
  BirdTest::TestOptions options;
  options.compile = false;
  options.interpret = false;
  options.type_check = false;
  options.semantic_analyze = false;
  options.code = "struct Person {\
                    name: str;\
                    fn say_name() {\
                      print self.name; \
                    }\
                  };";

  options.after_parse = [&](auto &error_tracker, auto &parser, auto &stmts) {
    ASSERT_FALSE(error_tracker.has_errors());
  };

  ASSERT_TRUE(BirdTest::compile(options));
}