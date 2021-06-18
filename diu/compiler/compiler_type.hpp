#ifndef COMPILER_TYPE_H_
#define COMPILER_TYPE_H_
#include <unordered_set>
#include <exception>
using std::unordered_set;

class compiler_type
{
private:
    /* data */
public:
    static const unordered_set<char32_t> signals;

    static bool issignal(char32_t ch)
    {
        return signals.count(ch) > 0;
    }

    static bool isnumber(char32_t ch) {
        if (ch >= '0' && ch <= '9') return true;
        return false;
    }

    static bool isempty(char32_t c) {
        if (c == ' ' || c == '\n' || c == '\t') return true;
        return false;
    }

    compiler_type(/* args */);
    ~compiler_type();
};

compiler_type::compiler_type(/* args */)
{
}

compiler_type::~compiler_type()
{
}

const unordered_set<char32_t> compiler_type::signals = {
    '+', '-', '*', '/', '%', '^', ',', '.', '>', '<', '=', '&', '|', '!', '(', ')', '[', ']', '{', '}', ':',
    '?', ';'};

class compile_error final : public std::exception
{
    string mWhat = "Compile Error";

public:
    compile_error(string message) : mWhat(message) {}
    const char *what() const noexcept override
    {
        return this->mWhat.c_str();
    }
};


#endif
