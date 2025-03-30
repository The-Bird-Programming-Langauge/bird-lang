#include "../../../include/visitors/code_gen.h"

void CodeGen::init_static_memory() {
  // // calculate the exact memory in pages to allocate
  BinaryenIndex max_pages = 3;

  // // call to create memory with all segments
  BinaryenSetMemory(mod,
                    1,         // initial pages
                    max_pages, // maximum pages
                    "memory", nullptr, nullptr, nullptr, nullptr, 0, 0);
}