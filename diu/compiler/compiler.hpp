#ifndef COMPILER_H_
#define COMPILER_H_
#include <string>
#include <vector>
#include "bytecode/bytecode_writer.hpp"
#include "lexer.h"
#include "ast/ast.hpp"
#include "bytecode/compile_bytecode.hpp"

using std::string;
using std::vector;
class compiler
{
private:
    /* data */
public:
    compiler()
    {
    }
    ~compiler() {}

    static void compile(string execName, vector<string> sources)
    {
        auto writer = make_shared<bytecode_writer>(execName);
        for (auto &file : sources)
        {
            lexer lex;
            auto tokens = lex.process_file(file, charset::utf8);
            shared_ptr<AST> ast = make_shared<AST>();
            ast->build_ast_from_tokens(tokens);
            compile_bytecode compiler(ast, writer);
            compiler.run();
        }
        writer->complete_make();
    }
};
#endif