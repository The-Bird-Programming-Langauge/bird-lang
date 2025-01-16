#pragma once

#include "exceptions/user_error_tracker.h"
#include "visitors/interpreter.h"
#include "visitors/semantic_analyzer.h"
#include "visitors/type_checker.h"
#include "parser.h"
#include "token.h"
#include "sym_table.h"

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

    bool compile(const TestOptions options)
    {
        auto code = options.code;
        UserErrorTracker error_tracker(code);
        std::vector<std::unique_ptr<Stmt>> ast;
        if (options.parse)
        {
            Parser parser(code, &error_tracker);
            ast = parser.parse();
            if (error_tracker.has_errors())
            {
                error_tracker.print_errors();
                return false;
            }

            if (options.after_parse.has_value())
            {
                options.after_parse.value()(error_tracker, parser, ast);
            }
        }

        if (options.semantic_analyze)
        {
            SemanticAnalyzer analyze_semantics(&error_tracker);
            analyze_semantics.analyze_semantics(&ast);

            if (options.after_semantic_analyze.has_value())
            {
                options.after_semantic_analyze.value()(error_tracker, analyze_semantics);
            }

            if (error_tracker.has_errors())
            {
                error_tracker.print_errors();
                return false;
            }
        }

        if (options.type_check)
        {
            TypeChecker type_checker(&error_tracker);
            type_checker.check_types(&ast);

            if (options.after_type_check.has_value())
            {
                options.after_type_check.value()(error_tracker, type_checker);
            }

            if (error_tracker.has_errors())
            {
                error_tracker.print_errors();
                return false;
            }
        }

        if (options.interpret)
        {
            Interpreter interpreter;
            interpreter.evaluate(&ast);

            if (options.after_interpret.has_value())
            {
                options.after_interpret.value()(interpreter);
            }
        }

        if (options.compile)
        {
            CodeGen code_gen;
            code_gen.generate(&ast);
            pid_t pid = fork();

            if (pid == -1)
            {
                exit(EXIT_FAILURE);
            }

            if (pid == 0) // child process
            {
                char *args[] = {(char *)"node", (char *)RUN_WASM_FILE_LOCATION, NULL};
                if (execvp("node", args) == -1)
                {
                    std::cerr << "Error running node" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else // parent process
            {
                waitpid(pid, nullptr, 0);

                std::ifstream output("./output.txt");
                std::string code;
                if (output.is_open())
                {
                    std::string line;
                    while (output.good())
                    {
                        getline(output, line);
                        code += line += '\n';
                    }
                }

                if (options.after_compile.has_value())
                {
                    options.after_compile.value()(code, code_gen);
                }

                output.close();
            }
        }

        return true;
    }
};
