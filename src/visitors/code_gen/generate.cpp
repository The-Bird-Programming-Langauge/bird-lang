#include "../../../include/visitors/code_gen.h"
#include "../../../include/visitors/hoist_visitor.h"
#include "../../../include/visitors/static_visitor.h"
#include <fstream>

void CodeGen::generate(std::vector<std::unique_ptr<Stmt>> *stmts) {
  this->init_std_lib();

  HoistVisitor hoist_visitor(this->struct_names);
  hoist_visitor.hoist(stmts);

  std::vector<std::string> static_strings;
  StaticVisitor static_visitor(static_strings);
  static_visitor.static_pass(stmts);

  this->init_static_memory(static_strings);

  BinaryenExpressionRef offset =
      BinaryenConst(this->mod, BinaryenLiteralInt32(this->current_offset));

  this->current_function_name = "main";
  auto main_function_body = std::vector<BinaryenExpressionRef>();
  this->function_locals[this->current_function_name] =
      std::vector<BinaryenType>();

  main_function_body.push_back(BinaryenCall(this->mod, "initialize_memory",
                                            &offset, 1, BinaryenTypeInt32()));

  for (auto &stmt : *stmts) {
    if (auto func_stmt = dynamic_cast<Func *>(stmt.get())) {
      func_stmt->accept(this);
      // no stack push here, automatically added
      continue;
    }
    if (auto type_stmt = dynamic_cast<TypeStmt *>(stmt.get())) {
      type_stmt->accept(this);
      // no stack push here, only type table
      continue;
    }

    if (auto struct_decl = dynamic_cast<StructDecl *>(stmt.get())) {
      struct_decl->accept(this);
      // no stack push here, only type table
      continue;
    }

    if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt.get())) {
      throw BirdException("return statement not allowed in main function");
    }

    if (auto break_stmt = dynamic_cast<BreakStmt *>(stmt.get())) {
      throw BirdException("break statement not allowed in main function");
    }

    if (auto continue_stmt = dynamic_cast<ContinueStmt *>(stmt.get())) {
      throw BirdException("continue statement not allowed in main function");
    }

    stmt->accept(this);
    auto result = this->stack.pop();
    if (result.type->type != BirdTypeType::VOID) {
      result = TaggedExpression(BinaryenDrop(this->mod, result.value));
    }
    main_function_body.push_back(result.value);
  }

  auto count = 0;
  for (auto &local : this->function_locals["main"]) {
    if (local == BinaryenTypeInt32()) {
      BinaryenAddGlobal(this->mod, std::to_string(count++).c_str(), local, true,
                        BinaryenConst(this->mod, BinaryenLiteralInt32(0)));
    } else if (local == BinaryenTypeFloat64()) {
      BinaryenAddGlobal(this->mod, std::to_string(count++).c_str(), local, true,
                        BinaryenConst(this->mod, BinaryenLiteralFloat64(0.0)));
    } else {
      throw BirdException("unsupported type");
    }
  }

  BinaryenType params = BinaryenTypeNone();
  BinaryenType results = BinaryenTypeNone();

  BinaryenExpressionRef body =
      BinaryenBlock(this->mod, nullptr, main_function_body.data(),
                    main_function_body.size(), BinaryenTypeNone());

  BinaryenAddFunction(this->mod, "main", params, results,
                      this->function_locals["main"].data(),
                      this->function_locals["main"].size(), body);

  BinaryenAddFunctionExport(this->mod, "main", "main");

#ifdef DEBUG
  BinaryenModulePrint(this->mod);
#endif

  BinaryenModuleAllocateAndWriteResult result =
      BinaryenModuleAllocateAndWrite(this->mod, nullptr);

  if (!result.binary || result.binaryBytes == 0) {
    std::cerr << "failed to serialize" << std::endl;
    return;
  }

  std::string filename = "output.wasm";
  std::ofstream file(filename, std::ios::binary);
  if (file.is_open()) {
    file.write(static_cast<char *>(result.binary), result.binaryBytes);
    file.close();
    std::cout << "wasm module written to " << filename << std::endl;
  } else {
    std::cerr << "failed to open file" << filename << std::endl;
  }

  free(result.binary);
}
