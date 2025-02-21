#pragma once

#include "../../include/exceptions/user_error_tracker.h"
#include "../../include/parser.h"
#include "../../include/visitors/interpreter.h"
#include "../../include/visitors/semantic_analyzer.h"
#include "../../include/visitors/type_checker.h"

#include "../../include/visitors/code_gen.h"

#include <functional>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace BirdTest {
struct TestOptions {
  std::string code;
  // bool lex = true;
  bool parse = true;
  bool type_check = true;
  bool semantic_analyze = true;
  bool interpret = true;
  bool compile = true;

  std::optional<std::function<void(UserErrorTracker &, Parser &,
                                   const std::vector<std::unique_ptr<Stmt>> &)>>
      after_parse;
  std::optional<std::function<void(UserErrorTracker &, SemanticAnalyzer &)>>
      after_semantic_analyze;
  std::optional<std::function<void(UserErrorTracker &, TypeChecker &)>>
      after_type_check;
  std::optional<std::function<void(Interpreter &)>> after_interpret;
  std::optional<std::function<void(std::string &, CodeGen &)>> after_compile;

  TestOptions() = default;
};

bool compile(const TestOptions options);
}; // namespace BirdTest
