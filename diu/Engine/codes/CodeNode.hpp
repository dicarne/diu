#ifndef CODE_NODE_H_
#define CODE_NODE_H_
#include <vector>
#include <string>

#include "CodeCodePage.hpp"
#include "CodeFunc.hpp"
using std::vector;
using std::string;
class CodeFunc;
class CodeCodePage;
class CodeNode
{
private:
    /* data */
public:
    CodeNode(/* args */) {}
    ~CodeNode() {}

    string name;
    std::shared_ptr<CodeCodePage> mod;

    unordered_map<string, shared_ptr<CodeFunc>> funcs;
};
#endif