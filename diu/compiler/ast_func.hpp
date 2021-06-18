#ifndef AST_FUNC_H_
#define AST_FUNC_H_
#include <string>
#include <vector>
#include "ast_expr.hpp"
#include "ast_assign.hpp"
using std::string;
using std::vector;

class ast_func_arg
{
public:
    string type_name;
    string arg_name;
};

class ast_statement {
public:
    enum class type {
        assign, expr, ret
    };
    type statemen_type;
    shared_ptr<ast_expr> expr;
    ast_assign assign;
};

class ast_func
{
private:
    /* data */
public:
    string name;
    vector<ast_func_arg> args;
    bool is_static;
    vector<ast_statement> atatements;
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