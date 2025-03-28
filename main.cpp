#include <cstring>
#include <fstream>
#include <iostream>

#include "include/exceptions/user_error_tracker.h"
#include "include/parser.h"
#include "include/visitors/ast_printer.h"
#include "include/visitors/closure_finder.h"
#include "include/visitors/code_gen.h"
#include "include/visitors/interpreter.h"
#include "include/visitors/semantic_analyzer.h"
#include "include/visitors/type_checker.h"

extern int bird_parse(const char *input);

void repl();
void compile(std::string filename);
void interpret(std::string filename);
std::string read_file(std::string filename);

int main(int argc, char *argv[]) {
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    std::string filename = argv[1];
    compile(filename);
  } else {
    if (!strcmp(argv[1], "-i")) {
      interpret(std::string(argv[2]));
    } else if (!strcmp(argv[2], "-i")) {
      interpret(std::string(argv[1]));
    }
  }

  return 0;
}

void repl() {
  Interpreter interpreter;
  std::string code;
  UserErrorTracker error_tracker(code);
  SemanticAnalyzer semantic_analyzer(error_tracker);
  TypeChecker type_checker(error_tracker);
  while (true) {
    std::cout << ">";
    std::getline(std::cin, code);

    error_tracker.add_code_line(code);

    Parser parser(code, &error_tracker);
    auto ast = parser.parse();

    if (error_tracker.has_errors()) {
      error_tracker.print_errors_and_exit();
    }

#ifdef DEBUG
    AstPrinter printer;
    printer.print_ast(&ast);
#endif

    semantic_analyzer.analyze_semantics(&ast);
    if (error_tracker.has_errors()) {
      error_tracker.print_errors_and_exit();
    }

    type_checker.check_types(&ast);
    if (error_tracker.has_errors()) {
      error_tracker.print_errors_and_exit();
    }

    try {
      interpreter.evaluate(&ast);
    } catch (std::runtime_error e) {
      std::cout << e.what() << std::endl;
    }
  }
}

void compile(std::string filename) {
  const auto code = read_file(filename);
  UserErrorTracker error_tracker(code);

  Parser parser(code, &error_tracker);
  auto ast = parser.parse();

  if (error_tracker.has_errors()) {
    error_tracker.print_errors_and_exit();
  }

#ifdef DEBUG
  AstPrinter printer;
  printer.print_ast(&ast);
#endif

  SemanticAnalyzer semantic_analyzer(error_tracker);
  semantic_analyzer.analyze_semantics(&ast);

  if (error_tracker.has_errors()) {
    error_tracker.print_errors_and_exit();
  }

  TypeChecker type_checker(error_tracker);
  type_checker.check_types(&ast);

  if (error_tracker.has_errors()) {
    error_tracker.print_errors_and_exit();
  }

  ClosureFinder closure_finder;
  auto function_capture_size = closure_finder.find_captured(&ast);

  CodeGen codegen(function_capture_size);
  codegen.generate(&ast);
}

void interpret(std::string filename) {
  auto code = read_file(filename);
  UserErrorTracker error_tracker(code);

  Parser parser(code, &error_tracker);
  auto ast = parser.parse();

  if (error_tracker.has_errors()) {
    error_tracker.print_errors_and_exit();
  }

#ifdef DEBUG
  AstPrinter printer;
  printer.print_ast(&ast);
#endif

  SemanticAnalyzer semantic_analyzer(error_tracker);
  semantic_analyzer.analyze_semantics(&ast);

  if (error_tracker.has_errors()) {
    error_tracker.print_errors_and_exit();
  }

  TypeChecker type_checker(error_tracker);
  type_checker.check_types(&ast);

  if (error_tracker.has_errors()) {
    error_tracker.print_errors_and_exit();
  }

  Interpreter interpreter;

  try {
    interpreter.evaluate(&ast);
  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
  }
}

std::string read_file(std::string filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    // TODO: handle this error
  }

  std::string code((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());

  return code;
}