#ifndef CODE_ENGINE_H_
#define CODE_ENGINE_H_
#include <unordered_map>
#include <string>
#include <memory>
#include "CodeCodePage.hpp"
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
    void add_code_page(shared_ptr<CodeCodePage> page)
    {
        auto m = modules.find(page->module_name);
        shared_ptr<CodeModule> p;
        if (m == modules.end())
        {
            modules[page->module_name] = make_shared<CodeModule>();
            p = modules[page->module_name];
        }
        else
        {
            p = m->second;
        }
        for (auto &node : *page->nodes)
        {
            p->node_to_code_page[node.first] = node.second;
        }
        p->pages.push_back(page);
    }
};
#endif