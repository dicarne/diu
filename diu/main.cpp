#include <iostream>
#include "Engine/Engine.hpp"
#include "Engine/Node.hpp"
#include "Engine/Type.hpp"

int main(int, char**) {
    std::cout << "Hello, Diu Exec!\n";
    Engine e(100);
    Node n(PID(100, 1));
    std::cout << n.Pid.pid << std::endl;
    TypeManager manager;
    Type Int; Int.name = "Int";
    Type Obj; Obj.name = "Obj";
    manager.reg_type(Obj, "Int");
    manager.reg_type(Int, "");
    return 0;
}
