#pragma once

#include "exceptions/bird_exception.h"
#include "token.h"
#include "import_path.h"
#include "binaryen-c.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>

class ImportItem
{
public:
  virtual ~ImportItem() {}
};

class ImportNamespace : public ImportItem
{
public:
  std::unordered_map<std::string, ImportItem*> import_items;

  ImportNamespace()
  {
    this->import_items.emplace();
  }

  ImportNamespace(std::unordered_map<std::string, ImportItem*> import_items)
  {
    this->import_items = import_items;
  }
};

class ImportVariable : public ImportItem {};

class ImportType : public ImportItem {};

class ImportStruct : public ImportItem {};

class ImportFunction : public ImportItem
{
public:
  std::tuple<BinaryenType, BinaryenType> codegen_import;

  ImportFunction() = default;

  ImportFunction(std::tuple<BinaryenType, BinaryenType> codegen_function_import)
  {
    this->codegen_import = codegen_function_import;
  }
};