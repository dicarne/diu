#ifndef ENGINE_H_
#define ENGINE_H_

#include <unordered_map>
#include <memory>
#include "RemoteEngine.hpp"
#include "Node.hpp"
#include "../conf.h"

using std::shared_ptr;
using std::unordered_map;

class Engine
{
private:
    unordered_map<int, RemoteEngine> engines;
    int version;
    unordered_map<int64, shared_ptr<Node>> nodes;
    int64 node_index = 0;

public:
    Engine(int version);
    ~Engine();
    void NewNode(shared_ptr<Node> node);
};

Engine engine(ENGINE_VERSION);

Engine::Engine(int version)
{
    this->version = version;
}

Engine::~Engine()
{
}

void Engine::NewNode(shared_ptr<Node> node)
{
    node->Pid = PID(engine.version, engine.node_index++);
    nodes[node->Pid.pid] = node;
}

#endif