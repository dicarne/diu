#ifndef AST_IF_H_
#define AST_IF_H_
#include "ast_expr.hpp"
#include "ast_func.hpp"
#include <memory>
using std::make_shared;
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
    ast_if(/* args */)
    {
        if_true = make_shared<vector<ast_statement>>();
        else_if = make_shared<vector<ast_if>>();
        if_false = make_shared<vector<ast_statement>>();
    }
    ~ast_if() {}
};

#endif