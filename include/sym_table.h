#pragma once

#include <map>
#include <string>
#include <memory>
#include "visitors/interpreter.h"
#include "exceptions/bird_exception.h"

/*
 * A table with key value pairs of identifiers and their respective values
 */
template <typename T>
class Environment
{

public:
    std::vector<std::map<std::string, T>> envs;

    void push_env()
    {
        envs.push_back(std::map<std::string, T>());
    }

    void pop_env()
    {
        envs.pop_back();
    }

    bool current_contains(std::string identifier)
    {
        if (envs.empty())
        {
            return false;
        }

        return envs.back().find(identifier) != envs.back().end();
    }

    bool contains(std::string identifier)
    {
        if (envs.empty())
        {
            return false;
        }

        for (auto it = envs.rbegin(); it != envs.rend(); it++)
        {
            if ((*it).find(identifier) != (*it).end())
            {
                return true;
            }
        }

        return false;
    }

    void declare(std::string identifier, T value)
    {
        if (envs.empty())
        {
            throw BirdException("no environment to declare variable " + identifier + " into");
        }

        if (current_contains(identifier))
        {
            throw BirdException("variable " + identifier + " already declared in current environment");
        }

        envs.back().insert({identifier, value});
    }

    void set(std::string identifier, T value)
    {
        if (envs.empty())
        {
            throw BirdException("no environment to set variable in");
        }

        for (auto it = envs.rbegin(); it != envs.rend(); it++)
        {
            if ((*it).find(identifier) != (*it).end())
            {
                (*it)[identifier] = value;
                return;
            }
        }

        throw BirdException("undefined identifier in environment: " + identifier);
    }

    T get(std::string identifier)
    {
        if (envs.empty())
        {
            throw BirdException("no environment to get variable from");
        }

        for (auto it = envs.rbegin(); it != envs.rend(); it++)
        {
            if ((*it).find(identifier) != (*it).end())
            {
                return (*it)[identifier];
            }
        }

        throw BirdException("undefined identifier in environment: " + identifier);
    }
};