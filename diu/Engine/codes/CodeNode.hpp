#ifndef CODE_NODE_H_
#define CODE_NODE_H_
#include <vector>
#include <string>

#include "CodeModule.hpp"
#include "CodeFunc.hpp"
using std::vector;
using std::string;
class CodeFunc;
class CodeModule;
class CodeNode
{
private:
    /* data */
public:
    CodeNode(/* args */) {}
    ~CodeNode() {}

    string name;
    std::weak_ptr<CodeModule> mod;

    unordered_map<string, shared_ptr<CodeFunc>> funcs;
};
#endif