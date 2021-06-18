#ifndef AST_FUNC_H_
#define AST_FUNC_H_
#include <string>
#include <vector>
using std::string;
using std::vector;

class ast_func_arg
{
public:
    string type_name;
    string arg_name;
};

class ast_func
{
private:
    /* data */
public:
    string name;
    vector<ast_func_arg> args;
    bool is_static;
    ast_func(/* args */);
    ~ast_func();
};

ast_func::ast_func(/* args */)
{
}

ast_func::~ast_func()
{
}

#endif