#ifndef ENGINE_H_
#define ENGINE_H_
#include <unordered_map>
#include <memory>

#include "../conf.h"
#include "runtime_error.hpp"

#include "RemoteEngine.hpp"
#include "Node.hpp"
#include "codes/CodeEngine.hpp"

using std::make_shared;
using std::shared_ptr;
using std::unordered_map;

class Node;
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
    static shared_ptr<Engine> I;
    shared_ptr<CodeEngine> codes;
    void Run(string mod, string node, string func);
    shared_ptr<Node> NewNode(string mod, string node);

    void SendMessage(NodeMessage *msg)
    {
        auto cb = msg->callbackNode;
        // TODO:
    }
};

shared_ptr<Engine> Engine::I = make_shared<Engine>(ENGINE_VERSION);

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
    node->load_code(codenode->second);
    node->init();
    return node;
}

void Engine::Run(string mod, string node, string func)
{
    auto nodei = NewNode(mod, node);
    NodeMessage m;
    //m.name = new string(func);
    nodei->direct_call(m);
}

#endif