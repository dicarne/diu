#ifndef BYTECODE_H_
#define BYTECODE_H_
#include <deque>
using std::deque;

class bytecode
{
private:
    deque<char> metadata;
    deque<char> constvalue;
public:
    bytecode(/* args */);
    ~bytecode();
};

bytecode::bytecode(/* args */)
{
}

bytecode::~bytecode()
{
}


#endif