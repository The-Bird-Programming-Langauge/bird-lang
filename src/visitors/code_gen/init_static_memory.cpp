#include "../../../include/visitors/code_gen.h"

void CodeGen::init_static_memory() {
  BinaryenIndex max_pages = 1000;

  BinaryenSetMemory(mod,
                    1000,      // initial pages
                    max_pages, // maximum pages
                    "memory", nullptr, nullptr, nullptr, nullptr, 0, 0);
}