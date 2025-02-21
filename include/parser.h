#pragma once

#include <memory>
#include <string>

#include "ast_node/stmt/stmt.h"
#include "exceptions/user_error_tracker.h"

#include "../build/parser.tab.hh"

extern void yy_scan_string(const char *yy_str);

class Parser {
  const std::string &code;
  UserErrorTracker *error_tracker;

public:
  Parser(const std::string &input, UserErrorTracker *error_tracker)
      : code(input), error_tracker(error_tracker) {}

  std::vector<std::unique_ptr<Stmt>> parse() {
    yy_scan_string(this->code.c_str());

    std::vector<std::unique_ptr<Stmt>> stmts;
    yy::yyParser parser(stmts, this->error_tracker);
    parser.parse();

    return stmts;
  }
};
