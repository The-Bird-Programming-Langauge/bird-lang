// #pragma once
// #include "../ast_node/index.h"
// #include "visitor_adapter.h"
// #include <algorithm>
// #include <iterator>
// #include <set>
// #include <unordered_map>

// /*
//  * This visitor is responsible for removing generic polymorphic funcitons
//  into
//  * concrete implementations
//  * Read more here: https://en.wikipedia.org/wiki/Monomorphization
//  */
// class Monomorphizer : public VisitorAdapter {
//   std::unordered_map<std::string,
//                      std::vector<std::vector<std::shared_ptr<ParseType::Type>>>>
//       requested_implementations;

//   std::set<std::string> function_names;
//   std::string function_name;

//   std::vector<std::shared_ptr<ParseType::Type>> looking_for;

// public:
//   Monomorphizer() {};
//   void monomorphize(std::vector<std::unique_ptr<Stmt>> *stmts) {
//     for (auto &stmt : *stmts) {
//       stmt->accept(this);
//     }

//     std::vector<std::pair<int, std::unique_ptr<Func>>> new_funcs{};

//     for (auto it = this->requested_implementations.begin();
//          it != this->requested_implementations.end(); it++) {

//       auto count = 0;
//       for (auto stmt_it = stmts->begin(); stmt_it != stmts->end(); stmt_it++)
//       {
//         if (Func *func = dynamic_cast<Func *>((*stmt_it).get())) {
//           if (func->identifier.lexeme != it->first) {
//             count += 1;
//             continue;
//           } else {

//             auto request_count = 0;
//             for (auto request : it->second) {
//               if (request.size() != func->generic_identifiers.size()) {
//                 // TODO: should this be an error?
//                 continue;
//               }
//               std::unordered_map<std::string,
//               std::shared_ptr<ParseType::Type>>
//                   generic_map;
//               for (int i = 0; i < request.size(); i += 1) {
//                 generic_map[func->generic_identifiers[i].lexeme] =
//                 request[i];
//               }

//               auto new_token = Token(
//                   func->identifier.token_type,
//                   std::to_string(request_count++) + func->identifier.lexeme,
//                   func->identifier.line_num, func->identifier.char_num);
//               auto new_return =
//                   func->return_type.has_value() &&
//                           generic_map.count(
//                               func->return_type.value()->get_token().lexeme)
//                       ? generic_map.at(
//                             func->return_type.value()->get_token().lexeme)
//                       : func->return_type;

//               std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>
//                   new_params{};

//               std::transform(
//                   func->param_list.begin(), func->param_list.end(),
//                   std::back_inserter(new_params),
//                   [&](std::pair<Token, std::shared_ptr<ParseType::Type>>
//                           param) {
//                     if (generic_map.count(param.second->get_token().lexeme))
//                     {
//                       return std::make_pair(
//                           param.first,
//                           generic_map.at(param.second->get_token().lexeme));
//                     }
//                     return param;
//                   });

//               new_funcs.push_back(
//                   {count, std::make_unique<Func>(new_token, new_return,
//                                                  new_params, func->block)});
//             }
//           }
//         }

//         count += 1;
//       }
//     }

//     std::vector<std::unique_ptr<Stmt>> new_stmts{};
//     for (int i = 0; i < stmts->size(); i += 1) {
//       if (i == new_funcs.back().first) {
//         while (i == new_funcs.back().first) {
//           new_stmts.push_back(std::move(new_funcs.back().second));
//           new_funcs.pop_back();
//         }
//       } else {
//         new_stmts.push_back(std::move((*stmts)[i]));
//       }
//     }

//     stmts->swap(new_stmts);
//   }

//   void visit_func(Func *func) override {
//     if (func->generic_identifiers.empty()) {
//       return;
//     }

//     function_names.insert(func->identifier.lexeme);
//   }

//   void visit_method(Method *method) override {
//     if (method->generic_identifiers.empty()) {
//       return;
//     }
//   }

//   void visit_call(Call *call) override {
//     if (call->type_args.empty()) {
//       return;
//     }

//     looking_for = call->type_args;
//     call->callable->accept(this);
//   }

//   void visit_method_call(MethodCall *method_call) override {
//     if (method_call->type_args.empty()) {
//       return;
//     }
//   }

//   void visit_lambda(Lambda *lambda) override {}

//   void visit_primary(Primary *primary) override {
//     if (primary->value.token_type == Token::IDENTIFIER &&
//         this->function_names.find(primary->value.lexeme) !=
//             this->function_names.end()) {
//       // this->function_name = primary->value.lexeme;
//       if (requested_implementations.count(primary->value.lexeme)) {
//         auto count = 0;
//         for (auto request :
//              requested_implementations.at(primary->value.lexeme)) {
//           if (request == looking_for) {
//             primary->value.lexeme =
//                 std::to_string(count) + primary->value.lexeme;
//             return;
//           }

//           count += 1;
//         }
//       }

//       if (this->requested_implementations.find(primary->value.lexeme) ==
//           this->requested_implementations.end()) {
//         this->requested_implementations[primary->value.lexeme] =
//             std::vector<std::vector<std::shared_ptr<ParseType::Type>>>{};
//       }

//       this->requested_implementations.at(primary->value.lexeme)
//           .push_back(looking_for);

//       primary->value.lexeme =
//           std::to_string(
//               this->requested_implementations[primary->value.lexeme].size() -
//               1) +
//           primary->value.lexeme;
//     }
//   }
// };
