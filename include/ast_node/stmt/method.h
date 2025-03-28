#pragma once

#include "../../parse_type.h"
#include "../../token.h"
#include "../../visitors/visitor.h"
#include "func.h"
#include "stmt.h"

// forward declaration
class Visitor;

/*
 * Method definition AST Node
 *
 * struct Foo {
 *  fn foobar(foo: int, bar: int) -> int {
 *     statement;
 *     statement;
 *     statement;
 *  }
 *}
 */
class Method : public Func {
public:
  Token class_identifier;
  Method(Token class_identifier, Func *func)
      : Func(func), class_identifier(class_identifier) {
    this->param_list.insert(
        this->param_list.begin(),
        std::make_pair<Token, std::shared_ptr<ParseType::Type>>(
            Token(Token::SELF, "self", identifier.line_num,
                  identifier.char_num),
            std::make_shared<ParseType::UserDefined>(class_identifier)));
  }

  void accept(Visitor *visitor) { visitor->visit_method(this); }
};
