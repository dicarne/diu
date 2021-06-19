#ifndef AST_FUNC_H_
#define AST_FUNC_H_
#include <string>
#include <vector>
#include "ast_expr.hpp"
#include "ast_assign.hpp"
#include "ast_if.hpp"
#include "ast_while.hpp"

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
        assign, expr, ret, if_, while_
    };
    type statemen_type;
    shared_ptr<ast_expr> expr;
    shared_ptr<ast_assign> assign;
    shared_ptr<ast_if> if_;
    shared_ptr<ast_while> while_;
};

class ast_func
{
private:
    /* data */
public:
    string name;
    vector<ast_func_arg> args;
    bool is_static;
    vector<ast_statement> statements;
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