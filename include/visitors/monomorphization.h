#pragma once
#include "../ast_node/index.h"
#include "generic_type_checker.h"
#include "generics_collector.h"
#include "type_checker.h"
#include "visitor_adapter.h"
#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_map>

/*
 * This visitor is responsible for removing generic polymorphic funcitons
 into
 * concrete implementations
 * Read more here: https://en.wikipedia.org/wiki/Monomorphization
 */
class Monomorphizer {
  std::unordered_map<std::string,
                     std::vector<std::vector<std::shared_ptr<ParseType::Type>>>>
      requested_implementations;

  std::set<std::string> function_names;
  std::string function_name;

  std::vector<std::shared_ptr<ParseType::Type>> looking_for;
  GenericsCollector generics_collector;
  GenericTypeChecker generic_type_checker;

public:
  Monomorphizer(TypeChecker &type_checker)
      : generic_type_checker(type_checker) {};
  void monomorphize(std::vector<std::unique_ptr<Stmt>> *stmts) {
    this->requested_implementations =
        this->generics_collector.collect_types(stmts);

    std::vector<std::pair<int, std::unique_ptr<Func>>> new_funcs{};

    for (auto it = this->requested_implementations.begin();
         it != this->requested_implementations.end(); it++) {

      auto count = 0;
      for (auto stmt_it = stmts->begin(); stmt_it != stmts->end(); stmt_it++) {
        if (Func *func = dynamic_cast<Func *>((*stmt_it).get())) {
          if (func->identifier.lexeme != it->first) {
            count += 1;
            continue;
          } else {

            auto request_count = 0;
            for (auto request : it->second) {
              std::unordered_map<std::string, std::shared_ptr<ParseType::Type>>
                  generic_map;
              for (int i = 0; i < request.size(); i += 1) {
                generic_map[func->generic_identifiers[i].lexeme] = request[i];
              }

              auto new_token = Token(
                  func->identifier.token_type,
                  std::to_string(request_count++) + func->identifier.lexeme,
                  func->identifier.line_num, func->identifier.char_num);
              auto new_return =
                  func->return_type.has_value() &&
                          generic_map.count(
                              func->return_type.value()->get_token().lexeme)
                      ? generic_map.at(
                            func->return_type.value()->get_token().lexeme)
                      : func->return_type;

              std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>
                  new_params{};

              std::transform(
                  func->param_list.begin(), func->param_list.end(),
                  std::back_inserter(new_params),
                  [&](std::pair<Token, std::shared_ptr<ParseType::Type>>
                          param) {
                    if (generic_map.count(param.second->get_token().lexeme)) {
                      return std::make_pair(
                          param.first,
                          generic_map.at(param.second->get_token().lexeme));
                    }
                    return param;
                  });

              std::unique_ptr<Func> new_func = std::make_unique<Func>(
                  new_token, new_return, new_params, func->block);
              new_func->set_type(func->get_type());
              new_funcs.push_back({count, std::move(new_func)});
            }
          }
        }

        count += 1;
      }
    }

    std::vector<std::unique_ptr<Stmt>> new_stmts{};
    for (int i = 0; i < stmts->size(); i += 1) {
      if (new_funcs.size() && i == new_funcs.back().first) {
        while (i == new_funcs.back().first) {
          new_stmts.push_back(std::move(new_funcs.back().second));
          new_funcs.pop_back();
        }
      } else {
        new_stmts.push_back(std::move((*stmts)[i]));
      }
    }

    stmts->swap(new_stmts);

    this->generic_type_checker.check_generic_types(stmts);
  }
};
