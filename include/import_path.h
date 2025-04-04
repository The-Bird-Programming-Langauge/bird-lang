#pragma once

#include "token.h"
#include <string>
#include <vector>

class ImportPath {
public:
  std::vector<std::string> path;
  std::vector<Token> token_path;
  std::string string_path;

  ImportPath() {
    this->path = {};
    this->token_path = {};
    this->string_path = "";
  }

  ImportPath(std::vector<std::string> path) {
    this->path = path;
    this->string_path = path_to_string_path(path);
    this->token_path = {};
  }

  ImportPath(std::vector<Token> token_path) {
    this->token_path = token_path;
    this->path = token_path_to_path(token_path);
    this->string_path = path_to_string_path(this->path);
  }

  ImportPath(std::string string_path) {
    this->string_path = string_path;
    this->path = string_path_to_path(string_path);
    this->token_path = {};
  }

  std::string get_file_path() {
    std::string result = "";

    for (int i = 0; i < this->path.size(); i += 1) {
      result += this->path[i];

      if (i != this->path.size() - 1) {
        result += "/";
      }
    }

    return result;
  }

  void add_string_token(std::string string_token) {
    this->path.push_back(string_token);
    this->string_path += "::" + string_token;
  }

  void add_token(Token token) {
    this->token_path.push_back(token);
    this->path.push_back(token.lexeme);
    this->string_path += "::" + token.lexeme;
  }

  void insert_import_path_at_beginning(ImportPath import_path) {
    this->path.insert(this->path.begin(), import_path.path.begin(), import_path.path.end());
    this->token_path.insert(this->token_path.begin(), import_path.token_path.begin(), import_path.token_path.end());
    this->string_path = import_path.string_path + "::" + this->string_path;
  }

  std::string path_to_string_path(std::vector<std::string> path) {
    std::string result = "";

    for (int i = 0; i < path.size(); i += 1) {
      result += path[i];

      if (i != path.size() - 1) {
        result += "::";
      }
    }

    return result;
  }

  std::vector<std::string> token_path_to_path(std::vector<Token> path) {
    std::vector<std::string> result;
  
    for (int i = 0; i < path.size(); i += 1) {
      result.push_back(path[i].lexeme);
    }

    return result;
  }

  std::vector<std::string> string_path_to_path(std::string path) {
    std::vector<std::string> result;
    std::string path_item = "";

    for (int i = 0; i < path.size(); i += 1) {
      if (path[i] == ':') {
        result.push_back(path_item);
        path_item = "";
        i += 1;
      } else {
        path_item += path[i];
      }
    }

    if (!path_item.empty()) {
      result.push_back(path_item);
    }

    return result;
  }
};