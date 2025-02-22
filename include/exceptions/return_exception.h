#pragma once

#include <exception>
#include <string>

/*
 * Return exception that should be thrown when a return statement is present in
 * a function, which results in the function exiting execution with the
 * possibility of a returned value.
 */
class ReturnException : public std::exception {
public:
  ReturnException() {}
};