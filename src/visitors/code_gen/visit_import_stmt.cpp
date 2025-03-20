#include "../../../include/visitors/code_gen.h"

void CodeGen::visit_import_stmt(ImportStmt *import_stmt)
{
  for (int i = 0; i < import_stmt->import_paths.size(); i += 1)
  {
    ImportPath import_path = ImportPath(import_stmt->import_paths[i]);
    auto [import_paths, import_items] = this->standard_library.get_items_recursively(import_path);

    for (int j = 0; j < import_paths.size(); i += 1)
    {
      // add import item to global namespace
    
      ImportFunction* function_item = dynamic_cast<ImportFunction*>(import_items[i]);
      if (function_item)
      {
        BinaryenAddFunctionImport(
          this->mod,
          import_path.path.back().c_str(),
          "env",
          import_path.path.back().c_str(),
          std::get<0>(function_item->codegen_import),
          std::get<1>(function_item->codegen_import));
      }
    }
  }
  
  this->stack.push(BinaryenBlock(this->mod, "EXIT", nullptr, 0, BinaryenTypeNone())); // temp
};