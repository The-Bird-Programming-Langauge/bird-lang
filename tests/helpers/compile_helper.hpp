#pragma once

#include "exceptions/user_error_tracker.h"
#include "visitors/interpreter.h"
#include "visitors/semantic_analyzer.h"
#include "visitors/type_checker.h"
#include "parser.h"
#include "token.h"
#include "sym_table.h"
#include "value.h"

#include "binaryen-c.h"
#include "visitors/code_gen.h"

#include <gtest/gtest.h>
#include <vector>
#include <functional>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

namespace BirdTest
{
    struct TestOptions
    {
        std::string code;
        // bool lex = true;
        bool parse = true;
        bool type_check = true;
        bool semantic_analyze = true;
        bool interpret = true;
        bool compile = true;

        std::optional<std::function<void(UserErrorTracker &, Parser &, const std::vector<std::unique_ptr<Stmt>> &)>> after_parse;
        std::optional<std::function<void(UserErrorTracker &, SemanticAnalyzer &)>> after_semantic_analyze;
        std::optional<std::function<void(UserErrorTracker &, TypeChecker &)>> after_type_check;
        std::optional<std::function<void(Interpreter &)>> after_interpret;
        std::optional<std::function<void(std::string &, CodeGen &)>> after_compile;

        TestOptions() = default;
    };

    bool compile(const TestOptions options);
};
