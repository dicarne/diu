#ifndef AST_WHILE_H_
#define AST_WHILE_H_
#include "ast_expr.hpp"
#include "ast_func.hpp"
class ast_statement;
class ast_while
{
private:
    /* data */
public:
    shared_ptr<ast_expr> cond;
    shared_ptr<vector<ast_statement>> statements;
    ast_while(/* args */) {}
    ~ast_while() {}
};
#endif