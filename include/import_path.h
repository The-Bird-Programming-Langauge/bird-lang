#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <sstream>

class ImportPath
{
public:
  std::vector<std::string> path;

  ImportPath()
  {
    this->path = {};
  }

  ImportPath(std::vector<std::string> path)
  {
    this->path = path;
  }

  ImportPath(std::vector<Token> path)
  {
    this->path = to_string_path(path);
  }

  ImportPath(std::string path)
  {
    this->path = path_split(path);
  }

  std::vector<std::string> to_string_path(std::vector<Token> path)
  {
    std::vector<std::string> result;
  
    for (int i = 0; i < path.size(); i += 1)
    {
      result.push_back(path[i].lexeme);
    }

    return result;
  }

  std::vector<std::string> path_split(std::string path)
  {
    std::vector<std::string> result;
    std::stringstream ss(path);
    std::string token;

    while (std::getline(ss, token, ':'))
    {
        if (!token.empty())
        {
            result.push_back(token);
        }
    }

    return result;
  }
};