#include <iostream>
#include "Engine/Engine.h"
#include "Engine/codes/CodeEngine.hpp"
#include "compiler/compiler.hpp"
#include "Engine/Object/json.h"

int main(int, char **)
{
    auto j = json::from("{\"a\":{\"b\":\"20\"}}");
    auto j2 = make_shared<Object>(ObjectRawType::Struct);
    auto s = make_shared<Object>("hello");
    j2->set_child("s", s);
    j->set_child("c", make_shared<Object>(j2->to_string()));
    auto js = j->to_string() ;
    std::cout << js << std::endl;
    auto jn = json::from(js);
    return 0;
    vector<string> files;
    files.push_back("test.diu");
    files.push_back("test2.diu");
    compiler::compile("test.diuc", files);

    Engine e = Engine(ENGINE_VERSION);
    e.load("test.diuc");
    e.Run("my", "Main", "main");
    e.RunCode();
    return 0;
}
