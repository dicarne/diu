#ifndef AST_H_
#define AST_H_

#include <deque>
#include "token.hpp"
using std::deque;

class AST
{
private:
    /* data */
public:
    AST(/* args */);
    ~AST();

    void build_ast(deque<token_base *> tokens)
    {
        
    }
};

AST::AST(/* args */)
{
}

AST::~AST()
{
}

#endif