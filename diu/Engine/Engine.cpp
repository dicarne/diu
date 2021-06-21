#include "Engine.h"

Engine::Engine(int version)
{
    this->version = version;
    this->codes = make_shared<CodeEngine>();
    ;
}

Engine::~Engine()
{
}

void Engine::AddNewNode(shared_ptr<Node> node)
{
    node_index++;
    node->Pid = PID(version, node_index);
    nodes[node->Pid.pid] = node;
    running[node->Pid.pid] = node;
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
    NodeMessage m(NodeMessageType::Call, func, 0, PID(0, 0), false);
    m.name = func;
    m.id = 0;
    m.callbackNode = PID(0, 0);
    nodei->direct_call(m);
}
void Engine::Run(string mod, string node, string func, shared_ptr<NodeMessage> message)
{
    if (message->name == "new")
    {
        auto nodei = NewNode(mod, node);
        Run(nodei->Pid, message);
    }
    else
    {
        // try find old:
        auto old_module = static_node.find(mod);
        if (old_module == static_node.end())
        {
            if (codes->modules.find(mod) != codes->modules.end())
            {
                static_node[mod] = unordered_map<string, shared_ptr<Node>>();
            }
            else
            {
                throw runtime_error("Can't find module: " + mod);
            }
        }
        auto &module_collection = old_module != static_node.end() ? old_module->second : static_node[mod];
        auto node_static_f = module_collection.find(node);
        if (node_static_f == module_collection.end())
        {
            module_collection[node] = NewNode(mod, node);
        }
        auto n = node_static_f != module_collection.end() ? node_static_f->second : module_collection[node];
        NodeMessage m(NodeMessageType::Call, message->name, message->id, message->callbackNode, message->async_);
        m.args = message->args;
        n->direct_call(m);
    }
}

void Engine::Run(PID node, shared_ptr<NodeMessage> msg)
{
    auto find_n = nodes.find(node.pid);
    if (find_n != nodes.end())
    {
        auto reply = new NodeMessage(NodeMessageType::Call, msg->name, msg->id, msg->callbackNode, msg->async_);
        reply->args = msg->args;
        find_n->second->push_massage(reply);
    }
}

void Engine::SendMessage(NodeMessage *msg)
{
    auto cb = msg->callbackNode;
    auto find_n = nodes.find(cb.pid);
    if (find_n != nodes.end())
    {
        find_n->second->push_massage(msg);
    }
}

void Engine::RunCode()
{
    int index = 10;
    bool all_complete = false;
    while (!all_complete)
    {
        all_complete = true;
        for (auto &kv : running)
        {
            kv.second->run_once();
            all_complete &= !kv.second->active;
        }
    }
}

void Engine::load(string byecode_file)
{
    bytecode_reader br(byecode_file);
    br.read_all(this->codes);
}

void Engine::ActiveNode(PID pid)
{
}