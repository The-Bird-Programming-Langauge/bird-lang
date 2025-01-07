#include "build/parser.tab.hh"
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include "ast_node/stmt/stmt.h"

// #include "include/old_parser/parser.h"
// #include "include/old_parser/lexer.h"
// #include "include/visitors/ast_printer.h"

// #define FILENAME "../parse_testing.bird"
#define FILENAME "../code.bird"

// std::string read_file(std::string filename);

extern void yy_scan_string(const char *yy_str);

int main(int argc, char *argv[])
{
    std::ifstream file(FILENAME);
    if (!file.is_open())
    {
        std::cout << "failed to open file" << std::endl;
        return -1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    yy_scan_string(code.c_str());

    std::vector<std::unique_ptr<Stmt>> stmts;
    yy::Parser::semantic_type *yylval = nullptr;
    yy::Parser::location_type *yyloc = nullptr;
    yy::Parser parser(stmts, yylval, yyloc);
    parser.set_debug_level(1);
    parser.parse();
    return 0;

    // auto code = read_file(FILENAME);
    // UserErrorTracker error_tracker(code);
    // Lexer lexer(code, &error_tracker);
    // auto tokens = lexer.lex();
    // Parser parser(tokens, &error_tracker);
    // auto ast = parser.parse();

    // if (error_tracker.has_errors())
    // {
    //     error_tracker.print_errors_and_exit();
    // }

    // AstPrinter printer;
    // printer.print_ast(&ast);
}

// std::string read_file(std::string filename)
// {
//     std::ifstream file(filename);
//     std::string code;
//     if (file.is_open())
//     {
//         std::string line;
//         while (file.good())
//         {
//             getline(file, line);
//             code += line += '\n';
//         }
//     }
//     return code;
// }