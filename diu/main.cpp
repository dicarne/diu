#include <iostream>
#include "Engine/Engine.hpp"
#include "Engine/Node.hpp"
#include "Engine/Type.hpp"
#include "compiler/lexer.hpp"
#include <deque>
#include <fstream>
#include "compiler/ast/ast.hpp"
#include "compiler/bytecode/bytecode_reader.hpp"
#include "compiler/bytecode/compile_bytecode.hpp"
#include "Engine/codes/CodeModule.hpp"
#include "Engine/codes/CodeEngine.hpp"

int main(int, char **)
{

    lexer lex;
    auto tokens = lex.process_char_buff("test.diu", charset::utf8);
    shared_ptr<AST> ast = make_shared<AST>();
    ast->build_ast_from_tokens(tokens);

    compile_bytecode compiler(ast, "test3.diuc");
    compiler.run();

    bytecode_reader br("test3.diuc");
    auto mod = br.readall();
    auto ce = make_shared<CodeEngine>();
    ENGINE.codes = ce;

    ce->modules[mod->module_name] = mod;

    ENGINE.Run("test3.diuc", "Main", "main");
    return 0;
}
