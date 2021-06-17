#ifndef ENGINE_H_
#define ENGINE_H_

#include <unordered_map>
#include "RemoteEngine.hpp"
#include "Type.hpp"

using std::unordered_map;

class Engine
{
private:
    unordered_map<int, RemoteEngine> engines;
    TypeManager types;
    int version;
public:
    Engine(int version);
    ~Engine();
};

Engine::Engine(int version)
{
    this->version = version;
}

Engine::~Engine()
{
}
#endif