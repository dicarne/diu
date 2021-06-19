#ifndef BYTE_STREAM_H_
#define BYTE_STREAM_H_

class load_error final : public std::exception
{
    string mWhat = "Runtime Error";

public:
    load_error(string message) : mWhat(message)
    {
    }
    const char *what() const noexcept override
    {
        return mWhat.c_str();
    }
};

#endif