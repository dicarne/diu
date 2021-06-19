#ifndef AST_IF_H_
#define AST_IF_H_
#include "ast_expr.hpp"
#include "ast_func.hpp"
class ast_statement;
class ast_if
{
private:
    /* data */
public:
    shared_ptr<ast_expr> cond;
    shared_ptr<vector<ast_statement>> if_true;
    shared_ptr<vector<ast_if>> else_if;
    shared_ptr<vector<ast_statement>> if_false;
    ast_if(/* args */) {}
    ~ast_if() {}
};

#endif