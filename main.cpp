#include <iostream>
#include <fstream>
#include <memory>
#include <cstring>

#include "include/lexer.h"
#include "include/parser.h"
#include "include/visitors/ast_printer.h"
#include "include/visitors/interpreter.h"
#include "include/visitors/semantic_analyzer.h"
#include "include/visitors/type_checker.h"

#include "include/ast_node/expr/expr.h"
#include "include/exceptions/user_error_tracker.h"

#include "include/visitors/code_gen.h"
#include "binaryen-c.h"
#include "src/parser2.hpp" // TODO: change this name

extern int bird_parse(const char *input);

void repl();
void compile(std::string filename);
void interpret(std::string filename);
std::string read_file(std::string filename);

int main(int argc, char *argv[])
{

    // if (argc == 1)
    // {
    //     repl();
    // }
    // else if (argc == 2)
    // {
    //     std::string filename = argv[1];
    //     compile(filename);
    // }
    // else
    // {
    //     if (!strcmp(argv[1], "-i"))
    //     {
    //         interpret(std::string(argv[2]));
    //     }
    //     else if (!strcmp(argv[2], "-i"))
    //     {
    //         interpret(std::string(argv[1]));
    //     }
    // }

    // auto code = read_file("../code.bird");
    bird_parse("1+1");

    return 0;
}

void repl()
{
    Interpreter interpreter;
    std::string code;
    UserErrorTracker error_tracker(code);
    SemanticAnalyzer semantic_analyzer(&error_tracker);
    TypeChecker type_checker(&error_tracker);
    while (true)
    {
        std::cout << ">";
        std::getline(std::cin, code);

        error_tracker.add_code_line(code);

        Lexer lexer(code, &error_tracker);
        auto tokens = lexer.lex();

#ifdef DEBUG
        lexer.print_tokens();
#endif

        Parser parser(tokens, &error_tracker);
        auto ast = parser.parse();

        if (error_tracker.has_errors())
        {
            error_tracker.print_errors_and_exit();
        }

#ifdef DEBUG
        AstPrinter printer;
        printer.print_ast(&ast);
#endif

        semantic_analyzer.analyze_semantics(&ast);
        if (error_tracker.has_errors())
        {
            error_tracker.print_errors_and_exit();
        }

        type_checker.check_types(&ast);
        if (error_tracker.has_errors())
        {
            error_tracker.print_errors_and_exit();
        }

        try
        {
            interpreter.evaluate(&ast);
        }
        catch (BirdException e)
        {
            std::cout << e.what() << std::endl;
        }
        catch (std::exception e)
        {
            std::cout << "err" << std::endl;
        }
    }
}

void compile(std::string filename)
{
    auto code = read_file(filename);
    UserErrorTracker error_tracker(code);

    Lexer lexer(code, &error_tracker);
    auto tokens = lexer.lex();

#ifdef DEBUG
    lexer.print_tokens();
#endif

    Parser parser(tokens, &error_tracker);
    auto ast = parser.parse();

    if (error_tracker.has_errors())
    {
        error_tracker.print_errors_and_exit();
    }

#ifdef DEBUG
    AstPrinter printer;
    printer.print_ast(&ast);
#endif

    SemanticAnalyzer semantic_analyzer(&error_tracker);
    semantic_analyzer.analyze_semantics(&ast);

    if (error_tracker.has_errors())
    {
        error_tracker.print_errors_and_exit();
    }

    TypeChecker type_checker(&error_tracker);
    type_checker.check_types(&ast);

    if (error_tracker.has_errors())
    {
        error_tracker.print_errors_and_exit();
    }
}

void interpret(std::string filename)
{
    auto code = read_file(filename);
    UserErrorTracker error_tracker(code);

    Lexer lexer(code, &error_tracker);
    auto tokens = lexer.lex();

#ifdef DEBUG
    lexer.print_tokens();
#endif

    Parser parser(tokens, &error_tracker);
    auto ast = parser.parse();

    if (error_tracker.has_errors())
    {
        error_tracker.print_errors_and_exit();
    }

#ifdef DEBUG
    AstPrinter printer;
    printer.print_ast(&ast);
#endif

    TypeChecker type_checker(&error_tracker);
    type_checker.check_types(&ast);

    if (error_tracker.has_errors())
    {
        error_tracker.print_errors_and_exit();
    }

    Interpreter interpreter;

    try
    {
        interpreter.evaluate(&ast);
    }
    catch (BirdException e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception e)
    {
        std::cout << "err" << std::endl;
    }
}

std::string read_file(std::string filename)
{
    std::ifstream file(filename);
    std::string code;
    if (file.is_open())
    {
        std::string line;
        while (file.good())
        {
            getline(file, line);
            code += line += '\n';
        }
    }

    return code;
}