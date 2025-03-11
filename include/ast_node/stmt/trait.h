#pragma once

#include <algorithm>
#include <csignal>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../../exceptions/bird_exception.h"
#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "stmt.h"

struct TraitMethodSignature {
  Token identifier;
  std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>> params;
  std::optional<std::shared_ptr<ParseType::Type>> return_type;
};

/*
 * Type statement AST Node that represents traits
 * ex:
 * trait Trait {
 *   fn foobar(x: int) -> int;
 * }
 */
class Trait : public Stmt {
public:
  std::vector<TraitMethodSignature> signatures;
  Token identifier;

  Trait(Token identifier,
        std::vector<std::tuple<
            Token,
            std::vector<std::pair<Token, std::shared_ptr<ParseType::Type>>>,
            std::optional<std::shared_ptr<ParseType::Type>>>>
            signatures)
      : identifier(identifier) {
    std::for_each(signatures.begin(), signatures.end(), [&](auto signature) {
      this->signatures.push_back((TraitMethodSignature){
          .identifier = std::get<0>(signature),
          .params = std::move(std::get<1>(signature)),
          .return_type = std::move(std::get<2>(signature))});
    });
  }

  void accept(Visitor *visitor) { visitor->visit_trait(this); }
};