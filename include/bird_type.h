#pragma once

#include <vector>

/*
 * Enum for the types of the language, used for type checking
 */
enum class BirdType
{
    INT,
    FLOAT,
    STRING,
    BOOL,
    VOID,
    ERROR,
    STRUCT
};

// TODO: figure out how to do first class functions
struct BirdFunction
{
    std::vector<BirdType> params;
    BirdType ret;

    BirdFunction(std::vector<BirdType> params, BirdType ret) : params(params), ret(ret) {}
    BirdFunction() = default;
};
