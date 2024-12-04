#pragma once

#include "exceptions/user_error_tracker.h"
#include "visitors/interpreter.h"
#include "visitors/semantic_analyzer.h"
#include "visitors/type_checker.h"
#include "../src/parser.cpp"
#include "../src/lexer.cpp"
#include "../src/callable.cpp"
#include "sym_table.h"

#include "binaryen-c.h"
#include "visitors/code_gen.h"

#include <gtest/gtest.h>
#include <vector>
#include <functional>
#include <filesystem>
#include <unistd.h>
#include <wait.h>

namespace BirdTest
{
    struct TestOptions
    {
        std::string code;
        bool lex = true;
        bool parse = true;
        bool type_check = true;
        bool semantic_analyze = true;
        bool interpret = true;
        bool compile = true;

        std::optional<std::function<void(UserErrorTracker &, Lexer &)>> after_lex;
        std::optional<std::function<void(UserErrorTracker &, Parser &)>> after_parse;
        std::optional<std::function<void(UserErrorTracker &, SemanticAnalyzer &)>> after_semantic_analyze;
        std::optional<std::function<void(UserErrorTracker &, TypeChecker &)>> after_type_check;
        std::optional<std::function<void(Interpreter &)>> after_interpret;
        std::optional<std::function<void(std::string &)>> after_compile;

        TestOptions() = default;
    };

    bool compile(const TestOptions options)
    {
        UserErrorTracker error_tracker(options.code);
        std::vector<Token> tokens;
        if (options.lex)
        {
            Lexer lexer(options.code, &error_tracker);
            tokens = lexer.lex();

            if (options.after_lex.has_value())
            {
                options.after_lex.value()(error_tracker, lexer);
            }
        }

        std::vector<std::unique_ptr<Stmt>> ast;
        if (options.parse)
        {
            Parser parser(tokens, &error_tracker);
            ast = parser.parse();

            if (options.after_parse.has_value())
            {
                options.after_parse.value()(error_tracker, parser);
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

            if (error_tracker.get_errors().size() > 0)
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

            if (error_tracker.get_errors().size() > 0)
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

            std::ifstream file(std::string("./output.wasm"));
            file.close();

            char *args[] = {(char *)"node", (char *)RUN_WASM_FILE_LOCATION, NULL};

            pid_t pid = fork();
            if (pid < 0)
            {
                std::cerr << "Fork failed" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (pid == 0) // child process
            {
                execvp("node", args);
            }
            else // parent process
            {
                wait(NULL);

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
                    options.after_compile.value()(code);
                }

                output.close();
            }
        }

        return true;
    }
};
