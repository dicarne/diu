#ifndef CODE_CODE_PAGE_H_
#define CODE_CODE_PAGE_H_
#include <vector>
#include <unordered_map>
#include <string>
#include "CodeNode.hpp"
#include "../../compiler/bytecode/op_code.hpp"
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

class CodeNode;
class CodeCodePage
{
private:
    /* data */
public:
    CodeCodePage(/* args */) {}
    ~CodeCodePage() {}
    shared_ptr<unordered_map<string, shared_ptr<CodeNode>>> nodes;
    string module_name;
    double engine_version;
    shared_ptr<unordered_map<int, double>> const_double;
    shared_ptr<vector<byte>> const_type;
    shared_ptr<unordered_map<int, string>> const_string;
    shared_ptr<vector<int>> package_index_map;
    shared_ptr<unordered_map<int, int>> outer_symbol_pkg_index_map;
    shared_ptr<unordered_map<string, string>> outer_symbol_pkg_map;
};
#endif