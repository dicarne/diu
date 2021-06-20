#ifndef ENGINE_H_
#define ENGINE_H_

#include <unordered_map>
#include <memory>
#include "RemoteEngine.hpp"
#include "Node.hpp"
#include "../conf.h"
#include "runtime_error.hpp"
#include "codes/CodeEngine.hpp"

using std::shared_ptr;
using std::unordered_map;
using std::make_shared;

class Engine
{
private:
    unordered_map<int, RemoteEngine> engines;
    int version;
    unordered_map<int64, shared_ptr<Node>> nodes;
    int64 node_index = 0;
    void AddNewNode(shared_ptr<Node> node);

public:
    Engine(int version);
    ~Engine();

    shared_ptr<CodeEngine> codes;
    void Run(string mod, string node, string func);
    shared_ptr<Node> NewNode(string mod, string node);
};

Engine ENGINE(ENGINE_VERSION);

Engine::Engine(int version)
{
    this->version = version;
}

Engine::~Engine()
{
}

void Engine::AddNewNode(shared_ptr<Node> node)
{
    node->Pid = PID(version, node_index++);
    nodes[node->Pid.pid] = node;
}

shared_ptr<Node> Engine::NewNode(string mod, string node_name)
{
    auto codemod = codes->modules.find(mod);
    if (codemod == codes->modules.end())
        throw runtime_error("can't find module " + mod);
    auto codenode = codemod->second->nodes->find(node_name);
    if (codenode == codemod->second->nodes->end())
        throw runtime_error("can't find node " + node_name + " in module " + mod);
    auto node = make_shared<Node>();
    AddNewNode(node);
    node->code_page = codenode->second;
    return node;
}

void Engine::Run(string mod, string node, string func)
{
    auto nodei = NewNode(mod, node);
    NodeMessage m;
    m.name = func;
    nodei->call(m);
}

#endif