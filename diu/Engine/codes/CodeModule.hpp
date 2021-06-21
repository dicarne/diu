#ifndef CODE_MODULE_H_
#define CODE_MODULE_H_
#include "CodeCodePage.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
using std::unordered_map;
using std::shared_ptr;
using std::string;
using std::vector;

class CodeModule
{
private:
    /* data */
public:
    unordered_map<string, shared_ptr<CodeNode>> node_to_code_page;
    vector<shared_ptr<CodeCodePage>> pages;
    CodeModule(/* args */) {}
    ~CodeModule() {}
};

#endif