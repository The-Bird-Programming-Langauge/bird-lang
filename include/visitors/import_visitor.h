#pragma once

#include <fstream>
#include <string>

#include "../import_env.h"
#include "../import_namespace_env.h"

/*
 * Visitor that semantically analyzes import statements and adds their import items to the AST.
 */
class ImportVisitor : public Visitor {
public:
  ImportEnvironment standard_library;
  ImportNamespaceEnvironment import_namespace_env;
  std::set<std::string> import_identifiers;
  UserErrorTracker &user_error_tracker;
  std::vector<std::unique_ptr<Stmt>> *stmts;
  int stmt_idx;

  ImportVisitor(UserErrorTracker &user_error_tracker) : user_error_tracker(user_error_tracker) {
    this->init_standard_library();
  }

  void init_standard_library() {
    standard_library.add_item(ImportPath("Math::Trig::arccos"));
    standard_library.add_item(ImportPath("Math::Trig::arcsin"));
    standard_library.add_item(ImportPath("Math::Trig::arctan"));
    standard_library.add_item(ImportPath("Math::Trig::cos"));
    standard_library.add_item(ImportPath("Math::Trig::sin"));
    standard_library.add_item(ImportPath("Math::Trig::tan"));
    standard_library.add_item(ImportPath("Math::Trig::to_degrees"));
    standard_library.add_item(ImportPath("Math::Trig::Triangle"));
  }

  void import(std::vector<std::unique_ptr<Stmt>> *stmts) {
    this->stmts = stmts;
    this->stmt_idx = 0;
    
    while (this->stmt_idx < stmts->size()) {
      (*stmts)[this->stmt_idx]->accept(this);
      this->stmt_idx += 1;
    }

    // Once the import_namespace_env data structure is filled, we can add all of the top level namespace statements to the AST.
    for (auto& [key, value] : import_namespace_env.namespace_item.namespace_items) {
      NamespaceStmtItem* namespace_item = dynamic_cast<NamespaceStmtItem*>(value);

      this->stmts->insert(this->stmts->begin(), std::unique_ptr<Stmt>(namespace_item->stmt));
      this->stmt_idx += 1;
    }
  }

  void visit_import_stmt(ImportStmt *import_stmt) {
    // Iterate over all of the individual import paths in the import statement.
    for (int i = 0; i < import_stmt->import_paths.size(); i += 1) {
      // Perform semantic analysis to verify that the import path is a valid item in the standard library.
      if (!this->standard_library.contains_item(import_stmt->import_paths[i])) {
        this->user_error_tracker.import_error("Import item with path '" + import_stmt->import_paths[i].string_path + "' does not exist in the standard library.", import_stmt->import_paths[i].token_path.back());
        continue;
      }

      // Retrieve import items associated with each import path. Then iterate over the import items.
      auto [import_paths, import_items] = this->standard_library.get_items_recursively(import_stmt->import_paths[i]);

      // Error if the import path contains import items that were already imported.
      std::set<std::string> repeated_imports;

      for (int j = 0; j < import_paths.size(); j += 1) {
        if (this->import_identifiers.find(import_paths[j].string_path) != this->import_identifiers.end()) {
          repeated_imports.insert(import_paths[j].string_path);
        } else {
          this->import_identifiers.insert(import_paths[j].string_path);
        }
      }

      if (!repeated_imports.empty()) {
        std::string repeated_imports_string = "";

        for (auto it = repeated_imports.begin(); it != repeated_imports.end(); ++it) {
          repeated_imports_string += *it;
  
          if (std::next(it) != repeated_imports.end()) {
              repeated_imports_string += ", ";
          }
        }

        this->user_error_tracker.import_error("Import path overrides the following import items that already exists in the global namespace: " + repeated_imports_string, import_stmt->import_paths[i].token_path.back());
      }

      for (int j = 0; j < import_paths.size(); j += 1) {
        // Parse the import item and insert it at the top of the AST.
        std::string import_file_path = "../standard_library/"+import_paths[j].get_file_path()+".bird";
        auto import_code = "\n" + this->read_file(import_file_path);
        UserErrorTracker import_error_tracker(import_code);

        Parser parser(import_code, &import_error_tracker);
        auto import_ast = parser.parse();

        if (import_error_tracker.has_errors()) {
          std::cout << "Standard Library Parser Errors at Path '"+import_file_path+"':" << std::endl;
          import_error_tracker.print_errors_and_exit();
          continue;
        }

        // We must insert import statements into their associated namespace statements at the top of the AST.
        // This requires the external creation and location of these namespace statements in the import_namespace_env data structure.
        std::unique_ptr<Stmt> import_stmt = std::move(import_ast[0]);
        import_namespace_env.add_item(import_paths[j], std::move(import_stmt));
      }
    }
  }

  std::string read_file(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      // TODO: handle this error
    }
  
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  
    return code;
  }

  void visit_block(Block *block) {}

  void visit_decl_stmt(DeclStmt *decl_stmt) {}

  void visit_assign_expr(AssignExpr *assign_expr) {}

  void visit_expr_stmt(ExprStmt *expr_stmt) {}

  void visit_print_stmt(PrintStmt *print_stmt) {}

  void visit_const_stmt(ConstStmt *const_stmt) {}

  void visit_while_stmt(WhileStmt *while_stmt) {}

  void visit_for_stmt(ForStmt *for_stmt) {}

  void visit_binary(Binary *binary) {}

  void visit_unary(Unary *unary) {}

  void visit_primary(Primary *primary) {}

  void visit_ternary(Ternary *ternary) {}

  void visit_func_helper(Func *func) {}

  void visit_func(Func *func) {}

  void visit_if_stmt(IfStmt *if_stmt) {}

  void visit_call(Call *call) {}

  void visit_return_stmt(ReturnStmt *return_stmt) {}

  void visit_break_stmt(BreakStmt *break_stmt) {}

  void visit_continue_stmt(ContinueStmt *continue_stmt) {}

  void visit_type_stmt(TypeStmt *type_stmt) {}

  bool identifer_in_any_environment(std::string identifer) { return false; }

  void visit_subscript(Subscript *subscript) {}

  void visit_struct_decl(StructDecl *struct_decl) {}

  void visit_direct_member_access(DirectMemberAccess *direct_member_access) {}

  void visit_struct_initialization(StructInitialization *struct_initialization) {}

  void visit_member_assign(MemberAssign *member_assign) {}

  void visit_as_cast(AsCast *as_cast) {}

  void visit_array_init(ArrayInit *array_init) {}

  void visit_index_assign(IndexAssign *index_assign) {}

  void visit_match_expr(MatchExpr *match_expr) {}

  void visit_method(Method *method) {}

  void visit_method_call(MethodCall *method_call) {}

  void visit_namespace(NamespaceStmt *_namespace) {}

  void visit_scope_resolution(ScopeResolutionExpr *scope_resolution) {}
};