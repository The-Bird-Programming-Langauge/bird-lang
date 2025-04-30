#pragma once
#include "../exceptions/user_error_tracker.h"
#include "visitor.h"
#include "visitor_adapter.h"
#include <cfloat>
#include <cstdint>
#include <float.h>
#include <limits>
#include <stdexcept>
#include <stdint.h>

class LiteralLimitChecker : VisitorAdapter {
  UserErrorTracker &user_error_tracker;

public:
  LiteralLimitChecker(UserErrorTracker &user_error_tracker)
      : user_error_tracker(user_error_tracker) {}

  void check_literal_limits(std::vector<std::unique_ptr<Stmt>> *stmts) {
    for (auto &stmt : *stmts) {
      stmt->accept(this);
    }
  }

  void visit_primary(Primary *primary) override {
    switch (primary->value.token_type) {
    case Token::Type::FLOAT_LITERAL: {
      // max double size
      try {
        std::stod(primary->value.lexeme);
      } catch (std::out_of_range e) {
        this->user_error_tracker.semantic_error("Float literal out of range",
                                                primary->value);
      }
    }
    case Token::Type::INT_LITERAL: {
      try {
        auto i = std::stoi(primary->value.lexeme);
        if (i > std::numeric_limits<int32_t>::max() ||
            i < std::numeric_limits<int32_t>::min()) {
          throw std::out_of_range("out of range");
        }
      } catch (std::out_of_range e) {
        this->user_error_tracker.semantic_error("Int literal out of range",
                                                primary->value);
      }
      break;
    }
    default: {
      // everything fine
    }
    }
  }
};