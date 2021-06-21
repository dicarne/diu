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
#include "compiler/compiler.hpp"

int main(int, char **)
{
    auto ce = make_shared<CodeEngine>();
    Engine e = Engine(ENGINE_VERSION);
    e.codes = ce;
    vector<string> files;
    files.push_back("test.diu");
    files.push_back("test2.diu");

    compiler::compile("test.diuc", files);

    bytecode_reader br("test.diuc");
    br.read_all(ce);

    e.Run("my", "Main", "main");
    e.RunCode();
    return 0;
}
