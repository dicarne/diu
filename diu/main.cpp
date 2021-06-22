#include <iostream>
#include "Engine/Engine.h"
#include "Engine/codes/CodeEngine.hpp"
#include "compiler/compiler.hpp"
#include "Engine/Object/json.h"

int main(int, char **)
{
    vector<string> files;
    files.push_back("test.diu");
    files.push_back("test2.diu");
    compiler::compile("test.diuc", files);

    Engine e = Engine(ENGINE_VERSION);
    e.Config("config.json");
    e.Load("test.diuc");
    e.RunCode();
    return 0;
}
