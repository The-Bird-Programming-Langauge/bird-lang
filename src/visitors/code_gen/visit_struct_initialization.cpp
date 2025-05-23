#include "../../../include/visitors/code_gen.h"
#include <binaryen-c.h>

void CodeGen::visit_struct_initialization(
    StructInitialization *struct_initialization) {
  auto type = this->type_table.get(struct_initialization->identifier.lexeme);

  std::shared_ptr<StructType> struct_type =
      safe_dynamic_pointer_cast<StructType>(type);
  if (struct_constructors.find(struct_initialization->identifier.lexeme) ==
      struct_constructors.end()) {
    this->create_struct_constructor(struct_type);
  }

  std::vector<BinaryenExpressionRef> args;
  for (auto &field : struct_type->fields) {
    auto found = false;
    for (auto &field_assignment : struct_initialization->field_assignments) {
      if (field.first == field_assignment.first) {
        found = true;
        field_assignment.second->accept(this);
        auto field_value = this->stack.pop();
        args.push_back(field_value.value);
        break;
      }
    }

    if (!found) {
      auto default_value =
          field.second->get_tag() == TypeTag::FLOAT
              ? BinaryenConst(this->mod, BinaryenLiteralFloat64(0.0))
          : field.second->get_tag() == TypeTag::STRING
              ? this->generate_string_from_string("").value
              : BinaryenConst(this->mod, BinaryenLiteralInt32(0));
      args.push_back(default_value);
    }
  }

  auto call =
      BinaryenCall(this->mod, struct_constructors[struct_type->name].c_str(),
                   args.data(), args.size(), BinaryenTypeInt32());
  auto create_ref = BinaryenCall(this->mod, struct_constructors["0ref"].c_str(),
                                 &call, 1, BinaryenTypeInt32());
  auto &locals = this->function_locals[this->current_function_name];
  locals.push_back(BinaryenTypeInt32());

  auto identifier = std::to_string(locals.size() - 1) + "temp";
  this->environment.declare(identifier,
                            TaggedIndex(locals.size() - 1, struct_type));

  auto set_ref = this->binaryen_set(identifier, create_ref);
  auto block = BinaryenBlock(this->mod, nullptr, &set_ref.value, 1,
                             bird_type_to_binaryen_type(type));
  auto get_ref = this->binaryen_get(identifier);
  auto register_root = BinaryenCall(this->mod, "register_root", &get_ref.value,
                                    1, BinaryenTypeNone());
  BinaryenBlockInsertChildAt(block, 1, register_root);
  BinaryenBlockInsertChildAt(block, 1, this->binaryen_get(identifier).value);

  this->stack.push(TaggedExpression(block, type));
}

void CodeGen::create_struct_constructor(
    std::shared_ptr<StructType> struct_type) {
  unsigned int size = 0;
  for (auto &field : struct_type->fields) {
    size += bird_type_byte_size(field.second);
  }

  std::vector<BinaryenExpressionRef> constructor_body;

  const auto size_literal =
      BinaryenConst(this->mod, BinaryenLiteralInt32(size));
  const auto num_ptrs = BinaryenConst(
      this->mod, BinaryenLiteralInt32(
                     this->struct_name_to_num_pointers[struct_type->name]));
  BinaryenExpressionRef args[2] = {size_literal, num_ptrs};
  auto call =
      BinaryenCall(this->mod, "mem_alloc", args, 2, BinaryenTypeInt32());

  std::vector<BinaryenType> param_types;
  for (auto &field : struct_type->fields) {
    param_types.push_back(bird_type_to_binaryen_type(field.second));
  }

  constructor_body.push_back(
      BinaryenLocalSet(this->mod, param_types.size(), call));

  int count = 0;
  for (auto &field : struct_type->fields) {
    auto type = field.second;

    auto offset = 0;
    for (auto &struct_field : struct_type->fields) {
      if (struct_field.first == field.first)
        break;

      offset += bird_type_byte_size(struct_field.second);
    }

    BinaryenExpressionRef args[3] = {
        BinaryenLocalGet(this->mod, param_types.size(), BinaryenTypeInt32()),
        BinaryenConst(this->mod, BinaryenLiteralInt32(offset)),
        BinaryenLocalGet(this->mod, count++, bird_type_to_binaryen_type(type))};

    constructor_body.push_back(
        BinaryenCall(this->mod, get_mem_set_for_type(type->get_tag()), args, 3,
                     BinaryenTypeNone()));
  }

  constructor_body.push_back(
      BinaryenReturn(this->mod, BinaryenLocalGet(this->mod, param_types.size(),
                                                 BinaryenTypeInt32())));
  auto constructor_var_types = BinaryenTypeInt32();
  BinaryenAddFunction(
      this->mod, struct_type->name.c_str(),
      BinaryenTypeCreate(param_types.data(), param_types.size()),
      BinaryenTypeInt32(), &constructor_var_types, 1,
      BinaryenBlock(this->mod, nullptr, constructor_body.data(),
                    constructor_body.size(), BinaryenTypeNone()));

  struct_constructors[struct_type->name] = struct_type->name;
}
