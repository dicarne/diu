#ifndef CODE_FUNC_H_
#define CODE_FUNC_H_
#include <vector>
using std::vector;
#include <string>
using std::string;

#include "../../compiler/bytecode/op_code.hpp"
#include "CodeNode.hpp"

class CodeNode;
class CodeFunc
{
private:
    /* data */
public:
    struct arg
    {
        int arg_name;
        int arg_type;
    };
    vector<opcomd> codes;
    std::weak_ptr<CodeNode> node;
    std::weak_ptr<CodeModule> mod;
    vector<arg> args;
    CodeFunc(/* args */) {}
    ~CodeFunc() {}
};
#endif