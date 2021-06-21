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

    {
        lexer lex;
        auto tokens = lex.process_char_buff("test.diu", charset::utf8);
        shared_ptr<AST> ast = make_shared<AST>();
        ast->build_ast_from_tokens(tokens);
        compile_bytecode compiler(ast, "test.diuc");
        compiler.run();
        bytecode_reader br("test.diuc");
        auto mod = br.readall();
        ce->add_code_page(mod);
    }

    {
        lexer lex;
        auto tokens = lex.process_char_buff("test2.diu", charset::utf8);
        shared_ptr<AST> ast = make_shared<AST>();
        ast->build_ast_from_tokens(tokens);
        compile_bytecode compiler(ast, "test2.diuc");
        compiler.run();
        bytecode_reader br("test2.diuc");
        auto mod = br.readall();
        ce->add_code_page(mod);
    }

    e.Run("my", "Main", "main");
    e.RunCode();
    return 0;
}
