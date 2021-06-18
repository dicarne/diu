#ifndef AST_ASSIGN_H_
#define AST_ASSIGN_H_
#include "compiler_type.hpp"
#include "ast_expr.hpp"
#include <string>
using std::string;
class ast_assign
{
private:
    /* data */
public:
    bool newsymbol;
    string name;
    op_type assign_type;
    shared_ptr<ast_expr> expr;
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