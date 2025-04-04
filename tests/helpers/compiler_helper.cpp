#include "compile_helper.hpp"
#include <cstdlib>
#include <fstream>

bool BirdTest::compile(const TestOptions options) {
  auto code = options.code;
  UserErrorTracker error_tracker(code);
  std::vector<std::unique_ptr<Stmt>> ast;
  if (options.parse) {
    Parser parser(code, &error_tracker);
    ast = parser.parse();
    if (error_tracker.has_errors()) {
      error_tracker.print_errors();
      return false;
    }

    if (options.after_parse.has_value()) {
      options.after_parse.value()(error_tracker, parser, ast);
    }
  }

  NameDecorator name_decorator;
  name_decorator.decorate(&ast);

  if (options.semantic_analyze) {
    SemanticAnalyzer analyze_semantics(error_tracker);
    analyze_semantics.analyze_semantics(&ast);

    if (options.after_semantic_analyze.has_value()) {
      options.after_semantic_analyze.value()(error_tracker, analyze_semantics);
    }

    if (error_tracker.has_errors()) {
      error_tracker.print_errors();
      return false;
    }
  }

  if (options.type_check) {
    TypeChecker type_checker(error_tracker);
    type_checker.check_types(&ast);
    if (options.after_type_check.has_value()) {
      options.after_type_check.value()(error_tracker, type_checker);
    }

    if (error_tracker.has_errors()) {
      error_tracker.print_errors();
      return false;
    }
  }

  if (options.interpret) {
    Interpreter interpreter;
    interpreter.evaluate(&ast);

    if (options.after_interpret.has_value()) {
      options.after_interpret.value()(interpreter);
    }
  }

  if (options.compile) {
    CodeGen code_gen;
    code_gen.generate(&ast);
    pid_t pid = fork();

    if (pid == -1) {
      exit(EXIT_FAILURE);
    }

    if (pid == 0) // child process
    {
#ifdef RUN_WASM_FILE_LOCATION
      char *args[] = {(char *)"node", (char *)RUN_WASM_FILE_LOCATION, NULL};
      if (execvp("node", args) == -1) {
        std::cerr << "Error running node" << std::endl;
        exit(EXIT_FAILURE);
      }
#endif
    } else // parent process
    {
      waitpid(pid, nullptr, 0);

      std::ifstream output("./output.txt");
      std::string code;
      if (output.is_open()) {
        std::string line;
        while (output.good()) {
          getline(output, line);
          code += line += '\n';
        }
      }

      if (options.after_compile.has_value()) {
        options.after_compile.value()(code, code_gen);
      }

      output.close();
    }
  }

  return true;
};
