#ifndef AST_EXPR_H_
#define AST_EXPR_H_
#include <string>
#include <vector>
#include "token.hpp"
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
        instance_value
    };
    string ins_value;
    vector<token_base*> expr_tokens;
    type expr_type;
};

ast_expr::ast_expr(/* args */)
{
}

ast_expr::~ast_expr()
{
}

#endif