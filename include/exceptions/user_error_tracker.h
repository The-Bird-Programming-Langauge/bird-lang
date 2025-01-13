#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include "../token.h"

class UserErrorTracker
{
    std::vector<std::tuple<std::string, Token>> errors;
    std::vector<std::string> code_lines;

    /*
     * Prints a nice little picture of where the error is
     */
    void print_where(unsigned int line_num, unsigned int char_num)
    {
        std::string line = this->code_lines[line_num - 1];

        const unsigned int line_width = 100;

        for (int i = 0; i < line_width; i++)
        {
            std::cout << '~';
        }
        std::cout << std::endl;

        if (line_num > 0)
        {
            std::cout << this->code_lines[line_num - 1] << std::endl;
        }

        std::cout << line << std::endl;
        for (int i = 0; i < char_num - 1; i++)
        {
            std::cout << '-';
        }
        std::cout << '^' << std::endl;

        if (line_num < this->code_lines.size() - 1)
        {
            std::cout << this->code_lines[line_num + 1] << std::endl;
        }

        std::cout << std::endl;
        for (int i = 0; i < line_width; i++)
        {
            std::cout << '~';
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }

    std::string format_message(std::string message, unsigned int line_num, unsigned int char_num)
    {
        return this->format_message(message) + " (line " + std::to_string(line_num) + ", character " + std::to_string(char_num) + ")";
    }

    std::string format_message(std::string message)
    {
        return ">>[ERROR] " + message;
    }

public:
    UserErrorTracker(std::string code)
    {
        std::stringstream code_stream(code);

        std::string line;
        while (std::getline(code_stream, line, '\n'))
        {
            this->code_lines.push_back(line);
        }
    }

    void add_code_line(std::string line)
    {
        this->code_lines.push_back(line);
    }

    bool has_errors()
    {
        return this->errors.size() > 0;
    }

    /*
     * Used when an unknown character is found in the lexing process
     */
    void undefined_character(char c, unsigned int line_num, unsigned int char_num = 0)
    {
        auto message = std::string("undefined character ") + c;
        std::cout << this->format_message(message, line_num, char_num) << std::endl;
        this->print_where(line_num, char_num);
        this->exit_program();
    }

    /*
     * Used when something is unterminated in the lexing process
     */
    void unterminated(std::string what, unsigned int line_num, unsigned int char_num = 0)
    {
        auto message = std::string("unterminated ") + what;
        std::cout << this->format_message(message, line_num, char_num) << std::endl;
        this->print_where(line_num, char_num);
        this->exit_program();
    }

    /*
     * Used when there is an unexpcted end of input
     */
    [[noreturn]] void unexpected_end_of_input()
    {
        this->print_errors();
        std::cout << this->format_message("unexpected end of input") << std::endl;

        // prints the end of the input
        this->print_where(this->code_lines.size(), this->code_lines.back().size());
        this->exit_program();
    }

    /*
     * Used when a symbol is expected, in a position but is not found
     */
    void expected(std::string symbol, std::string where, Token token)
    {
        this->errors.push_back(std::make_tuple(this->format_message("expected " + symbol + " " + where, token.line_num, token.char_num), token));
    }

    void type_mismatch(std::string where, Token token)
    {
        this->errors.push_back(std::make_tuple(this->format_message("type mismatch: " + where, token.line_num, token.char_num), token));
    }

    void type_error(std::string message, Token token)
    {
        this->errors.push_back(std::make_tuple(this->format_message("type error: " + message, token.line_num, token.char_num), token));
    }

    void semantic_error(std::string message, Token token)
    {
        this->errors.push_back(std::make_tuple(this->format_message("semantic error: " + message, token.line_num, token.char_num), token));
    }

    const std::vector<std::tuple<std::string, Token>> get_errors() const
    {
        return this->errors;
    }

    void print_errors()
    {
        for (auto error : this->errors)
        {
            std::cout << std::get<0>(error) << std::endl;
            auto token = std::get<1>(error);
            this->print_where(token.line_num, token.char_num);
        }

        std::cout << "[" << this->errors.size() << " ERRORS FOUND]" << std::endl;
    }

    void print_errors_and_exit()
    {
        this->print_errors();
        this->exit_program();
    }

    [[noreturn]] void exit_program()
    {
        std::cout << "[BIRD] aborting program" << std::endl;
        exit(1);
    }
};
