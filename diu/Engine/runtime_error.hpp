#ifndef RUNTIME_ERROR_H_
#define RUNTIME_ERROR_H_
#include <exception>
#include <string>
using std::string;

class runtime_error final : public std::exception
{
    string mWhat = "Runtime Error";

public:
    runtime_error(string message) : mWhat(message)
    {
    }
    const char *what() const noexcept override
    {
        return mWhat.c_str();
    }
};

#endif