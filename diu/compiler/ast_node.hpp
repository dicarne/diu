#ifndef AST_NODE_H_
#define AST_NODE_H_
#include <string>
#include <unordered_map>
#include "ast_func.hpp"
using std::string;
using std::unordered_map;

class ast_node
{
private:
    /* data */
public:
    unordered_map<string, ast_func> funcs;
    ast_node(/* args */);
    ~ast_node();
};

ast_node::ast_node(/* args */)
{
}

ast_node::~ast_node()
{
}

#endif