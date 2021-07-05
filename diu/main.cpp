#include <iostream>
#include "Engine/Engine.h"
#include "Engine/codes/CodeEngine.hpp"
#include "compiler/compiler.hpp"
#include "Engine/Object/json.h"

int main(int, char **)
{
    vector<string> files;
    files.push_back("diulib/test/testmain.diu");
    files.push_back("diulib/debug/assert.diu");
    files.push_back("diulib/test/test1.diu");
    
    compiler::compile("diulib/test/test.diuc", files);
 
    Engine e = Engine();
    e.Config("diulib/test/config.json");
    e.Load("diulib/test/test.diuc");
    e.RunCode();
    return 0;
}
