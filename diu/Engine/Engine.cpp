#include "Engine.h"

Engine::Engine(int version)
{
    this->version = version;
}

Engine::~Engine()
{
}

void Engine::AddNewNode(shared_ptr<Node> node)
{
    node_index++;
    node->Pid = PID(version, node_index);
    nodes[node->Pid.pid] = node;
}

shared_ptr<Node> Engine::NewNode(string mod, string node_name)
{
    auto codemod = codes->modules.find(mod);
    if (codemod == codes->modules.end())
        throw runtime_error("can't find module " + mod);
    auto codenode = codemod->second->node_to_code_page.find(node_name);
    if (codenode == codemod->second->node_to_code_page.end())
        throw runtime_error("can't find node " + node_name + " in module " + mod);
    auto node = make_shared<Node>();
    AddNewNode(node);
    node->engine = this;
    node->load_code(codenode->second);
    node->init();
    return node;
}

void Engine::Run(string mod, string node, string func)
{
    auto nodei = NewNode(mod, node);
    NodeMessage m;
    m.name = func;
    m.id = 0;
    m.callbackNode = PID(0, 0);
    nodei->direct_call(m);
}
void Engine::Run(string mod, string node, string func, shared_ptr<NodeMessage> message)
{
    auto nodei = NewNode(mod, node);
    NodeMessage m;
    m.name = message->name;
    m.id = message->id;
    m.callbackNode = message->callbackNode;
    m.args = message->args;
    nodei->direct_call(m);
}
void Engine::SendMessage(NodeMessage *msg)
{
    auto cb = msg->callbackNode;
    // TODO:
}

void Engine::RunCode()
{
    int index = 10;
    while (index-- > 0)
    {
        for (auto &kv : nodes)
        {
            kv.second->run_once();
        }
    }
}