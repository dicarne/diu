#ifndef CODE_ENGINE_H_
#define CODE_ENGINE_H_
#include <unordered_map>
#include <string>
#include <memory>
#include "CodeModule.hpp"
using std::shared_ptr;

class CodeEngine
{
private:
    /* data */
public:
    CodeEngine(/* args */) {}
    ~CodeEngine() {}
    std::unordered_map<string, shared_ptr<CodeModule>> modules;
};
#endif