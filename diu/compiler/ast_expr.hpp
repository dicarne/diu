#ifndef AST_EXPR_H_
#define AST_EXPR_H_
#include <string>
#include <vector>
#include "token.hpp"
#include <memory>
using std::shared_ptr;
using std::string;
using std::vector;

class ast_expr
{
private:
    /* data */
public:
    ast_expr(/* args */);
    ~ast_expr();
    enum class type
    {
        expressions,
        instance_num,
        instance_string,
        func_call,
        object_chain
    };
    string ins_value;

    // use in func_call and object_chain
    vector<string> caller;
    string func_name;
    vector<shared_ptr<ast_expr>> args;

    shared_ptr<ast_expr> left;
    shared_ptr<ast_expr> right;

    op_type op;

    type expr_type;
};

ast_expr::ast_expr(/* args */)
{
}

ast_expr::~ast_expr()
{
}

#endif