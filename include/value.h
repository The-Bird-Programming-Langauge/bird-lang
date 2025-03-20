#pragma once
#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "exceptions/bird_exception.h"

class Value;

template <typename T> inline bool is_type(Value value);

inline bool is_numeric(Value value);

inline bool is_integral(Value value);

template <typename T> inline bool is_matching_type(Value left, Value right);

template <typename T> inline T as_type(const Value &value);

template <typename T, typename U> inline T to_type(Value value);

template <typename Op>
Value coerce_numerics_with_operation(Value left, Value right, Op op);

inline double numeric_to_float(Value value);
inline int numeric_to_int(Value value);

using variant =
    std::variant<int, unsigned int, double, std::string, bool,
                 std::shared_ptr<std::vector<Value>>,
                 std::shared_ptr<std::unordered_map<std::string, Value>>,
                 std::nullptr_t>;
class Value {
public:
  variant data;
  bool is_mutable;

  Value(variant data, bool is_mutable = false)
      : data(data), is_mutable(is_mutable) {}
  Value() = default;

  Value operator+(Value right) {
    if (is_matching_type<std::string>(*this, right))
      return Value(as_type<std::string>(*this) + as_type<std::string>(right));

    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '+' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left + right; });
  }

  Value operator-(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '-' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left - right; });
  }

  Value operator*(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '*' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left * right; });
  }

  Value operator/(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '/' binary operator could not be used to "
                          "interpret these values.");
    }

    if (numeric_to_float(right) == 0)
      throw BirdException("Division by zero.");

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left / right; });
  }

  Value operator%(Value right) {
    if ((!is_integral(*this) || !is_integral(right))) {
      throw BirdException("The '%' binary operator could not be used to "
                          "interpret these values.");
    }

    if (numeric_to_int(right) == 0)
      throw BirdException("Modulo by zero.");

    int left_val = numeric_to_int(*this);
    int right_val = numeric_to_int(right);
    return Value(static_cast<unsigned int>(left_val % right_val));
  }

  Value operator>(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '>' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left > right; });
  }

  Value operator>=(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '>=' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left >= right; });
  }

  Value operator<(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '<' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left < right; });
  }

  Value operator<=(Value right) {
    if (!is_numeric(*this) || !is_numeric(right)) {
      throw BirdException("The '<=' binary operator could not be used to "
                          "interpret these values.");
    }

    return coerce_numerics_with_operation(
        *this, right, [](auto left, auto right) { return left <= right; });
  }

  Value operator!=(Value right) {
    if (is_numeric(*this) && is_numeric(right)) {
      return coerce_numerics_with_operation(
          *this, right, [](auto left, auto right) { return left != right; });
    }

    if (is_type<std::string>(*this) && is_type<std::string>(right))
      return Value(as_type<std::string>(*this) != as_type<std::string>(right));

    if (is_type<bool>(*this) && is_type<bool>(right))
      return Value(as_type<bool>(*this) != as_type<bool>(right));

    if (is_type<std::nullptr_t>(*this) && is_type<std::nullptr_t>(right))
      return Value(false);

    if (is_type<std::nullptr_t>(*this) || is_type<std::nullptr_t>(right))
      return Value(true);

    throw BirdException("The '!=' binary operator could not be used to "
                        "interpret these values.");
  }

  Value operator==(Value right) {
    if (is_numeric(*this) && is_numeric(right)) {
      return coerce_numerics_with_operation(
          *this, right, [](auto left, auto right) { return left == right; });
    }

    if (is_type<std::string>(*this) && is_type<std::string>(right))
      return Value(as_type<std::string>(*this) == as_type<std::string>(right));

    if (is_type<bool>(*this) && is_type<bool>(right))
      return Value(as_type<bool>(*this) == as_type<bool>(right));

    throw BirdException("The '==' binary operator could not be used to "
                        "interpret these values.");
  }

  Value operator!() {
    if (is_type<bool>(*this))
      return Value(!as_type<bool>(*this));

    throw BirdException(
        "The '!' unary operator could not be used to interpret these values.");
  }

  Value operator-() {
    if (is_type<int>(*this))
      return Value(-as_type<int>(*this));

    if (is_type<unsigned int>(*this))
      return Value(-to_type<int, unsigned int>(*this));

    if (is_type<double>(*this))
      return Value(-as_type<double>(*this));

    throw BirdException(
        "The '-' unary operator could not be used to interpret these values.");
  }

  Value &operator=(const Value &right) {
    if (this != &right) {
      this->data = right.data;
      this->is_mutable = this->is_mutable ? true : right.is_mutable;
    }

    return *this;
  }

  Value operator[](const Value &index) {
    if (is_type<std::string>(*this)) {
      if (!is_type<int>(index)) {
        throw BirdException(
            "The subscript operator requires an integer index.");
      }

      std::string str = as_type<std::string>(*this);
      int idx = as_type<int>(index);

      if (idx < 0 || idx >= str.size())
        throw BirdException("Index out of bounds.");

      return Value(std::string(1, str[idx]));
    }
    if (is_type<std::shared_ptr<std::vector<Value>>>(*this)) {
      if (!is_type<int>(index)) {
        throw BirdException(
            "The subscript operator requires an integer index.");
      }

      auto arr = as_type<std::shared_ptr<std::vector<Value>>>(*this);
      int idx = as_type<int>(index);

      if (idx < 0 || idx >= arr->size())
        throw BirdException("Index out of bounds.");

      return (*arr)[idx];
    }

    throw BirdException(
        "The subscript operator could not be used to interpret these values.");
  }

  Value length() {
    if (is_type<std::shared_ptr<std::vector<Value>>>(*this)) {
      auto arr = as_type<std::shared_ptr<std::vector<Value>>>(*this);
      return Value((int)arr->size());
    }

    throw BirdException("No length function for this argument");
  }

  friend std::ostream &operator<<(std::ostream &os, const Value &obj) {
    if (is_type<int>(obj))
      os << as_type<int>(obj);

    else if (is_type<unsigned int>(obj))
      os << as_type<unsigned int>(obj);

    else if (is_type<double>(obj))
      os << as_type<double>(obj);

    else if (is_type<std::string>(obj))
      os << as_type<std::string>(obj);

    else if (is_type<bool>(obj))
      os << (as_type<bool>(obj) ? "true" : "false");

    return os;
  }
};

