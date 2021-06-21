#include <iostream>
#include "Engine/Engine.h"
#include "Engine/Node.h"
#include "Engine/Type.hpp"
#include "compiler/lexer.hpp"
#include <deque>
#include <fstream>
#include "compiler/ast/ast.hpp"
#include "compiler/bytecode/bytecode_reader.hpp"
#include "compiler/bytecode/compile_bytecode.hpp"
#include "Engine/codes/CodeCodePage.hpp"
#include "Engine/codes/CodeEngine.hpp"

int main(int, char **)
{
    auto ce = make_shared<CodeEngine>();
    Engine e = Engine(ENGINE_VERSION);
    e.codes = ce;
    auto writer = make_shared<bytecode_writer>("test.diuc");

    {
        lexer lex;
        auto tokens = lex.process_char_buff("test.diu", charset::utf8);
        shared_ptr<AST> ast = make_shared<AST>();
        ast->build_ast_from_tokens(tokens);
        compile_bytecode compiler(ast, writer);
        compiler.run();
    }

    {
        lexer lex;
        auto tokens = lex.process_char_buff("test2.diu", charset::utf8);
        shared_ptr<AST> ast = make_shared<AST>();
        ast->build_ast_from_tokens(tokens);
        compile_bytecode compiler(ast, writer);
        compiler.run();
    }

    writer->complete_make();

    bytecode_reader br("test.diuc");
    br.read_all(ce);

    e.Run("my", "Main", "main");
    e.RunCode();
    return 0;
}
