#include "visitors/code_gen.h"

void CodeGen::init_static_memory(std::vector<std::string> &strings)
{
    for (auto &str : strings)
    {
        this->add_memory_segment(str);
    }

    std::vector<const char *> segments;
    std::vector<BinaryenIndex> sizes;
    bool *passive = new bool[memory_segments.size()];
    std::vector<BinaryenExpressionRef> offsets;

    // add all memory segment information to
    // vectors to set at once
    for (const auto &segment : memory_segments)
    {
        segments.push_back(segment.data);
        sizes.push_back(segment.size);
        passive[segments.size() - 1] = false;
        offsets.push_back(segment.offset);
    }
    // since static memory is added at once we can
    // calculate the exact memory in pages to allocate
    BinaryenIndex max_pages = (this->current_offset / 65536) + 2;

    // call to create memory with all segments
    BinaryenSetMemory(
        mod,
        1,         // initial pages
        max_pages, // maximum pages
        "memory",
        segments.data(),
        passive,
        offsets.data(),
        sizes.data(),
        segments.size(),
        0);

    delete[] passive;
}