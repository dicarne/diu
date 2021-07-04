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
    shared_ptr<CodeNode> node;
    shared_ptr<CodeCodePage> mod;
    vector<arg> args;
    string name;
    bool is_static = false;
    CodeFunc(/* args */) {}
    ~CodeFunc() {}
};
#endif