#ifndef AST_EXPR_H_
#define AST_EXPR_H_
#include <string>
#include <vector>
#include "../token.hpp"
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
        func_call_run,
        object_chain,
        await_call,
        array_,
        find_child
    };
    string ins_value;
    vector<shared_ptr<ast_expr>> array;

    // use in func_call and object_chain
    vector<string> caller;
    string func_name;
    vector<shared_ptr<ast_expr>> args;

    // 表达式左子树
    shared_ptr<ast_expr> left;
    shared_ptr<ast_expr> right;
    // 值表达式
    shared_ptr<ast_expr> here;

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