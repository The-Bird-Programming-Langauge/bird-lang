#include "build/parser.tab.hh"
#include "src/scanner.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include "ast_node/stmt/stmt.h"

int main(int argc, char *argv[])
{
    std::ifstream file("../code.bird");
    if (!file.is_open())
    {
        std::cout << "failed to open file" << std::endl;
        return -1;
    }
    Bird::Scanner scanner(&file);
    std::vector<std::unique_ptr<Stmt>> stmts;
    Bird::Parser parser(scanner, stmts);
    // parser.set_debug_level(1);
    parser.parse();
}