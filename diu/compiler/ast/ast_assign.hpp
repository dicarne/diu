#ifndef AST_ASSIGN_H_
#define AST_ASSIGN_H_
#include "../compiler_type.h"
#include "ast_expr.hpp"
#include <string>
#include <vector>
using std::string;
using std::vector;

class ast_assign
{
private:
    /* data */
public:
    bool newsymbol = false;
    //string name = "";
    //vector<string> object_chain;
    shared_ptr<ast_expr> expr;
    shared_ptr<ast_expr> object_find;
    bool start_with_dot = false;
    ast_assign(/* args */);
    ~ast_assign();
};

ast_assign::ast_assign(/* args */)
{
}

ast_assign::~ast_assign()
{
}

#endif