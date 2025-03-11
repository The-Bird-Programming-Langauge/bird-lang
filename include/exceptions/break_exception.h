#pragma once

#include <exception>
#include <string>

/*
 * Break exception that should be thrown when a break statement is present in a
 * loop, which results in the loop exiting execution.
 */
class BreakException : public std::exception {
public:
  BreakException() {}
};