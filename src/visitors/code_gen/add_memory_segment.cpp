#include "visitors/code_gen.h"

void CodeGen::add_memory_segment(const std::string &str)
{
    if (this->str_offsets.find(str) != this->str_offsets.end())
    {
        return;
    }

    auto str_offset = this->current_offset;
    this->current_offset += str.size() + 1;

    MemorySegment segment = {
        str.c_str(),
        static_cast<BinaryenIndex>(str.size() + 1), // + 1 for '\0'
        BinaryenConst(this->mod, BinaryenLiteralInt32(str_offset))};

    this->str_offsets[str] = str_offset;

    this->memory_segments.push_back(segment);
}
