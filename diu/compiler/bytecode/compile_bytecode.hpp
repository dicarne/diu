#ifndef COMPILE_BYTECODE_H_
#define COMPILE_BYTECODE_H_
#include "../ast/ast.hpp"
#include "bytecode_writer.hpp"

class compile_bytecode
{
private:
    shared_ptr<AST> ast;
    string path;
    shared_ptr<bytecode_writer> writer;

    unordered_map<string, int> const_value_index;

public:
    compile_bytecode(shared_ptr<AST> ast, string path) : ast(ast), path(path)
    {
        writer = make_shared<bytecode_writer>(path);
    }
    ~compile_bytecode() {}

    void run()
    {
        writer->set_meta_data(ast->engine_ver);
        for (auto &kv : ast->packages)
        {
            auto pkg = kv.second.packname;
            if (const_value_index.find(pkg) == const_value_index.end())
            {
                const_value_index[pkg] = writer->write_const_string(pkg);
            }
            for (auto &it : kv.second.import_symbol)
            {
                if (const_value_index.find(it) == const_value_index.end())
                {
                    const_value_index[it] = writer->write_const_string(it);
                }
            }
        }

        for (auto &kv : ast->packages)
        {
            auto pkg = kv.second.packname;
            writer->reg_package(const_value_index[pkg]);
        }

        writer->make();
    }
};
#endif