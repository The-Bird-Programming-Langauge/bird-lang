#pragma once

#include <exception>

/*
 * Exceptions that should be thrown because a user made a mistake in BIRD code
 */
class UserException : public std::exception {
public:
  UserException() {}
};