template <typename T> inline bool is_type(Value value) {
  return std::holds_alternative<T>(value.data);
}

inline bool is_numeric(Value value) {
  return is_type<int>(value) || is_type<double>(value) ||
         is_type<unsigned int>(value);
}

inline bool is_integral(Value value) {
  return is_type<int>(value) || is_type<unsigned int>(value);
}

template <typename T> inline bool is_matching_type(Value left, Value right) {
  return is_type<T>(left) && is_type<T>(right);
}

template <typename T> inline T as_type(const Value &value) {
  return std::get<T>(value.data);
}

template <typename T, typename U> inline T to_type(Value value) {
  return is_type<T>(value) ? as_type<T>(value)
                           : static_cast<T>(as_type<U>(value));
}

// Precondition: left and right both contain numeric types
// Postcondition: the type of the returned Value will be as follows:
//    if left or right is a double -> double
//    if left or right is a signed int -> signed int
//    if left and right are both unsigned -> unsigned int
template <typename Op>
Value coerce_numerics_with_operation(Value left, Value right, Op op) {
  if (is_type<double>(left) || is_type<double>(right)) {
    return Value(op(numeric_to_float(left), numeric_to_float(right)));
  }

  if (is_type<int>(left) || is_type<int>(right)) {
    return Value(op(numeric_to_int(left), numeric_to_int(right)));
  }

  if (is_type<unsigned int>(left) && is_type<unsigned int>(right)) {
    return Value(op(as_type<unsigned int>(left), as_type<unsigned int>(right)));
  }

  throw BirdException("Failed to coerce numeric values");
}

inline double numeric_to_float(Value value) {
  if (is_type<double>(value))
    return as_type<double>(value);

  if (is_type<int>(value))
    return static_cast<double>(as_type<int>(value));

  if (is_type<unsigned int>(value))
    return static_cast<double>(as_type<unsigned int>(value));

  throw BirdException("Failed to convert numeric value to float");
}

inline int numeric_to_int(Value value) {
  if (is_type<int>(value))
    return as_type<int>(value);

  if (is_type<unsigned int>(value))
    return static_cast<int>(as_type<unsigned int>(value));

  if (is_type<double>(value))
    return static_cast<int>(as_type<double>(value));

  throw BirdException("Failed to convert numeric value to int");
}

inline unsigned int numeric_to_unsigned_int(Value value) {
  if (is_type<unsigned int>(value))
    return as_type<unsigned int>(value);

  if (is_type<int>(value))
    return static_cast<unsigned int>(as_type<int>(value));

  if (is_type<double>(value))
    return static_cast<unsigned int>(as_type<double>(value));

  throw BirdException("Failed to convert numeric value to unsigned int");
}

struct SemanticValue {
  bool is_mutable;

  SemanticValue(bool is_mutable) : is_mutable(is_mutable) {}
  SemanticValue() = default;
};